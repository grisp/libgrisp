/*
 * Copyright (c) 2024 embedded brains GmbH.  All rights reserved.
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

#include <grisp.h>
#include <grisp/power.h>

#if defined(GRISP_PLATFORM_GRISP_NANO)

#include <assert.h>
#include <pthread.h>
#include <stm32u5/hal.h>

#define PW_CNT_SD_PORT GPIOD
#define PW_CNT_SD_PIN GPIO_PIN_7

#define PW_CNT_SPI_PORT GPIOA
#define PW_CNT_SPI_PIN GPIO_PIN_15

#define PW_CNT_I2C_PORT GPIOB
#define PW_CNT_I2C_PIN GPIO_PIN_0

#define PW_CNT_UWB_PORT GPIOB
#define PW_CNT_UWB_PIN GPIO_PIN_7

#define PW_CNT_CRYPTO_PORT GPIOB
#define PW_CNT_CRYPTO_PIN GPIO_PIN_8

#define PW_CNT_UART_PORT GPIOA
#define PW_CNT_UART_PIN GPIO_PIN_8

#define PW_CNT_SP_CAP_PORT GPIOB
#define PW_CNT_SP_CAP_PIN GPIO_PIN_4

static void
grisp_nano_power_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	HAL_GPIO_WritePin(PW_CNT_SD_PORT, PW_CNT_SD_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_SD_PIN;
	HAL_GPIO_Init(PW_CNT_SD_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PW_CNT_SPI_PORT, PW_CNT_SPI_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_SPI_PIN;
	HAL_GPIO_Init(PW_CNT_SPI_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PW_CNT_I2C_PORT, PW_CNT_I2C_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_I2C_PIN;
	HAL_GPIO_Init(PW_CNT_I2C_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PW_CNT_UWB_PORT, PW_CNT_UWB_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_UWB_PIN;
	HAL_GPIO_Init(PW_CNT_UWB_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PW_CNT_CRYPTO_PORT, PW_CNT_CRYPTO_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_CRYPTO_PIN;
	HAL_GPIO_Init(PW_CNT_CRYPTO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PW_CNT_UART_PORT, PW_CNT_UART_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_UART_PIN;
	HAL_GPIO_Init(PW_CNT_UART_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PW_CNT_SP_CAP_PORT, PW_CNT_SP_CAP_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PW_CNT_SP_CAP_PIN;
	HAL_GPIO_Init(PW_CNT_SP_CAP_PORT, &GPIO_InitStruct);
}

void
grisp_power_switch(grisp_power_module mod, bool on)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, grisp_nano_power_init);

	GPIO_TypeDef *gpio = PW_CNT_SD_PORT;
	uint16_t pin = PW_CNT_SD_PIN;

	switch (mod) {
	case GRISP_POWER_SD:
		gpio = PW_CNT_SD_PORT;
		pin = PW_CNT_SD_PIN;
		break;
	case GRISP_POWER_SPI:
		gpio = PW_CNT_SPI_PORT;
		pin = PW_CNT_SPI_PIN;
		break;
	case GRISP_POWER_I2C:
		gpio = PW_CNT_I2C_PORT;
		pin = PW_CNT_I2C_PIN;
		break;
	case GRISP_POWER_UWB:
		gpio = PW_CNT_UWB_PORT;
		pin = PW_CNT_UWB_PIN;
		break;
	case GRISP_POWER_CRYPTO:
		gpio = PW_CNT_CRYPTO_PORT;
		pin = PW_CNT_CRYPTO_PIN;
		break;
	case GRISP_POWER_UART:
		gpio = PW_CNT_UART_PORT;
		pin = PW_CNT_UART_PIN;
		break;
	case GRISP_POWER_SP_CAP:
		gpio = PW_CNT_SP_CAP_PORT;
		pin = PW_CNT_SP_CAP_PIN;
		on = !on;
		break;
	default:
		assert(false);
		break;
	}

	HAL_GPIO_WritePin(gpio, pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
#endif
