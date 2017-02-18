/*
 * Copyright (c) 2013-2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_data/tegra_emc_pdata.h>

#include "board.h"
#include "board-mocha.h"
#include "tegra-board-id.h"
#include <linux/platform/tegra/tegra12_emc.h>
#include "devices.h"

/*
 * MOCHA emc init.
 */
int __init mocha_emc_init(void)
{
	if (of_find_compatible_node(NULL, NULL, "nvidia,tegra12-emc")) {
		pr_info("Loading EMC tables from DeviceTree.\n");
	}
	tegra12_emc_init();
	return 0;
}
