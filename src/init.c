/*
 * Copyright (c) 2016 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <unistd.h>
#include <string.h>

#include <machine/rtems-bsd-commands.h>

#include <rtems.h>
#include <rtems/bsd/bsd.h>
#include <rtems/bdbuf.h>
#include <rtems/console.h>
#include <rtems/irq.h>
#include <rtems/malloc.h>
#include <rtems/media.h>
#include <rtems/score/armv7m.h>
#include <rtems/shell.h>
#include <rtems/stringto.h>
#include <rtems/dhcpcd.h>

#include <grisp.h>
#include <grisp/init.h>
#ifdef GRISP_PLATFORM_GRISP_BASE
#include <grisp/pin-config.h>
#endif

#include <bsp.h>
#include <bsp/irq.h>
#ifdef GRISP_PLATFORM_GRISP_BASE
#include <bsp/i2c.h>
#endif

#if defined(GRISP_PLATFORM_GRISP_BASE)
#include <bsp/atsam-spi.h>
#include <bsp/spi.h>
#endif
#include <dev/spi/spi.h>

#define PRIO_INIT_TASK		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_MEDIA_SERVER	200
#define STACK_SIZE_MEDIA_SERVER	(64 * 1024)
#define PRIO_IRQ_SERVER		50
#define STACK_SIZE_IRQ_SERVER	(8 * 1024)

#define EVT_MOUNTED		RTEMS_EVENT_9

#define SAF_CS			0

#if defined(GRISP_PLATFORM_GRISP_BASE)
static const atsam_spi_config spi_config = {.spi_peripheral_id = ID_SPI0,
                                            .spi_regs = SPI0};
#endif

static rtems_id wait_mounted_task_id = RTEMS_INVALID_ID;

static rtems_status_code
media_listener(rtems_media_event event, rtems_media_state state,
    const char *src, const char *dest, void *arg)
{
	printf(
		"media listener: event = %s, state = %s, src = %s",
		rtems_media_event_description(event),
		rtems_media_state_description(state),
		src
	);

	if (dest != NULL) {
		printf(", dest = %s", dest);
	}

	if (arg != NULL) {
		printf(", arg = %p\n", arg);
	}

	printf("\n");

	if (event == RTEMS_MEDIA_EVENT_MOUNT &&
	    state == RTEMS_MEDIA_STATE_SUCCESS) {
		rtems_event_send(wait_mounted_task_id, EVT_MOUNTED);
	}

	return RTEMS_SUCCESSFUL;
}

void
grisp_init_buses() {
	int rv = -1;

#if defined(GRISP_PLATFORM_GRISP_BASE)
	rv = spi_bus_register_atsam(GRISP_SPI_DEVICE, &spi_config);
	assert(rv == 0);

	rv = atsam_register_i2c_0();
	assert(rv == 0);

	rv = atsam_register_i2c_1();
	assert(rv == 0);
#elif defined(GRISP_PLATFORM_GRISP2)
	rv = spi_bus_register_imx(GRISP_SPI_DEVICE, GRISP_SPI_FDT_ALIAS);
	assert(rv == 0);

	rv = i2c_bus_register_imx(GRISP_I2C0_DEVICE, GRISP_I2C0_FDT_ALIAS);
	assert(rv == 0);

	rv = i2c_bus_register_imx(GRISP_I2C1_DEVICE, GRISP_I2C1_FDT_ALIAS);
	assert(rv == 0);
#endif
}

rtems_status_code
grisp_init_wait_for_sd(void)
{
	puts("waiting for SD...\n");
	rtems_status_code sc;
	const rtems_interval max_mount_time = 3000 /
	    rtems_configuration_get_milliseconds_per_tick();
	rtems_event_set out;

	sc = rtems_event_receive(EVT_MOUNTED, RTEMS_WAIT, max_mount_time, &out);
	assert(sc == RTEMS_SUCCESSFUL || sc == RTEMS_TIMEOUT);

	return sc;
}

void
grisp_init_lower_self_prio(void)
{
	rtems_status_code sc;
	rtems_task_priority oldprio;

	/* Let other tasks run to complete background work */
	sc = rtems_task_set_priority(RTEMS_SELF,
	    (rtems_task_priority)PRIO_INIT_TASK, &oldprio);
	assert(sc == RTEMS_SUCCESSFUL);
}

void
grisp_init_sd_card(void)
{
	rtems_status_code sc;

	wait_mounted_task_id = rtems_task_self();

	sc = rtems_bdbuf_init();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_listener_add(media_listener, NULL);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_server_initialize(
	    PRIO_MEDIA_SERVER,
	    STACK_SIZE_MEDIA_SERVER,
	    RTEMS_DEFAULT_MODES,
	    RTEMS_DEFAULT_ATTRIBUTES
	    );
	assert(sc == RTEMS_SUCCESSFUL);
}

static void
grisp_init_network_ifconfig_lo0(void)
{
	int exit_code;
	char *lo0[] = {
		"ifconfig",
		"lo0",
		"inet",
		"127.0.0.1",
		"netmask",
		"255.255.255.0",
		NULL
	};
	char *lo0_inet6[] = {
		"ifconfig",
		"lo0",
		"inet6",
		"::1",
		"prefixlen",
		"128",
		"alias",
		NULL
	};

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0), lo0);
	assert(exit_code == EX_OK);

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0_inet6), lo0_inet6);
	assert(exit_code == EX_OK);
}

// Gets the start position of value from "key=value" string
char *
get_env_value(char *const env) {
	unsigned pos = 0;
	for (unsigned i = 0; env[i] != '\0'; i++) {
        if (env[i] == '=') pos = i;
    }
	assert(pos != 0);
	return env + ++pos * sizeof(char);
}

void
write_resolv_conf(char * dns_ips) {
	FILE *file = fopen("/etc/resolv.conf", "w");
    if (file == NULL) {
        perror("Error opening /etc/resolv.conf");
    } else {
        fprintf(file, "domain grisp.local\n");
        char* token = strtok(dns_ips, " ");
        while (token != NULL) {
            fprintf(file, "nameserver %s\n", token);
            token = strtok(NULL, " ");
		}
        fclose(file);
	}
}

void
grisp_dhcpcd_hook_handler(rtems_dhcpcd_hook *hook, char *const *env) {
	(void)hook;
	char *dns_ips = NULL;
	bool skip_resolv = false;
	while (*env != NULL) {
        if (strstr(*env, "new_domain_name_servers") != NULL) {
            char *ip_list_ptr = get_env_value(*env);
            dns_ips = strdup(ip_list_ptr);
		}
		if (strstr(*env, "skip_hooks") != NULL &&
			strstr(*env, "resolv.conf") != NULL) {
            skip_resolv = true;
		}
        ++env;
	}
	if(dns_ips != NULL) {
		if (!skip_resolv){
			write_resolv_conf(dns_ips);
		}
        free(dns_ips);
	}
}

void
grisp_init_dhcpcd(rtems_task_priority prio)
{
	static char *argv_no_conf[] = { "dhcpcd", NULL };
	static rtems_dhcpcd_config config = {};

	config.argv = argv_no_conf;
	config.argc = RTEMS_BSD_ARGC(argv_no_conf);
	config.priority = prio;

	static rtems_dhcpcd_hook dhcpcd_hook = {
		.name = "grisp_dhcp_handler",
		.handler = grisp_dhcpcd_hook_handler
	};
	rtems_dhcpcd_add_hook(&dhcpcd_hook);

	rtems_dhcpcd_start(&config);
}

void
grisp_init_libbsd(void)
{
	rtems_status_code sc;

	grisp_saf1761_basic_init();

	sc = rtems_bsd_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	grisp_init_network_ifconfig_lo0();
	grisp_wlan_power_up();

	/* Let the callout timer allocate its resources */
	sc = rtems_task_wake_after( 2 );
	assert(sc == RTEMS_SUCCESSFUL);
}

#if defined(LIBBSP_ARM_ATSAM_BSP_H)
void
grisp_saf1761_basic_init(void)
{
	const Pin saf_reset = GRISP_SAF_RESET;
	const Pin saf_irq = GRISP_SAF_IRQ;

	/*
	 * Set up the waveforms for the EBI. The minimum steps to set up the
	 * patterns is 1/BOARD_MCK. On a 123 MHz MCK, this is 8.13 ns. On 150
	 * MHz it is 6.67 ns. We don't have to think about anything shorter than
	 * that.
	 *
	 * A BOARD_MCK > 150 MHz may be not supported by this code.
	 */
	assert(BOARD_MCK <= 150000000);
	const uint32_t ns_per_tick = 1000 * 1000 * 1000 / BOARD_MCK;

	/* Write pattern (ck is one clock cycle):
	 *      __ ___________________________________________________ ___
	 * A/D: __X___________________________________________________X___
	 *      ____________________                 _____________________
	 * NWE:                     \_______________/
	 *      ____________                                  ____________
	 * NCS:             \________________________________/
	 *
	 * time:   <-1 ck-> <-1 ck-> <- min 17 ns -> <-1 ck-> <-1 ck->
	 *
	 * NOTE: 17 ns is taken from SAF1761 data sheet.
	 */
	/* NOTE: nwe_setup, ncs_wr_setup and nwe_pulse have been increased
	 * by 2 ticks to have enough reserve. */
	const uint32_t nwe_setup = 2 + 2;
	const uint32_t ncs_wr_setup = 1 + 2;
	const uint32_t nwe_hold = 2;
	const uint32_t nwe_pulse = 17 / ns_per_tick + 1 + 2;
	const uint32_t ncs_wr_pulse = nwe_pulse + 2;
	const uint32_t nwe_cycle = nwe_setup + nwe_pulse + nwe_hold;

	/* Read pattern (ck is one clock cycle):
	 *      __ _________________________________________________ _______
	 * A:   __X_________________________________________________X_______
	 *      ___________                   ______________________________
	 * NRD:            \_________________/
	 *      __                                     _____________________
	 * NCS:   \___________________________________/
	 *                              _________
	 * D:   -----------------------(_________)--------------------------
	 *
	 * time:   <-1 ck-> <-- min 22 ns --> <-1 ck-> <--- x ck --->
	 *         <------------------ min 36 ns ------------------->
	 *
	 * NOTE: 22 and 36 ns is taken from SAF1761 data sheet. The x ck depends
	 * on the 36 ns. But it has to be at least one.
	 */
	/* NOTE: nrd_setup, ncs_rd_setup and nrd_pulse have been increased
	 * by 4 ticks. Without this, the controller doesn't generate interrupts
	 * cleanly. */
	const uint32_t nrd_setup = 1 + 4;
	const uint32_t ncs_rd_setup = 0 + 4;
	const uint32_t nrd_pulse = 22 / ns_per_tick + 1 + 4;
	const uint32_t nrd_hold = 1;
	const uint32_t ncs_rd_pulse = nrd_pulse + nrd_setup + nrd_hold;
	const uint32_t nrd_cycle_min = 36 / ns_per_tick + 1;
	const uint32_t nrd_cycle =
	    nrd_cycle_min > (nrd_pulse + nrd_setup + nrd_hold + 1) ?
	    nrd_cycle_min : nrd_pulse + nrd_setup + nrd_hold + 1;

	/* Enable SMC */
	PMC_EnablePeripheral(ID_SMC);

	/* disable SMC write protection */
	SMC->SMC_WPMR = 0x534D4300;

	SMC->SMC_CS_NUMBER[SAF_CS].SMC_SETUP =
	    SMC_SETUP_NCS_RD_SETUP(ncs_rd_setup) |
	    SMC_SETUP_NRD_SETUP(nrd_setup) |
	    SMC_SETUP_NCS_WR_SETUP(ncs_wr_setup) |
	    SMC_SETUP_NWE_SETUP(nwe_setup);
	SMC->SMC_CS_NUMBER[SAF_CS].SMC_PULSE =
	    SMC_PULSE_NCS_RD_PULSE(ncs_rd_pulse) |
	    SMC_PULSE_NRD_PULSE(nrd_pulse) |
	    SMC_PULSE_NCS_WR_PULSE(ncs_wr_pulse) |
	    SMC_PULSE_NWE_PULSE(nwe_pulse);
	SMC->SMC_CS_NUMBER[SAF_CS].SMC_CYCLE =
	    SMC_CYCLE_NRD_CYCLE(nrd_cycle) |
	    SMC_CYCLE_NWE_CYCLE(nwe_cycle);
	SMC->SMC_CS_NUMBER[SAF_CS].SMC_MODE =
	    SMC_MODE_READ_MODE | SMC_MODE_WRITE_MODE |
	    SMC_MODE_EXNW_MODE_DISABLED | SMC_MODE_BAT_BYTE_SELECT |
	    SMC_MODE_DBW_16_BIT | SMC_MODE_TDF_CYCLES(1);

	/* enable SMC write protection */
	SMC->SMC_WPMR = 0x534D4301;

	PIO_DisableIt(&saf_irq);

	/* Release SAF out of it's reset. */
	PIO_Set(&saf_reset);
}

void
grisp_wlan_power_up(void)
{
	const Pin wlan_en = GRISP_WLAN_EN;
	PIO_Clear(&wlan_en);
}

void
grisp_wlan_power_down(void)
{
	const Pin wlan_en = GRISP_WLAN_EN;
	PIO_Set(&wlan_en);
}
#elif defined(LIBBSP_ARM_IMX_BSP_H)
void
grisp_saf1761_basic_init(void)
{
	/* Not necessary for GRiSP2 */
}

void
grisp_wlan_power_up(void)
{
#warning FIXME: Implement
}

void
grisp_wlan_power_down(void)
{
#warning FIXME: Implement
}
#endif
