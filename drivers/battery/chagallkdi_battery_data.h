/*
 * battery_data.h
 * Samsung Mobile Battery data Header
 *
 *
 * Copyright (C) 2012 Samsung Electronics, Inc.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __SEC_BATTERY_DATA_H
#define __SEC_BATTERY_DATA_H __FILE__

static struct battery_data_t samsung_battery_data[] = {
	/* SDI battery data (High voltage 4.35V) */
	{
		.RCOMP0 = 0x57,
		.RCOMP_charging = 0x57,
		.temp_cohot = -787,
		.temp_cocold = -5525,
		.is_using_model_data = true,
		.type_str = "SDI",
	}
};

#define CAPACITY_MAX			992
#define CAPACITY_MAX_MARGIN	50
#define CAPACITY_MIN			0

static sec_bat_adc_table_data_t temp_table[] = {
	{25136,  700},
	{25198,  650},
	{25271,  600},
	{25360,  550},
	{25466,  500},
	{25591,  450},
	{25747,  400},
	{25928,  350},
	{26140,  300},
	{26410,  250},
	{26724,  200},
	{27098,  150},
	{27552,  100},
	{28086,  50},
	{28721,	 0},
	{29474, -50},
	{30351, -100},
	{31312, -150},
	{32384, -200}
};

#define TEMP_HIGH_THRESHOLD_EVENT	567
#define TEMP_HIGH_RECOVERY_EVENT	480
#define TEMP_LOW_THRESHOLD_EVENT	-50
#define TEMP_LOW_RECOVERY_EVENT		0
#define TEMP_HIGH_THRESHOLD_NORMAL	567
#define TEMP_HIGH_RECOVERY_NORMAL	480
#define TEMP_LOW_THRESHOLD_NORMAL	-50
#define TEMP_LOW_RECOVERY_NORMAL	0
#define TEMP_HIGH_THRESHOLD_LPM		520
#define TEMP_HIGH_RECOVERY_LPM		472
#define TEMP_LOW_THRESHOLD_LPM		-20
#define TEMP_LOW_RECOVERY_LPM		0

#endif /* __SEC_BATTERY_DATA_H */
