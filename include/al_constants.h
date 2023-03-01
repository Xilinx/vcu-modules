/*
 * al_constants.h constants defined
 *
 * Copyright (C) 2019, Allegro DVT (www.allegrodvt.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _AL_CONSTANTS_H_
#define ifndef _AL_CONSTANTS_H_

/* MCU Mailbox */
#define MAILBOX_CMD                     0x7000
#define MAILBOX_STATUS                  0x7800
#define MAILBOX_SIZE                    (0x800 - 0x8)

/* MCU Bootloader */
#define AL5_MCU_INTERRUPT_HANDLER       0x0010
#define MCU_SLEEP_INSTRUCTION           0xba020004 /* Instruction "mbar 16" */

/* MCU Control Regs */
#define AL5_MCU_RESET                   0x9000
#define AL5_MCU_RESET_MODE              0x9004
#define AL5_MCU_STA                     0x9008
#define AL5_MCU_WAKEUP                  0x900c

#define AL5_ICACHE_ADDR_OFFSET_MSB      0x9010
#define AL5_ICACHE_ADDR_OFFSET_LSB      0x9014
#define AL5_DCACHE_ADDR_OFFSET_MSB      0x9018
#define AL5_DCACHE_ADDR_OFFSET_LSB      0x901c

#define AL5_MCU_INTERRUPT               0x9100
#define AL5_MCU_INTERRUPT_MASK          0x9104
#define AL5_MCU_INTERRUPT_CLR           0x9108

#define AXI_ADDR_OFFSET_IP              0x9208

/* MCU Cache */
#define MCU_SUBALLOCATOR_SIZE           (1024 * 1024 * 32)      /* 32 MB */
#define MCU_CACHE_OFFSET                0x80000000
#define AL5_ICACHE_SIZE                 (1024 * 600)            /* 600 KB (for possible extensions) */
#define AL5_RC_PLUGIN_CODE_START        (1024 * 512)
#define MCU_SRAM_SIZE                   0x8000                  /* 32 kB */

#endif /* _AL_CONSTANTS_H_ */
