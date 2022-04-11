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

#ifndef GRISP_INIT_H
#define GRISP_INIT_H

#include <rtems.h>
#include <grisp.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(GRISP_PLATFORM_GRISP_BASE)
#define GRISP_SPI_DEVICE "/dev/spibus"
#define GRISP_I2C0_DEVICE "/dev/i2c-0"
#define GRISP_I2C1_DEVICE "/dev/i2c-1"
#elif defined(GRISP_PLATFORM_GRISP2)
#define GRISP_SPI_FDT_ALIAS "spi0"
#define GRISP_SPI_DEVICE "/dev/spibus"
#define GRISP_I2C0_FDT_ALIAS "i2c0"
#define GRISP_I2C0_DEVICE "/dev/i2c-0"
#define GRISP_I2C1_FDT_ALIAS "i2c1"
#define GRISP_I2C1_DEVICE "/dev/i2c-1"

#define GRISP_INDUSTRIAL_SPI_ONBOARD_FDT_ALIAS "spi0"
#define GRISP_INDUSTRIAL_SPI_PMOD_FDT_ALIAS "spi2"
#define GRISP_INDUSTRIAL_I2C_FDT_ALIAS "i2c0"
#endif

typedef void (*grisp_check_and_create_wlandev)(void);

void	grisp_init_buses(void);
void	grisp_init_libbsd(void);
void	grisp_init_sd_card(void);
void	grisp_init_lower_self_prio(void);
rtems_status_code grisp_init_wait_for_sd(void);
void	grisp_saf1761_basic_init(void);
void	grisp_wlan_power_up(void);
void	grisp_wlan_power_down(void);
void	grisp_init_wpa_supplicant(const char *conf_file,
	    rtems_task_priority prio,
	    grisp_check_and_create_wlandev create_wlan);
void	grisp_init_dhcpcd(rtems_task_priority prio);
void	grisp_init_dhcpcd_with_config(rtems_task_priority prio, const char *conf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GRISP_INIT_H */
