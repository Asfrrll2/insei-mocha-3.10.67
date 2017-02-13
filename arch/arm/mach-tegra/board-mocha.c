/*
 * arch/arm/mach-tegra/board-ardbeg.c
 *
 * Copyright (c) 2013-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c/i2c-hid.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/i2c-tegra.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/spi/spi.h>
#include <linux/spi/rm31080a_ts.h>
#include <linux/maxim_sti.h>
#include <linux/memblock.h>
#include <linux/spi/spi-tegra.h>
#include <linux/nfc/pn544.h>
#include <linux/rfkill-gpio.h>
#include <linux/skbuff.h>
#include <linux/ti_wilink_st.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/smb349-charger.h>
#include <linux/max17048_battery.h>
#include <linux/leds.h>
#include <linux/i2c/at24.h>
#include <linux/of_platform.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/platform_data/serial-tegra.h>
#include <linux/edp.h>
#include <linux/usb/tegra_usb_phy.h>
#include <linux/mfd/palmas.h>
#include <linux/clk/tegra.h>
#include <linux/clocksource.h>
#include <linux/irqchip.h>
#include <linux/irqchip/tegra.h>
#include <linux/tegra-soc.h>
#include <linux/platform_data/tegra_usb_modem_power.h>
#include <linux/platform_data/tegra_ahci.h>
#include <linux/irqchip/tegra.h>
#include <sound/max98090.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf-tegra.h>

#include <mach/irqs.h>
#include <mach/io_dpd.h>
#include <mach/i2s.h>
#include <linux/platform/tegra/isomgr.h>
#include <mach/tegra_asoc_pdata.h>
#include <mach/dc.h>
#include <mach/tegra_usb_pad_ctrl.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/xusb.h>

#include "board.h"
#include "board-mocha.h"
#include "board-common.h"
#include "board-panel.h"
#include "board-touch-raydium.h"
#include "board-touch-maxim_sti.h"
#include <linux/platform/tegra/clock.h>
#include <linux/platform/tegra/common.h>
#include "devices.h"
#include "gpio-names.h"
#include "iomap.h"
#include "pm.h"
#include "tegra-board-id.h"
#include "tegra-of-dev-auxdata.h"

static struct board_info board_info, display_board_info;

#ifdef CONFIG_BLUEDROID_PM
static struct resource mocha_bluedroid_pm_resources[] = {
	[0] = {
		.name   = "shutdown_gpio",
		.start  = TEGRA_GPIO_PR1,
		.end    = TEGRA_GPIO_PR1,
		.flags  = IORESOURCE_IO,
	},
	[1] = {
		.name = "host_wake",
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
	[2] = {
		.name = "gpio_ext_wake",
		.start  = TEGRA_GPIO_PEE1,
		.end    = TEGRA_GPIO_PEE1,
		.flags  = IORESOURCE_IO,
	},
	[3] = {
		.name = "gpio_host_wake",
		.start  = TEGRA_GPIO_PU6,
		.end    = TEGRA_GPIO_PU6,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device mocha_bluedroid_pm_device = {
	.name = "bluedroid_pm",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(mocha_bluedroid_pm_resources),
	.resource       = mocha_bluedroid_pm_resources,
};

static noinline void __init mocha_setup_bluedroid_pm(void)
{
	mocha_bluedroid_pm_resources[1].start =
		mocha_bluedroid_pm_resources[1].end =
				gpio_to_irq(TEGRA_GPIO_PU6);
	platform_device_register(&mocha_bluedroid_pm_device);
}
#endif

static __initdata struct tegra_clk_init_table mocha_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
	{ "hda",	"pll_p",	108000000,	false},
	{ "hda2codec_2x", "pll_p",	48000000,	false},
	{ "pwm",	"pll_p",	48000000,	false},
	{ "i2s1",	"pll_a_out0",	0,		false},
	{ "i2s3",	"pll_a_out0",	0,		false},
	{ "i2s4",	"pll_a_out0",	0,		false},
	{ "spdif_out",	"pll_a_out0",	0,		false},
	{ "d_audio",	"pll_a_out0",	12288000,	false},
	{ "dam0",	"clk_m",	12000000,	false},
	{ "dam1",	"clk_m",	12000000,	false},
	{ "dam2",	"clk_m",	12000000,	false},
	{ "audio1",	"i2s1_sync",	0,		false},
	{ "audio3",	"i2s3_sync",	0,		false},
	{ "vi_sensor",	"pll_p",	150000000,	false},
	{ "vi_sensor2",	"pll_p",	150000000,	false},
	{ "cilab",	"pll_p",	150000000,	false},
	{ "cilcd",	"pll_p",	150000000,	false},
	{ "cile",	"pll_p",	150000000,	false},
	{ "i2c1",	"pll_p",	3200000,	false},
	{ "i2c2",	"pll_p",	3200000,	false},
	{ "i2c3",	"pll_p",	3200000,	false},
	{ "i2c4",	"pll_p",	3200000,	false},
	{ "i2c5",	"pll_p",	3200000,	false},
	{ "sbc1",	"pll_p",	25000000,	false},
	{ "sbc2",	"pll_p",	25000000,	false},
	{ "sbc3",	"pll_p",	25000000,	false},
	{ "sbc4",	"pll_p",	25000000,	false},
	{ "sbc5",	"pll_p",	25000000,	false},
	{ "sbc6",	"pll_p",	25000000,	false},
	{ "uarta",	"pll_p",	408000000,	false},
	{ "uartb",	"pll_p",	408000000,	false},
	{ "uartc",	"pll_p",	408000000,	false},
	{ "uartd",	"pll_p",	408000000,	false},
	{ NULL,		NULL,		0,		0},
};

static void mocha_i2c_init(void)
{
	struct board_info board_info;
	tegra_get_board_info(&board_info);

    /* i2c_register_board_info(0, audio_board_info, ARRAY_SIZE(audio_board_info)); */
}

static struct platform_device *mocha_devices[] __initdata = {
	&tegra_rtc_device,
#if defined(CONFIG_CRYPTO_DEV_TEGRA_SE) && !defined(CONFIG_USE_OF)
	&tegra12_se_device,
#endif
	&tegra_ahub_device,
	&tegra_dam_device0,
	&tegra_dam_device1,
	&tegra_dam_device2,
	&tegra_i2s_device0,
	&tegra_i2s_device1,
	&tegra_i2s_device3,
	&tegra_i2s_device4,
	&tegra_spdif_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&baseband_dit_device,
	&tegra_offload_device,
	&tegra30_avp_audio_device,
};

static struct tegra_usb_platform_data tegra_udc_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_DEVICE,
	.u_data.dev = {
		.vbus_pmu_irq = 0,
		.charging_supported = true,
		.remote_wakeup_supported = false,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

#if !defined(CONFIG_ARM64)
static struct tegra_usb_platform_data tegra_ehci1_utmi_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.hot_plug = false,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 15,
		.xcvr_lsfslew = 0,
		.xcvr_lsrslew = 3,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
		.vbus_oc_map = 0x4,
		.xcvr_hsslew_lsb = 2,
	},
};

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
};

#else
static struct tegra_usb_otg_data tegra_otg_pdata;
#endif

static void mocha_usb_init(void)
{
#if !defined(CONFIG_ARM64)
	int usb_port_owner_info = tegra_get_usb_port_owner_info();
#endif
	struct board_info bi;
	tegra_get_pmu_board_info(&bi);

	/* ST8 is supported through DT, return */
	if (board_info.board_id == BOARD_P1761 ||
			of_machine_is_compatible("nvidia,green-arrow"))
		return;

#if !defined(CONFIG_ARM64)
	if (board_info.sku == 1100 || board_info.board_id == BOARD_P1761 ||
					board_info.board_id == BOARD_E1784)
		tegra_ehci1_utmi_pdata.u_data.host.turn_off_vbus_on_lp0 = true;

		/* MOCHA */

			/*
			 * Set the maximum voltage that can be supplied
			 * over USB vbus that the board supports if we use
			 * a quick charge 2 wall charger.
			 */
			tegra_udc_pdata.qc2_voltage = TEGRA_USB_QC2_9V;
			tegra_udc_pdata.u_data.dev.qc2_current_limit_ma = 1200;

			/* charger needs to be set to 3A - h/w will do 2A */
			tegra_udc_pdata.u_data.dev.dcp_current_limit_ma = 3000;


	if (!(usb_port_owner_info & UTMI1_PORT_OWNER_XUSB)) {
		tegra_otg_pdata.is_xhci = false;
		tegra_udc_pdata.u_data.dev.is_xhci = false;
	} else {
		tegra_otg_pdata.is_xhci = true;
		tegra_udc_pdata.u_data.dev.is_xhci = true;
	}

	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);
#endif

	/* Setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

}

static struct tegra_xusb_platform_data xusb_pdata = {
	.portmap = TEGRA_XUSB_SS_P0 | TEGRA_XUSB_USB2_P0 | TEGRA_XUSB_SS_P1 |
			TEGRA_XUSB_USB2_P1 | TEGRA_XUSB_USB2_P2,
};

#ifdef CONFIG_TEGRA_XUSB_PLATFORM
static void mocha_xusb_init(void)
{
	int usb_port_owner_info = tegra_get_usb_port_owner_info();

	xusb_pdata.lane_owner = (u8) tegra_get_lane_owner_info();
	pr_info("Shield ERS 0x%x\n", board_info.board_id);
	/* Shield ERS */
	if (!(usb_port_owner_info & UTMI1_PORT_OWNER_XUSB))
		xusb_pdata.portmap &= ~(TEGRA_XUSB_USB2_P0 |
			TEGRA_XUSB_SS_P0);

	if (!(usb_port_owner_info & UTMI2_PORT_OWNER_XUSB))
		xusb_pdata.portmap &= ~(TEGRA_XUSB_USB2_P1 |
			TEGRA_XUSB_USB2_P2 | TEGRA_XUSB_SS_P1);


	if (usb_port_owner_info & HSIC1_PORT_OWNER_XUSB)
		xusb_pdata.portmap |= TEGRA_XUSB_HSIC_P0;

	if (usb_port_owner_info & HSIC2_PORT_OWNER_XUSB)
		xusb_pdata.portmap |= TEGRA_XUSB_HSIC_P1;
}
#endif

#ifdef CONFIG_USE_OF
static struct of_dev_auxdata mocha_auxdata_lookup[] __initdata = {
	OF_DEV_AUXDATA("nvidia,tegra124-se", 0x70012000, "tegra12-se", NULL),
	OF_DEV_AUXDATA("nvidia,tegra132-dtv", 0x7000c300, "dtv", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-dtv", 0x7000c300, "dtv", NULL),
#if defined(CONFIG_ARM64)
	OF_DEV_AUXDATA("nvidia,tegra132-udc", 0x7d000000, "tegra-udc.0",
			&tegra_udc_pdata.u_data.dev),
	OF_DEV_AUXDATA("nvidia,tegra132-otg", 0x7d000000, "tegra-otg",
			&tegra_otg_pdata),
	OF_DEV_AUXDATA("nvidia,tegra132-ehci", 0x7d004000, "tegra-ehci.1",
			NULL),
	OF_DEV_AUXDATA("nvidia,tegra132-ehci", 0x7d008000, "tegra-ehci.2",
			NULL),
#endif
	OF_DEV_AUXDATA("nvidia,tegra124-udc", TEGRA_USB_BASE, "tegra-udc.0",
			NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-otg", TEGRA_USB_BASE, "tegra-otg",
			NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-host1x", TEGRA_HOST1X_BASE, "host1x",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-gk20a", TEGRA_GK20A_BAR0_BASE,
		"gk20a.0", NULL),
#ifdef CONFIG_ARCH_TEGRA_VIC
	OF_DEV_AUXDATA("nvidia,tegra124-vic", TEGRA_VIC_BASE, "vic03.0", NULL),
#endif
	OF_DEV_AUXDATA("nvidia,tegra124-msenc", TEGRA_MSENC_BASE, "msenc",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-vi", TEGRA_VI_BASE, "vi", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-isp", TEGRA_ISP_BASE, "isp.0", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-isp", TEGRA_ISPB_BASE, "isp.1", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-pwm", TEGRA_PWFM_BASE, "tegra-pwm", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-tsec", TEGRA_TSEC_BASE, "tsec", NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-xhci", 0x70090000, "tegra-xhci",
				&xusb_pdata),
	OF_DEV_AUXDATA("nvidia,tegra132-xhci", 0x70090000, "tegra-xhci",
				&xusb_pdata),
	OF_DEV_AUXDATA("nvidia,tegra124-dc", TEGRA_DISPLAY_BASE, "tegradc.0",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-dc", TEGRA_DISPLAY2_BASE, "tegradc.1",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-nvavp", 0x60001000, "nvavp",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-dfll", 0x70110000, "tegra_cl_dvfs",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra132-dfll", 0x70040084, "tegra_cl_dvfs",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-efuse", TEGRA_FUSE_BASE, "tegra-fuse",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-camera", 0, "pcl-generic",
				NULL),
	OF_DEV_AUXDATA("nvidia,tegra124-ahci-sata", 0x70027000, "tegra-sata.0",
		NULL),
	OF_DEV_AUXDATA("nvidia,tegra-bluedroid_pm", 0, "bluedroid_pm",
		NULL),
	OF_DEV_AUXDATA("pwm-backlight", 0, "pwm-backlight", NULL),
#ifdef CONFIG_TEGRA_CEC_SUPPORT
	OF_DEV_AUXDATA("nvidia,tegra124-cec", 0x70015000, "tegra_cec", NULL),
#endif
	OF_DEV_AUXDATA("nvidia,tegra-audio-rt5639", 0x0, "tegra-snd-rt5639",
		NULL),
	OF_DEV_AUXDATA("nvidia,icera-i500", 0, "tegra_usb_modem_power", NULL),
	OF_DEV_AUXDATA("nvidia,ptm", 0x7081c000, "ptm", NULL),
	OF_DEV_AUXDATA("nvidia,tegra30-hda", 0x70030000, "tegra30-hda", NULL),
	{}
};
#endif

static int __init mocha_touch_init(void)
{
  	return 0; // Fix me
}

static void __init mocha_sysedp_init(void)
{
	struct board_info bi;

	tegra_get_board_info(&bi);

	switch (bi.board_id) {
	case BOARD_E1780:
		if (bi.sku == 1100)
			tn8_new_sysedp_init();
		break;
	default:
		break;
	}
}

static void __init mocha_sysedp_dynamic_capping_init(void)
{
	struct board_info bi;

	tegra_get_board_info(&bi);

	switch (bi.board_id) {
	case BOARD_E1780:
		if (bi.sku == 1100)
			tn8_sysedp_dynamic_capping_init();
		break;
	default:
		break;
	}
}

static void __init tegra_mocha_early_init(void)
{
	mocha_sysedp_init();
	tegra_clk_init_from_table(mocha_clk_init_table);
	tegra_clk_verify_parents();
	tegra_soc_device_init("mocha");
}

static struct tegra_io_dpd pexbias_io = {
	.name			= "PEX_BIAS",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 4,
};
static struct tegra_io_dpd pexclk1_io = {
	.name			= "PEX_CLK1",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 5,
};
static struct tegra_io_dpd pexclk2_io = {
	.name			= "PEX_CLK2",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 6,
};

static void __init tegra_mocha_late_init(void)
{
	struct board_info board_info;
	tegra_get_board_info(&board_info);
	pr_info("board_info: id:sku:fab:major:minor = 0x%04x:0x%04x:0x%02x:0x%02x:0x%02x\n",
		board_info.board_id, board_info.sku,
		board_info.fab, board_info.major_revision,
		board_info.minor_revision);

#ifndef CONFIG_MACH_EXUMA
	tegra_disp_defer_vcore_override();
#endif
	mocha_usb_init();
	
#ifdef CONFIG_TEGRA_XUSB_PLATFORM
	mocha_xusb_init();
#endif
	mocha_i2c_init();

	platform_add_devices(mocha_devices, ARRAY_SIZE(mocha_devices));

	tegra_io_dpd_init();
	mocha_sdhci_init();
	mocha_regulator_init();
	mocha_suspend_init();
	mocha_emc_init();
	isomgr_init();
	mocha_touch_init();

		/* put PEX pads into DPD mode to save additional power */
		tegra_io_dpd_enable(&pexbias_io);
		tegra_io_dpd_enable(&pexclk1_io);
		tegra_io_dpd_enable(&pexclk2_io);
	mocha_sensors_init();
	mocha_soctherm_init();

#ifdef CONFIG_BLUEDROID_PM
	mocha_setup_bluedroid_pm();
#endif
	mocha_sysedp_dynamic_capping_init();
}

static void __init tegra_mocha_init_early(void)
{
	tegra_get_board_info(&board_info);
	tegra12x_init_early();
}

static int tegra_mocha_notifier_call(struct notifier_block *nb,
				    unsigned long event, void *data)
{
#ifndef CONFIG_TEGRA_HDMI_PRIMARY
	struct device *dev = data;
#endif
	switch (event) {
	case BUS_NOTIFY_BIND_DRIVER:
#ifndef CONFIG_TEGRA_HDMI_PRIMARY
		if (dev->of_node) {
			if (of_device_is_compatible(dev->of_node,
				"pwm-backlight")) {
				tegra_pwm_bl_ops_register(dev);
			}
		}
#endif
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}


static int tegra_mocha_i2c_notifier_call(struct notifier_block *nb,
			unsigned long event, void *data)
{
#ifndef CONFIG_TEGRA_HDMI_PRIMARY
	struct device *dev = data;
#endif

	switch (event) {
	case BUS_NOTIFY_BIND_DRIVER:
#ifndef CONFIG_TEGRA_HDMI_PRIMARY
		if (dev->of_node) {
			if (of_device_is_compatible(dev->of_node,
					"ti,lp8550") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8551") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8552") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8553") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8554") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8555") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8556") ||
				of_device_is_compatible(dev->of_node,
					"ti,lp8557")) {
					ti_lp855x_bl_ops_register(dev);
			}
		}
#endif
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block platform_nb = {
	.notifier_call = tegra_mocha_notifier_call,
};


static struct notifier_block i2c_nb = {
	.notifier_call = tegra_mocha_i2c_notifier_call,
};

static void __init tegra_mocha_dt_init(void)
{
	if(of_machine_is_compatible("nvidia,green-arrow"))
		regulator_has_full_constraints();

	tegra_get_board_info(&board_info);
	tegra_get_display_board_info(&display_board_info);
	bus_register_notifier(&platform_bus_type, &platform_nb);
	bus_register_notifier(&i2c_bus_type, &i2c_nb);
	tegra_mocha_early_init();
#ifdef CONFIG_USE_OF
	mocha_camera_auxdata(mocha_auxdata_lookup);
	of_platform_populate(NULL,
		of_default_bus_match_table, mocha_auxdata_lookup,
		&platform_bus);
#endif

	tegra_mocha_late_init();
}

static void __init tegra_mocha_reserve(void)
{
#if defined(CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM) || \
		defined(CONFIG_TEGRA_NO_CARVEOUT)
	ulong carveout_size = 0;
#else
	ulong carveout_size = SZ_1G;
#endif
	ulong vpr_size = 186 * SZ_1M;

	tegra_reserve4(carveout_size, 0, 0, vpr_size);
}

static const char * const mocha_dt_board_compat[] = {
	"nvidia,mocha",
	NULL
};

DT_MACHINE_START(MOCHA, "mocha")
	.atag_offset	= 0x100,
	.smp		= smp_ops(tegra_smp_ops),
	.map_io		= tegra_map_common_io,
	.reserve	= tegra_mocha_reserve,
	.init_early	= tegra_mocha_init_early,
	.init_irq	= irqchip_init,
	.init_time	= clocksource_of_init,
	.init_machine	= tegra_mocha_dt_init,
	.dt_compat	= mocha_dt_board_compat,
	.init_late      = tegra_init_late
MACHINE_END
