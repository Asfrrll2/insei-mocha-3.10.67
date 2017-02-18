/*
 * arch/arm/mach-tegra/board-ardbeg-sensors.c
 *
 * Copyright (c) 2013-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/mpu_iio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/nct1008.h>
#include <linux/pid_thermal_gov.h>
#include <linux/tegra-fuse.h>
#include <linux/of_platform.h>
#include <mach/edp.h>
#include <mach/io_dpd.h>
#include <media/camera.h>
#include <media/ar0261.h>
#include <media/imx135.h>
#include <media/imx214.h>
#include <media/imx208.h>
#include <media/imx219.h>
#include <media/imx179.h>
#include <media/dw9718.h>
#include <media/dw9714.h>
#include <media/as364x.h>
#include <media/ov5693.h>
#include <media/ov7695.h>
#include <media/mt9m114.h>
#include <media/ad5823.h>
#include <media/max77387.h>

#include <linux/platform_device.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#include <media/tegra_v4l2_camera.h>

#include <linux/platform/tegra/cpu-tegra.h>
#include "devices.h"
#include "board.h"
#include "board-common.h"
#include "board-mocha.h"
#include "tegra-board-id.h"

#if defined(ARCH_TEGRA_12x_SOC)
static struct i2c_board_info mocha_i2c_board_info_cm32181[] = {
	{
		I2C_BOARD_INFO("cm32181", 0x48),
	},
};
#endif


/*
 * Soc Camera platform driver for testing //FIXME this is a clear code
 */


static struct tegra_io_dpd csia_io = {
	.name			= "CSIA",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 0,
};

static struct tegra_io_dpd csib_io = {
	.name			= "CSIB",
	.io_dpd_reg_index	= 0,
	.io_dpd_bit		= 1,
};

static struct tegra_io_dpd csie_io = {
	.name			= "CSIE",
	.io_dpd_reg_index	= 1,
	.io_dpd_bit		= 12,
};

static struct camera_data_blob mocha_camera_lut[] = {
	/*{"ardbeg_imx135_pdata", &ardbeg_imx135_data},
	{"ardbeg_dw9718_pdata", &ardbeg_dw9718_data},
	{"ardbeg_ar0261_pdata", &ardbeg_ar0261_data},
	{"ardbeg_mt9m114_pdata", &ardbeg_mt9m114_pdata},
	{"ardbeg_ov5693_pdata", &ardbeg_ov5693_pdata},
	{"ardbeg_ad5823_pdata", &ardbeg_ad5823_pdata},
	{"ardbeg_as3648_pdata", &ardbeg_as3648_data},
	{"ardbeg_ov7695_pdata", &ardbeg_ov7695_pdata},
	{"ardbeg_imx179_pdata", &ardbeg_imx179_data},
	{"ardbeg_imx219_pdata", &ardbeg_imx219_data},
	{"ardbeg_ov5693f_pdata", &ardbeg_ov5693_front_pdata},
	{"ardbeg_imx214_pdata", &ardbeg_imx214_data},
	{"ardbeg_imx208_pdata", &ardbeg_imx208_data},		FIXME
	{"ardbeg_dw9714_pdata", &ardbeg_dw9714_data},*/
	{},
};


void __init mocha_camera_auxdata(void *data)
{
	struct of_dev_auxdata *aux_lut = data;
	while (aux_lut && aux_lut->compatible) {
		if (!strcmp(aux_lut->compatible, "nvidia,tegra124-camera")) {
			pr_info("%s: update camera lookup table.\n", __func__);
			aux_lut->platform_data = mocha_camera_lut;
		}
		aux_lut++;
	}
}

static int mocha_camera_init(void)
{
	struct board_info board_info;

	pr_debug("%s: ++\n", __func__);
	tegra_get_board_info(&board_info);

	/* put CSIA/B/C/D/E IOs into DPD mode to
	 * save additional power for mocha
	 */
	tegra_io_dpd_enable(&csia_io);
	tegra_io_dpd_enable(&csib_io);
	tegra_io_dpd_enable(&csie_io);

	return 0;
}

static struct pid_thermal_gov_params cpu_pid_params = {
	.max_err_temp = 4000,
	.max_err_gain = 1000,

	.gain_p = 1000,
	.gain_d = 0,

	.up_compensation = 15,
	.down_compensation = 15,
};

static struct thermal_zone_params cpu_tzp = {
	.governor_name = "pid_thermal_gov",
	.governor_params = &cpu_pid_params,
};

static struct thermal_zone_params board_tzp = {
	.governor_name = "pid_thermal_gov"
};

static struct nct1008_platform_data mocha_nct72_pdata = {
	.loc_name = "tegra",
	.supported_hwrev = true,
	.conv_rate = 0x06, /* 4Hz conversion rate */
	.offset = 0,
	.extended_range = true,

	.sensors = {
		[LOC] = {
			.tzp = &board_tzp,
			.shutdown_limit = 120, /* C */
			.passive_delay = 1000,
			.num_trips = 1,
			.trips = {
				{
					.cdev_type = "therm_est_activ",
					.trip_temp = 40000,
					.trip_type = THERMAL_TRIP_ACTIVE,
					.hysteresis = 1000,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
					.mask = 1,
				},
			},
		},
		[EXT] = {
			.tzp = &cpu_tzp,
			.shutdown_limit = 95, /* C */
			.passive_delay = 1000,
			.num_trips = 2,
			.trips = {
				{
					.cdev_type = "shutdown_warning",
					.trip_temp = 93000,
					.trip_type = THERMAL_TRIP_PASSIVE,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
					.mask = 0,
				},
				{
					.cdev_type = "cpu-balanced",
					.trip_temp = 83000,
					.trip_type = THERMAL_TRIP_PASSIVE,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
					.hysteresis = 1000,
					.mask = 1,
				},
			}
		}
	}
};

#ifdef CONFIG_TEGRA_SKIN_THROTTLE
static struct pid_thermal_gov_params skin_pid_params = {
	.max_err_temp = 4000,
	.max_err_gain = 1000,

	.gain_p = 1000,
	.gain_d = 0,

	.up_compensation = 15,
	.down_compensation = 15,
};

static struct thermal_zone_params skin_tzp = {
	.governor_name = "pid_thermal_gov",
	.governor_params = &skin_pid_params,
};

static struct nct1008_platform_data mocha_nct72_tskin_pdata = {
	.loc_name = "skin",

	.supported_hwrev = true,
	.conv_rate = 0x06, /* 4Hz conversion rate */
	.offset = 0,
	.extended_range = true,

	.sensors = {
		[LOC] = {
			.shutdown_limit = 95, /* C */
			.num_trips = 0,
			.tzp = NULL,
		},
		[EXT] = {
			.shutdown_limit = 85, /* C */
			.passive_delay = 10000,
			.polling_delay = 1000,
			.tzp = &skin_tzp,
			.num_trips = 1,
			.trips = {
				{
					.cdev_type = "skin-balanced",
					.trip_temp = 50000,
					.trip_type = THERMAL_TRIP_PASSIVE,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
					.mask = 1,
				},
			},
		}
	}
};
#endif

static struct i2c_board_info mocha_i2c_nct72_board_info[] = {
	{
		I2C_BOARD_INFO("nct72", 0x4c),
		.platform_data = &mocha_nct72_pdata,
		.irq = -1,
	},
#ifdef CONFIG_TEGRA_SKIN_THROTTLE
	{
		I2C_BOARD_INFO("nct72", 0x4d),
		.platform_data = &mocha_nct72_tskin_pdata,
		.irq = -1,
	}
#endif
};

static int mocha_nct72_init(void)
{
	int nct72_port = TEGRA_GPIO_PI6;
	int ret = 0;
	int i;
	struct thermal_trip_info *trip_state;
	struct board_info board_info;

	tegra_get_board_info(&board_info);
	/* raise NCT's thresholds if soctherm CP,FT fuses are ok */
	if ((tegra_fuse_calib_base_get_cp(NULL, NULL) >= 0) &&
	    (tegra_fuse_calib_base_get_ft(NULL, NULL) >= 0)) {
		mocha_nct72_pdata.sensors[EXT].shutdown_limit += 20;
		for (i = 0; i < mocha_nct72_pdata.sensors[EXT].num_trips;
			 i++) {
			trip_state = &mocha_nct72_pdata.sensors[EXT].trips[i];
			if (!strncmp(trip_state->cdev_type, "cpu-balanced",
					THERMAL_NAME_LENGTH)) {
				trip_state->cdev_type = "_none_";
				break;
			}
		}
	} else {
		tegra_platform_edp_init(
			mocha_nct72_pdata.sensors[EXT].trips,
			&mocha_nct72_pdata.sensors[EXT].num_trips,
					12000); /* edp temperature margin */
		tegra_add_cpu_vmax_trips(
			mocha_nct72_pdata.sensors[EXT].trips,
			&mocha_nct72_pdata.sensors[EXT].num_trips);
		tegra_add_tgpu_trips(
			mocha_nct72_pdata.sensors[EXT].trips,
			&mocha_nct72_pdata.sensors[EXT].num_trips);
		tegra_add_vc_trips(
			mocha_nct72_pdata.sensors[EXT].trips,
			&mocha_nct72_pdata.sensors[EXT].num_trips);
		tegra_add_core_vmax_trips(
			mocha_nct72_pdata.sensors[EXT].trips,
			&mocha_nct72_pdata.sensors[EXT].num_trips);
	}

	mocha_i2c_nct72_board_info[0].irq = gpio_to_irq(nct72_port);

	ret = gpio_request(nct72_port, "temp_alert");
	if (ret < 0)
		return ret;

	ret = gpio_direction_input(nct72_port);
	if (ret < 0) {
		pr_info("%s: calling gpio_free(nct72_port)", __func__);
		gpio_free(nct72_port);
	}
		i2c_register_board_info(1, mocha_i2c_nct72_board_info,
			ARRAY_SIZE(mocha_i2c_nct72_board_info));

	return ret;
}

int __init mocha_sensors_init(void)
{
	struct board_info board_info;
	tegra_get_board_info(&board_info);
	mocha_camera_init();
	mocha_nct72_init();

#if defined(ARCH_TEGRA_12x_SOC)
	/* TN8 and PM359 don't have ALS CM32181 */
	if (!of_machine_is_compatible("nvidia,mocha"))
		i2c_register_board_info(0, mocha_i2c_board_info_cm32181,
			ARRAY_SIZE(mocha_i2c_board_info_cm32181));
#endif
	return 0;
}
