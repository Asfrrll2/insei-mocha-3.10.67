/*
 * arch/arm/mach-tegra/panel-s-wqxga-7-9-x6.c
 *
 * Copyright (c) 2014, XIAOMI CORPORATION. All rights reserved.
 * Copyright (C) 2016 XiaoMi, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http:
 */

#include <mach/dc.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/tegra_pwm_bl.h>
#include <linux/regulator/consumer.h>
#include <linux/pwm_backlight.h>
#include <linux/max8831_backlight.h>
#include <linux/platform_data/lp855x.h>
#include <linux/leds.h>
#include <linux/ioport.h>
#include <generated/mach-types.h>
#include "board.h"
#include "board-panel.h"
#include "devices.h"
#include "gpio-names.h"
#include "tegra11_host1x_devices.h"

#define TEGRA_DSI_GANGED_MODE	1

#define DSI_PANEL_RESET		1

#define DC_CTRL_MODE	(TEGRA_DC_OUT_CONTINUOUS_MODE | \
						TEGRA_DC_OUT_INITIALIZED_MODE)

static bool reg_requested;
static struct regulator *avdd_lcd_vsp_5v5;
static struct regulator *avdd_lcd_vsn_5v5;
static struct regulator *dvdd_lcd_1v8;
static u16 en_panel_rst;

static int ardbeg_dsi_regulator_get(struct device *dev)
{
	int err = 0;

	if (reg_requested)
		return 0;
	dvdd_lcd_1v8 = regulator_get(dev, "dvdd_lcdio");
	if (IS_ERR_OR_NULL(dvdd_lcd_1v8)) {
		err = PTR_ERR(dvdd_lcd_1v8);
		dvdd_lcd_1v8 = NULL;
	}
	avdd_lcd_vsp_5v5 = regulator_get(dev, "avdd_lcd");
	if (IS_ERR_OR_NULL(avdd_lcd_vsp_5v5)) {
		pr_err("avdd_lcd regulator get failed\n");
		err = PTR_ERR(avdd_lcd_vsp_5v5);
		avdd_lcd_vsp_5v5 = NULL;
	}
	avdd_lcd_vsn_5v5 = regulator_get(dev, "bvdd_lcd");
	if (IS_ERR_OR_NULL(avdd_lcd_vsn_5v5)) {
		pr_err("avdd_lcd regulator get failed\n");
		err = PTR_ERR(avdd_lcd_vsn_5v5);
		avdd_lcd_vsn_5v5 = NULL;
	}

	reg_requested = true;
	return 0;
}

static int dsi_s_wqxga_7_9_postpoweron(struct device *dev)
{
	return 0;
}

static int dsi_s_wqxga_7_9_enable(struct device *dev)
{
	int err = 0;

	err = ardbeg_dsi_regulator_get(dev);
	if (err < 0) {
		pr_err("dsi regulator get failed\n");
	}

	err = tegra_panel_gpio_get_dt("s,wqxga-7-9-x6", &panel_of);
	if (err < 0) {
		pr_err("dsi gpio request failed\n");
	}

	if (gpio_is_valid(panel_of.panel_gpio[TEGRA_GPIO_RESET]))
		en_panel_rst = panel_of.panel_gpio[TEGRA_GPIO_RESET];

	if (dvdd_lcd_1v8) {
		err = regulator_enable(dvdd_lcd_1v8);
		if (err < 0) {
			pr_err("dvdd_lcd regulator enable failed\n");
		}
		msleep(12);
	}

	if (avdd_lcd_vsp_5v5) {
		err = regulator_enable(avdd_lcd_vsp_5v5);
		if (err < 0) {
			pr_err("avdd_lcd regulator enable failed\n");
		}
		msleep(12);
	}

	if (avdd_lcd_vsn_5v5) {
		err = regulator_enable(avdd_lcd_vsn_5v5);
		if (err < 0) {
			pr_err("bvdd_lcd regulator enable failed\n");
		}
	}

	if (avdd_lcd_vsn_5v5)
		msleep(24);
#if DSI_PANEL_RESET
	pr_info("panel: %s\n", __func__);
	gpio_direction_output(en_panel_rst, 1);
	usleep_range(1000, 3000);
	gpio_set_value(en_panel_rst, 0);
	usleep_range(1000, 3000);
	gpio_set_value(en_panel_rst, 1);
	msleep(32);
#endif

	return 0;
}

static int dsi_s_wqxga_7_9_disable(struct device *dev)
{
	int err = 0;
	
	err = tegra_panel_gpio_get_dt("s,wqxga-7-9-x6", &panel_of);
	if (err < 0) {
		pr_err("dsi gpio request failed\n");
	}

	/* If panel rst gpio is specified in device tree,
	 * use that.
	 */
	if (gpio_is_valid(panel_of.panel_gpio[TEGRA_GPIO_RESET]))
		en_panel_rst = panel_of.panel_gpio[TEGRA_GPIO_RESET];
		
	pr_info("panel: %s\n", __func__);
	gpio_set_value(en_panel_rst, 0);
	msleep(10);
	if (avdd_lcd_vsn_5v5)
		regulator_disable(avdd_lcd_vsn_5v5);
	msleep(10);
	if (avdd_lcd_vsp_5v5)
		regulator_disable(avdd_lcd_vsp_5v5);
	msleep(10);
	if (dvdd_lcd_1v8)
		regulator_disable(dvdd_lcd_1v8);

	return 0;
}

static int dsi_s_wqxga_7_9_postsuspend(void)
{
	int err = 0;
	err = tegra_panel_gpio_get_dt("s,wqxga-7-9-x6", &panel_of);
	if (err < 0) {
		pr_err("dsi gpio request failed\n");
	}

	/* If panel rst gpio is specified in device tree,
	 * use that.
	 */
	if (gpio_is_valid(panel_of.panel_gpio[TEGRA_GPIO_RESET]))
		en_panel_rst = panel_of.panel_gpio[TEGRA_GPIO_RESET];
		
	pr_info("%s\n", __func__);
	gpio_set_value(en_panel_rst, 0);
	return 0;
}

static int dsi_s_wqxga_7_9_bl_notify(struct device *dev, int brightness)
{
	struct backlight_device *bl = NULL;
	struct pwm_bl_data *pb = NULL;
	int cur_sd_brightness = atomic_read(&sd_brightness);
	bl = (struct backlight_device *)dev_get_drvdata(dev);
	pb = (struct pwm_bl_data *)dev_get_drvdata(&bl->dev);

	/* SD brightness is a percentage */
	brightness = (brightness * cur_sd_brightness) / 255;

	/* Apply any backlight response curve */
	if (brightness > 255)
		pr_info("Error: Brightness > 255!\n");
	else if (pb->bl_measured)
		brightness = pb->bl_measured[brightness];

	return brightness;
}

static int dsi_s_wqxga_7_9_check_fb(struct device *dev, struct fb_info *info)
{
	struct platform_device *pdev = NULL;
	pdev = to_platform_device(bus_find_device_by_name(
		&platform_bus_type, NULL, "tegradc.0"));
	return info->device == &pdev->dev;
}

static struct pwm_bl_data_dt_ops  dsi_s_wqxga_7_9_pwm_bl_ops = {
	.notify = dsi_s_wqxga_7_9_bl_notify,
	.check_fb = dsi_s_wqxga_7_9_check_fb,
	.blnode_compatible = "ti,lp8556",
};

struct tegra_panel_ops dsi_s_wqxga_7_9_x6_ops = {
	.enable = dsi_s_wqxga_7_9_enable,
	.postpoweron = dsi_s_wqxga_7_9_postpoweron,
	.postsuspend = dsi_s_wqxga_7_9_postsuspend,
	.disable = dsi_s_wqxga_7_9_disable,
	.pwm_bl_ops = &dsi_s_wqxga_7_9_pwm_bl_ops,
};

