#include <init.h>
#include <sizes.h>
#include <io.h>
#include <asm/barebox-arm-head.h>
#include <asm/barebox-arm.h>
#include <mach/am33xx-silicon.h>
#include <mach/am33xx-clock.h>
#include <mach/generic.h>
#include <mach/sdrc.h>
#include <mach/sys_info.h>
#include <mach/syslib.h>
#include <mach/am33xx-mux.h>
#include <mach/am33xx-generic.h>
#include <mach/wdt.h>

#define DDR2_RD_DQS		0x12
#define DDR2_PHY_FIFO_WE	0x80
#define	DDR2_WR_DQS		0x00
#define	DDR2_PHY_WRLVL		0x00
#define	DDR2_PHY_GATELVL	0x00
#define	DDR2_PHY_WR_DATA	0x40

static const struct am33xx_cmd_control ddr2_cmd_ctrl = {
	.slave_ratio0	= 0x80,
	.dll_lock_diff0	= 0x0,
	.invert_clkout0	= 0x0,
	.slave_ratio1	= 0x80,
	.dll_lock_diff1	= 0x0,
	.invert_clkout1	= 0x0,
	.slave_ratio2	= 0x80,
	.dll_lock_diff2	= 0x0,
	.invert_clkout2	= 0x0,
};

static const struct am33xx_emif_regs ddr2_regs = {
	.emif_read_latency	= 0x5,
	.emif_tim1		= 0x0666B3D6,
	.emif_tim2		= 0x143731DA,
	.emif_tim3		= 0x00000347,
	.sdram_config		= 0x43805332,
	.sdram_config2		= 0x43805332,
	.sdram_ref_ctrl		= 0x0000081a,
};

static const struct am33xx_ddr_data ddr2_data = {
	.rd_slave_ratio0        = (DDR2_RD_DQS << 30) | (DDR2_RD_DQS << 20) |
				(DDR2_RD_DQS << 10) | (DDR2_RD_DQS << 0),
	.wr_dqs_slave_ratio0    = (DDR2_WR_DQS << 30) | (DDR2_WR_DQS << 20) |
				(DDR2_WR_DQS << 10) | (DDR2_WR_DQS << 0),
	.wrlvl_init_ratio0	= (DDR2_PHY_WRLVL << 30) |
				(DDR2_PHY_WRLVL << 20) |
				(DDR2_PHY_WRLVL << 10) |
				(DDR2_PHY_WRLVL << 0),
	.gatelvl_init_ratio0	= (DDR2_PHY_GATELVL << 30) |
				(DDR2_PHY_GATELVL << 20) |
				(DDR2_PHY_GATELVL << 10) |
				(DDR2_PHY_GATELVL << 0),
	.fifo_we_slave_ratio0	= (DDR2_PHY_FIFO_WE << 30) |
				(DDR2_PHY_FIFO_WE << 20) |
				(DDR2_PHY_FIFO_WE << 10) |
				(DDR2_PHY_FIFO_WE << 0),
	.wr_slave_ratio0        = (DDR2_PHY_WR_DATA << 30) |
				(DDR2_PHY_WR_DATA << 20) |
				(DDR2_PHY_WR_DATA << 10) |
				(DDR2_PHY_WR_DATA << 0),
	.use_rank0_delay	= 0x01,
	.dll_lock_diff0		= 0x0,
};

/**
 * @brief The basic entry point for board initialization.
 *
 * This is called as part of machine init (after arch init).
 * This is again called with stack in SRAM, so not too many
 * constructs possible here.
 *
 * @return void
 */
static int beaglebone_board_init(void)
{
	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	__raw_writel(WDT_DISABLE_CODE1, AM33XX_WDT_REG(WSPR));
	while(__raw_readl(AM33XX_WDT_REG(WWPS)) != 0x0);
	__raw_writel(WDT_DISABLE_CODE2, AM33XX_WDT_REG(WSPR));
	while(__raw_readl(AM33XX_WDT_REG(WWPS)) != 0x0);

	if (running_in_sdram())
		return 0;

	pll_init(MPUPLL_M_500, 24, DDRPLL_M_266);

	am335x_sdram_init(0x18B, &ddr2_cmd_ctrl, &ddr2_regs, &ddr2_data);

	am33xx_enable_uart0_pin_mux();

	return 0;
}

void __bare_init __naked barebox_arm_reset_vector(uint32_t *data)
{
	am33xx_save_bootinfo(data);

	arm_cpu_lowlevel_init();

	beaglebone_board_init();

	barebox_arm_entry(0x80000000, SZ_256M, 0);
}
