/* Copyright (c) 2021 Peer Stritzinger GmbH, All rights reserved.
 * Aumuellerstr. 14, 82216 Maisach, Germany <info@stritzinger.com>
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

#ifndef GRISP_H
#define GRISP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <bsp.h>

#ifdef LIBBSP_ARM_ATSAM_BSP_H
#define GRISP
#define GRISP_PLATFORM_GRISP_BASE
#define GRISP_PLATFORM "grisp_base"
#endif
#if defined LIBBSP_ARM_IMX_BSP_H
#define GRISP
#define GRISP_PLATFORM_GRISP2
#define GRISP_PLATFORM "grisp2"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GRISP_H */
