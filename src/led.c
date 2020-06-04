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

#include <bsp.h>
#include <assert.h>

#include <grisp/led.h>

#ifdef LIBBSP_ARM_ATSAM_BSP_H
#include <grisp/pin-config.h>

static const Pin led_rgb1_red = GRISP_LED_1R;
static const Pin led_rgb1_green = GRISP_LED_1G;
static const Pin led_rgb1_blue = GRISP_LED_1B;

static const Pin led_rgb2_red = GRISP_LED_2R;
static const Pin led_rgb2_green = GRISP_LED_2G;
static const Pin led_rgb2_blue = GRISP_LED_2B;

rtems_status_code
grisp_led_set(int led_nr, bool r, bool g, bool b)
{
	const Pin *led_r = NULL;
	const Pin *led_g = NULL;
	const Pin *led_b = NULL;

	switch (led_nr) {
	case 1:
		led_r = &led_rgb1_red;
		led_g = &led_rgb1_green;
		led_b = &led_rgb1_blue;
		break;
	case 2:
		led_r = &led_rgb2_red;
		led_g = &led_rgb2_green;
		led_b = &led_rgb2_blue;
		break;
	default:
		return RTEMS_INVALID_NUMBER;
		break;
	}

	if (r) {
		PIO_Set(led_r);
	} else {
		PIO_Clear(led_r);
	}

	if (g) {
		PIO_Set(led_g);
	} else {
		PIO_Clear(led_g);
	}

	if (b) {
		PIO_Set(led_b);
	} else {
		PIO_Clear(led_b);
	}

	return RTEMS_SUCCESSFUL;
}

#elif defined(LIBBSP_ARM_IMX_BSP_H)
#include <bsp/fdt.h>
#include <libfdt.h>
#include <pthread.h>
#include <bsp/imx-gpio.h>

static struct {
	struct imx_gpio_pin red;
	struct imx_gpio_pin green;
	struct imx_gpio_pin blue;
} leds[2];

static void
grisp2_init_led_by_fdt(
	struct imx_gpio_pin *led,
	const void* fdt,
	const char *fdt_path
)
{
	rtems_status_code sc;
	int node;
	node = fdt_path_offset(fdt, fdt_path);
	sc = imx_gpio_init_from_fdt_property(led,
	    node, "gpios", IMX_GPIO_MODE_OUTPUT, 0);
	assert(sc == RTEMS_SUCCESSFUL);
	imx_gpio_set_output(led, 0);
}

static void
grisp2_init_leds(void)
{
	const void *fdt;

	fdt = bsp_fdt_get();
	grisp2_init_led_by_fdt(&leds[0].red,   fdt, "/leds/grisp-rgb1-red");
	grisp2_init_led_by_fdt(&leds[0].green, fdt, "/leds/grisp-rgb1-green");
	grisp2_init_led_by_fdt(&leds[0].blue,  fdt, "/leds/grisp-rgb1-blue");
	grisp2_init_led_by_fdt(&leds[1].red,   fdt, "/leds/grisp-rgb2-red");
	grisp2_init_led_by_fdt(&leds[1].green, fdt, "/leds/grisp-rgb2-green");
	grisp2_init_led_by_fdt(&leds[1].blue,  fdt, "/leds/grisp-rgb2-blue");
}

rtems_status_code
grisp_led_set(int led_nr, bool r, bool g, bool b)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, grisp2_init_leds);

	imx_gpio_set_output(&leds[led_nr-1].red,   r ? 1 : 0);
	imx_gpio_set_output(&leds[led_nr-1].green, g ? 1 : 0);
	imx_gpio_set_output(&leds[led_nr-1].blue,  b ? 1 : 0);

	return RTEMS_SUCCESSFUL;
}
#endif

void
grisp_led_set1(bool r, bool g, bool b)
{
	rtems_status_code sc;
	sc = grisp_led_set(1, r, g, b);
	assert(sc == RTEMS_SUCCESSFUL);
}

void
grisp_led_set2(bool r, bool g, bool b)
{
	rtems_status_code sc;
	sc = grisp_led_set(2, r, g, b);
	assert(sc == RTEMS_SUCCESSFUL);
}
