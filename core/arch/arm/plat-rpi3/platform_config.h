/*
 * Copyright (c) 2016, Sequitur Labs Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

# define RPI3_CONFIG

# if 0
# ifdef CFG_TEE_CORE_TA_TRACE
# undef CFG_TEE_CORE_TA_TRACE
# define CFG_TEE_CORE_TA_TRACE 1
# endif
# endif

# define CFG_USE_UBOOT_HOOKS

# ifdef CFG_TEE_CORE_TA_TRACE
# undef CFG_TEE_CORE_TA_TRACE
# endif

# define CFG_DEBUG_VM_ALL 0

# if CFG_DEBUG_VM_ALL
# define CFG_DEBUG_VA
# endif

# if CFG_DEBUG_VM_ALL /* lots of debug output */
# define CFG_TEE_CORE_TA_TRACE 1
# ifdef TRACE_LEVEL
# undef TRACE_LEVEL
# define TRACE_LEVEL TRACE_MAX
# ifdef DEBUG_XLAT_TABLE
# undef DEBUG_XLAT_TABLE
# endif
# define DEBUG_XLAT_TABLE 1
# endif /* TRACE_LEVEL */
# endif /* CFG_DEBUG_VM_ALL */

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		64

#ifdef ARM64
#ifdef CFG_WITH_PAGER
#error "Pager not supported for ARM64"
#endif
#endif /* ARM64 */

/* 16550 UART */
#define CONSOLE_UART_BASE	0x3f215040 /* uart0 */
#define CONSOLE_BAUDRATE	115200
#define CONSOLE_UART_CLK_IN_HZ	19200000

#define HEAP_SIZE		(64 * 1024)

/*
 * RPi memory map
 *
 * No secure memory on RPi...
 *
 *
 *    Available to Linux <above>
 *  0x0a00_0000
 *    TA RAM: 16 MiB                          |
 *  0x0842_0000                               | TZDRAM
 *    TEE RAM: 4 MiB (CFG_TEE_RAM_VA_SIZE)    |
 *  0x0840_0000 [ARM Trusted Firmware       ] -
 *  0x0840_0000 [TZDRAM_BASE, BL32_LOAD_ADDR] -
 *    Shared memory: 4 MiB                    |
 *  0x0800_0000                               | DRAM0
 *    Available to Linux                      |
 *  0x0000_0000 [DRAM0_BASE]                  -
 *
 */

#define DRAM0_BASE		0x00000000
#define DRAM0_SIZE		0x40000000

#define CFG_SHMEM_START		(0x08000000) //below ATF
#define CFG_SHMEM_SIZE		(4 * 1024 * 1024)

#define TZDRAM_BASE		(CFG_SHMEM_START + CFG_SHMEM_SIZE)
#define TZDRAM_SIZE		(32 * 1024 * 1024)

#define CFG_TEE_CORE_NB_CORE	4

#define CFG_TEE_RAM_VA_SIZE	(4 * 1024 * 1024)

#define CFG_TEE_LOAD_ADDR	(TZDRAM_BASE + 0x20000)

#define CFG_TEE_RAM_PH_SIZE	CFG_TEE_RAM_VA_SIZE
#define CFG_TEE_RAM_START	TZDRAM_BASE

#define CFG_TA_RAM_START	ROUNDUP((TZDRAM_BASE + CFG_TEE_RAM_VA_SIZE), \
					CORE_MMU_DEVICE_SIZE)

# define CFG_TA_RAM_SIZE        (16 * 1024 * 1024) 

#define DEVICE0_BASE		ROUNDDOWN(CONSOLE_UART_BASE, \
					  CORE_MMU_DEVICE_SIZE)
#define DEVICE0_PA_BASE         DEVICE0_BASE
#define DEVICE0_SIZE		CORE_MMU_DEVICE_SIZE
#define DEVICE0_TYPE		MEM_AREA_IO_NSEC

#endif /* PLATFORM_CONFIG_H */
