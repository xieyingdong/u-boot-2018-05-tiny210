/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include "s5pv210.h"

/*
 * Register usages:
 *
 * r5 has zero always
 */

	.globl lowlevel_init
lowlevel_init:
	mov	r9, lr

	bl clock_init
	/* for UART */
	bl uart_asm_init

	mov	lr, r9
	mov	pc, lr


// 时钟控制器基地址
#define ELFIN_CLOCK_POWER_BASE		0xE0100000	

// 时钟相关的寄存器相对时钟控制器基地址的偏移值
#define APLL_LOCK_OFFSET		0x00		
#define MPLL_LOCK_OFFSET		0x08

#define APLL_CON0_OFFSET		0x100
#define APLL_CON1_OFFSET		0x104
#define MPLL_CON_OFFSET			0x108

#define CLK_SRC0_OFFSET			0x200
#define CLK_SRC1_OFFSET			0x204
#define CLK_SRC2_OFFSET			0x208
#define CLK_SRC3_OFFSET			0x20c
#define CLK_SRC4_OFFSET			0x210
#define CLK_SRC5_OFFSET			0x214
#define CLK_SRC6_OFFSET			0x218
#define CLK_SRC_MASK0_OFFSET	0x280
#define CLK_SRC_MASK1_OFFSET	0x284

#define CLK_DIV0_OFFSET			0x300
#define CLK_DIV1_OFFSET			0x304
#define CLK_DIV2_OFFSET			0x308
#define CLK_DIV3_OFFSET			0x30c
#define CLK_DIV4_OFFSET			0x310
#define CLK_DIV5_OFFSET			0x314
#define CLK_DIV6_OFFSET			0x318
#define CLK_DIV7_OFFSET			0x31c

#define CLK_DIV0_MASK			0x7fffffff

#define APLL_MDIV      	 		0x7d
#define APLL_PDIV       		0x3
#define APLL_SDIV       		0x1
#define MPLL_MDIV				0x29b
#define MPLL_PDIV				0xc
#define MPLL_SDIV				0x1

#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)
#define APLL_VAL			set_pll(APLL_MDIV,APLL_PDIV,APLL_SDIV)
#define MPLL_VAL			set_pll(MPLL_MDIV,MPLL_PDIV,MPLL_SDIV)


clock_init:
	ldr	r0, =ELFIN_CLOCK_POWER_BASE
	
	// 1 设置各种时钟开关，暂时不使用PLL
	ldr	r1, =0x0
	// 芯片手册P378 寄存器CLK_SRC：Select clock source 0 (Main)
	str	r1, [r0, #CLK_SRC0_OFFSET]				

	// 2 设置锁定时间，使用默认值即可
	// 设置PLL后，时钟从Fin提升到目标频率时，需要一定的时间，即锁定时间
	ldr	r1,	=0x0000FFFF					
	str	r1,	[r0, #APLL_LOCK_OFFSET]				
	str r1, [r0, #MPLL_LOCK_OFFSET]	 				


	// 3 设置分频
	// 清bit[0~31]
	ldr r1, [r0, #CLK_DIV0_OFFSET]					
	ldr	r2, =CLK_DIV0_MASK					
	bic	r1, r1, r2
	ldr	r2, =0x14131440						
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	// 4 设置PLL
	// FOUT = MDIV*FIN/(PDIV*2^(SDIV-1))=0x7d*24/(0x3*2^(1-1))=1000 MHz
	ldr	r1, =APLL_VAL						
	str	r1, [r0, #APLL_CON0_OFFSET]
	// FOUT = MDIV*FIN/(PDIV*2^SDIV)=0x29b*24/(0xc*2^1)= 667 MHz
	ldr	r1, =MPLL_VAL						
	str	r1, [r0, #MPLL_CON_OFFSET]

	// 5 设置各种时钟开关,使用PLL
	ldr	r1, [r0, #CLK_SRC0_OFFSET]
	ldr	r2, =0x10001111
	orr	r1, r1, r2
	str	r1, [r0, #CLK_SRC0_OFFSET]

	mov	pc, lr


/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:

	/* set GPIO(GPA) to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x22222222
	str   	r1, [r0, #GPA0CON_OFFSET]

	ldr     r1, =0x2222
	str     r1, [r0, #GPA1CON_OFFSET]

	// HP V210 use. SMDK not use.
#if defined(CONFIG_VOGUES)
	ldr    r1, =0x100
	str    r1, [r0, #GPC0CON_OFFSET]

	ldr    r1, =0x4
	str    r1, [r0, #GPC0DAT_OFFSET]
#endif

	ldr	r0, =ELFIN_UART_CONSOLE_BASE		@0xEC000000
	mov	r1, #0x0
	str	r1, [r0, #UFCON_OFFSET]
	mov	r1, #0x1
	str	r1, [r0, #UMCON_OFFSET]

	mov	r1, #0x3
	str	r1, [r0, #ULCON_OFFSET]

	ldr	r1, =0x3c5
	str	r1, [r0, #UCON_OFFSET]

	ldr	r1, =0x23
	str	r1, [r0, #UBRDIV_OFFSET]

	ldr	r1, =0x1
	str	r1, [r0, #UDIVSLOT_OFFSET]
send:
	ldr	r1, =0x4f
	str	r1, [r0, #UTXH_OFFSET]		@'O'

	ldr	r1, =0x4b
	str	r1, [r0, #UTXH_OFFSET]		@'K'

	ldr	r1, =0x0a
	str	r1, [r0, #UTXH_OFFSET]		@'K'
	b send

	mov pc, lr


