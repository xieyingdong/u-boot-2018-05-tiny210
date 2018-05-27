// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2008-2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/sromc.h>
#include <netdev.h>
#include <asm/mach-types.h>
#include <asm/arch/dmc.h>          /*add by shl*/

#ifndef CONFIG_SPL_BUILD      /*add by shl*/

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */
static void smc9115_pre_init(void)
{
	u32 smc_bw_conf, smc_bc_conf;

	/* gpio configuration GPK0CON */
	gpio_cfg_pin(S5PC100_GPIO_K00 + CONFIG_ENV_SROM_BANK, S5P_GPIO_FUNC(2));

	/* Ethernet needs bus width of 16 bits */
	smc_bw_conf = SMC_DATA16_WIDTH(CONFIG_ENV_SROM_BANK);
	smc_bc_conf = SMC_BC_TACS(0x0) | SMC_BC_TCOS(0x4) | SMC_BC_TACC(0xe)
			| SMC_BC_TCOH(0x1) | SMC_BC_TAH(0x4)
			| SMC_BC_TACP(0x6) | SMC_BC_PMC(0x0);

	/* Select and configure the SROMC bank */
	s5p_config_sromc(CONFIG_ENV_SROM_BANK, smc_bw_conf, smc_bc_conf);
}

int board_init(void)
{
	smc9115_pre_init();

	gd->bd->bi_arch_number = MACH_TYPE_TINY210;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board:\tSMDKC100\n");
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

#else     /*CONFIG_SPL_BUILD (add by shl)*/

void ddr_init(void)
{
	struct s5pv210_dmc0 *const dmc0 = (struct s5pv210_dmc0 *)samsung_get_base_dmc0();
	struct s5pv210_dmc1 *const dmc1 = (struct s5pv210_dmc1 *)samsung_get_base_dmc1();
	
	/* DMC0 */
	writel(0x00101000, &dmc0->phycontrol0);
	writel(0x00101002, &dmc0->phycontrol0);			/* DLL on */
	writel(0x00000086, &dmc0->phycontrol1);
	writel(0x00101003, &dmc0->phycontrol0);			/* DLL start */
	
	while ((readl(&dmc0->phystatus) & 0x7) != 0x7); /* wait DLL locked */

	writel(0x0FFF2350, &dmc0->concontrol);			/* Auto Refresh Counter should be off */
	writel(0x00202430, &dmc0->memcontrol);			/* Dynamic power down should be off */
	writel(0x20E01323, &dmc0->memconfig0);
	
	writel(0xFF000000, &dmc0->prechconfig);
	writel(0xFFFF00FF, &dmc0->pwrdnconfig);
	
	writel(0x00000618, &dmc0->timingaref);			/* 7.8us * 200MHz = 1560 = 0x618  */
	writel(0x19233309, &dmc0->timingrow);
	writel(0x23240204, &dmc0->timingdata);
	writel(0x09C80232, &dmc0->timingpower);
	
	writel(0x07000000, &dmc0->directcmd);			/* NOP */
	writel(0x01000000, &dmc0->directcmd);			/* PALL */
	writel(0x00020000, &dmc0->directcmd);			/* EMRS2 */
	writel(0x00030000, &dmc0->directcmd);			/* EMRS3 */
	writel(0x00010400, &dmc0->directcmd);			/* EMRS enable DLL */
	writel(0x00000542, &dmc0->directcmd);			/* DLL reset */
	writel(0x01000000, &dmc0->directcmd); 			/* PALL */
	writel(0x05000000, &dmc0->directcmd);			/* auto refresh */
	writel(0x05000000, &dmc0->directcmd);			/* auto refresh */
	writel(0x00000442, &dmc0->directcmd);			/* DLL unreset */
	writel(0x00010780, &dmc0->directcmd);			/* OCD default */
	writel(0x00010400, &dmc0->directcmd);			/* OCD exit */
	
	writel(0x0FF02030, &dmc0->concontrol);			/* auto refresh on */
	writel(0xFFFF00FF, &dmc0->pwrdnconfig);
	writel(0x00202400, &dmc0->memcontrol);

	/* DMC1 */
	writel(0x00101000, &dmc1->phycontrol0);
	writel(0x00101002, &dmc1->phycontrol0);			/* DLL on */
	writel(0x00000086, &dmc1->phycontrol1);
	writel(0x00101003, &dmc1->phycontrol0);			/* DLL start */

	while ((readl(&dmc1->phystatus) & 0x7) != 0x7); /* wait DLL locked */

	writel(0x0FFF2350, &dmc1->concontrol);			/* Auto Refresh Counter should be off */
	writel(0x00202430, &dmc1->memcontrol);			/* Dynamic power down should be off */
	writel(0x40E01323, &dmc1->memconfig0);
	
	writel(0xFF000000, &dmc1->prechconfig);
	writel(0xFFFF00FF, &dmc1->pwrdnconfig);
	
	writel(0x00000618, &dmc1->timingaref);			/* 7.8us * 200MHz = 1560 = 0x618  */
	writel(0x19233309, &dmc1->timingrow);
	writel(0x23240204, &dmc1->timingdata);
	writel(0x09C80232, &dmc1->timingpower);
	
	writel(0x07000000, &dmc1->directcmd);			/* NOP */
	writel(0x01000000, &dmc1->directcmd);			/* PALL */
	writel(0x00020000, &dmc1->directcmd);			/* EMRS2 */
	writel(0x00030000, &dmc1->directcmd);			/* EMRS3 */
	writel(0x00010400, &dmc1->directcmd);			/* EMRS enable DLL */
	writel(0x00000542, &dmc1->directcmd);			/* DLL reset */
	writel(0x01000000, &dmc1->directcmd); 			/* PALL */
	writel(0x05000000, &dmc1->directcmd);			/* auto refresh */
	writel(0x05000000, &dmc1->directcmd);			/* auto refresh */
	writel(0x00000442, &dmc1->directcmd);			/* DLL unreset */
	writel(0x00010780, &dmc1->directcmd);			/* OCD default */
	writel(0x00010400, &dmc1->directcmd);			/* OCD exit */
	
	writel(0x0FF02030, &dmc1->concontrol);			/* auto refresh on */
	writel(0xFFFF00FF, &dmc1->pwrdnconfig);
	writel(0x00202400, &dmc1->memcontrol);
}

void copy_bl2_to_ram(void)
{
/*
** ch:  通道
** sb:  起始块
** bs:  块大小
** dst: 目的地
** i: 	是否初始化
*/
#define CopySDMMCtoMem(ch, sb, bs, dst, i) \
	(((unsigned char(*)(int, unsigned int, unsigned short, unsigned int*, unsigned char))\
	(*((unsigned int *)0xD0037F98)))(ch, sb, bs, dst, i))
	
	unsigned int V210_SDMMC_BASE = *(volatile unsigned int *)(0xD0037488);	// V210_SDMMC_BASE
	unsigned char ch = 0;
	
	/* 参考S5PV210手册7.9.1 SD/MMC REGISTER MAP */
	if (V210_SDMMC_BASE == 0xEB000000)		// 通道0
		ch = 0;
	else if (V210_SDMMC_BASE == 0xEB200000)	// 通道2
		ch = 2;
	
	CopySDMMCtoMem(ch, 32, 400, (unsigned int *)CONFIG_SYS_SDRAM_BASE, 0);
}

#endif /*CONFIG_SPL_BUILD (add by shl)*/
