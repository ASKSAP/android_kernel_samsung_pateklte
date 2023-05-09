/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef _CABC_TUNING_DATA_MONDRIAN_
#define _CABC_TUNING_DATA_MONDRIAN_

#define MASTER_DSI_PDEST 1

#define CABC_LEVEL_MAX 3

static char CABC_NORMAL_1[CABC_TUNE_FIRST_SIZE] = {
	0xB8,
	0x07,
	0x87,
	0x38,
	0x20,
	0x03,
	0xBE,
};

static char CABC_NORMAL_2[CABC_TUNE_SECOND_SIZE] = {
	0xB9,
	0x07,
	0x87,
	0x38,
	0x20,
	0x03,
	0xFF,
};

static char CABC_NORMAL_3[CABC_TUNE_THIRD_SIZE] = {
	0x5E,
	0x03,
};

static char CABC_NORMAL_4[CABC_TUNE_FOURTH_SIZE] = {
	0xCE,
	0x75,
	0x7D,
	0x80,
	0x84,
	0x8A,
	0x8F,
	0x94,
	0x99,
	0x9D,
	0xA1,
	0xA6,
	0xAA,
	0xAF,
	0xB2,
	0xB5,
	0xB7,
	0xB8,
	0x04,
	0x00,
	0x04,
	0x04,
	0x44,
	0x24,
};

static char CABC_NORMAL_5[CABC_TUNE_FIFTH_SIZE] = {
	0xC1,
	0x0C,
	0x60,
	0x00,
	0x00,
	0x80,
	0x00,
	0x44,
	0x04,
	0x00,
	0x80,
	0xE7,
	0x96,
	0x02,
	0x40,
	0x2D,
	0xF7,
	0x00,
	0x38,
	0x40,
	0x94,
	0x10,
	0x01,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x02,
	0x00,
	0x00,
	0x02,
	0x00,
	0x01,
	0x11,
};


static char CABC_NEGATIVE_5[CABC_TUNE_FIFTH_SIZE] = {
	0xC1,
	0x1C,
	0x60,
	0x00,
	0x00,
	0x80,
	0x00,
	0x44,
	0x04,
	0x00,
	0x80,
	0xE7,
	0x96,
	0x02,
	0x40,
	0x2D,
	0xF7,
	0x00,
	0x38,
	0x40,
	0x94,
	0x10,
	0x01,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x02,
	0x00,
	0x00,
	0x02,
	0x00,
	0x01,
	0x11,
};

static char CABC_SELECT_0[CABC_TUNE_SELECT_SIZE] = {
	0x55, 0x00,
};

static char CABC_SELECT_1[CABC_TUNE_SELECT_SIZE] = {
	0x55, 0x01,
};

static char CABC_SELECT_2[CABC_TUNE_SELECT_SIZE] = {
	0x55, 0x02,
};

#endif