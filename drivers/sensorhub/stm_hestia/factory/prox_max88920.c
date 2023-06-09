/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */
#include "../ssp.h"

#ifdef CONFIG_SENSORS_SSP_TMG399X
#define	VENDOR		"AMS"
#define	CHIP_ID		"TMG399X"
#else
#define	VENDOR		"MAXIM"
#define	CHIP_ID		"MAX88920"
#endif

#define CANCELATION_FILE_PATH	"/efs/prox_cal"
#define LCD_LDI_FILE_PATH	"/sys/class/lcd/panel/window_type"

#define LINE_1		'4'
#define LINE_2		'2'

#define LDI_OTHERS	'0'
#define LDI_GRAY	'1'
#define LDI_WHITE	'2'

#if defined(CONFIG_SEC_KACTIVE_PROJECT)
#define CAL_SKIP_ADC	52
#define CAL_FAIL_ADC	80
#elif defined(CONFIG_SENSORS_SSP_STM_HESTIA)
#define CAL_SKIP_ADC	170 /* 50% * LOW THD */
#define CAL_FAIL_ADC	480 /* 100% * HIGH THD */
#else
#define CAL_SKIP_ADC	55
#define CAL_FAIL_ADC	90
#endif

/*************************************************************************/
/* factory Sysfs                                                         */
/*************************************************************************/

static ssize_t prox_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", VENDOR);
}

static ssize_t prox_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CHIP_ID);
}

static ssize_t proximity_avg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u16 prox[3];
	struct ssp_data *data = dev_get_drvdata(dev);

	prox[0] = data->buf[PROXIMITY_RAW].prox[1];
	prox[1] = data->buf[PROXIMITY_RAW].prox[2];
	prox[2] = data->buf[PROXIMITY_RAW].prox[3];

	prox[0] = (prox[0] < PROX_TRIM) ? 0 : prox[0] - PROX_TRIM;
	prox[1] = (prox[1] < PROX_TRIM) ? 0 : prox[1] - PROX_TRIM;
	prox[2] = (prox[2] < PROX_TRIM) ? 0 : prox[2] - PROX_TRIM;

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n",
		prox[0], prox[1], prox[2]);
}

static ssize_t proximity_avg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	char chTempbuf[9] = { 0, };
	int iRet;
	int64_t dEnable;
	struct ssp_data *data = dev_get_drvdata(dev);

	s32 dMsDelay = 20;
	memcpy(&chTempbuf[0], &dMsDelay, 4);

	iRet = kstrtoll(buf, 10, &dEnable);
	if (iRet < 0)
		return iRet;

	if (dEnable) {
		send_instruction(data, ADD_SENSOR, PROXIMITY_RAW, chTempbuf, 9);
		data->bProximityRawEnabled = true;
	} else {
		send_instruction(data, REMOVE_SENSOR, PROXIMITY_RAW,
			chTempbuf, 4);
		data->bProximityRawEnabled = false;
	}

	return size;
}

static u16 get_proximity_rawdata(struct ssp_data *data)
{
	u16 uRowdata = 0;
	char chTempbuf[9] = { 0, };

	s32 dMsDelay = 20;
	memcpy(&chTempbuf[0], &dMsDelay, 4);

	if (data->bProximityRawEnabled == false) {
		send_instruction(data, ADD_SENSOR, PROXIMITY_RAW, chTempbuf, 9);
		msleep(200);
		uRowdata = data->buf[PROXIMITY_RAW].prox[0];
		send_instruction(data, REMOVE_SENSOR, PROXIMITY_RAW,
			chTempbuf, 4);
	} else {
		uRowdata = data->buf[PROXIMITY_RAW].prox[0];
	}

	return uRowdata;
}

static ssize_t proximity_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u16 value;
	struct ssp_data *data = dev_get_drvdata(dev);

	value = get_proximity_rawdata(data) - PROX_TRIM;

	value = (value > 1023) ? 0 : value;

	return sprintf(buf, "%u\n", value);
}

static ssize_t proximity_raw_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u16 value;
	struct ssp_data *data = dev_get_drvdata(dev);

	value = get_proximity_rawdata(data) - PROX_TRIM;

	value = (value > 1023) ? 0 : value;

	return sprintf(buf, "%u\n", value);
}

static int get_proximity_threshold(struct ssp_data *data)
{
	if (data->uCrosstalk < (CAL_SKIP_ADC + PROX_TRIM)) {
		data->uProxCanc = 0;
		data->uProxCalResult = 2;
		pr_info("[SSP] crosstalk < %d, skip calibration\n", CAL_SKIP_ADC);
	} else if (data->uCrosstalk <= (CAL_FAIL_ADC + PROX_TRIM)) {
		data->uProxCanc = data->uCrosstalk * 5 / 10;
		data->uProxCalResult = 1;
	} else {
		data->uProxCanc = 0;
		data->uProxCalResult = 0;
		pr_info("[SSP] crosstalk > %d, calibration failed\n", CAL_FAIL_ADC);
		return ERROR;
	}
	data->uProxHiThresh = data->uProxHiThresh_default + data->uProxCanc;
	data->uProxLoThresh = data->uProxLoThresh_default + data->uProxCanc;

	pr_info("[SSP] %s - crosstalk_offset = %u(%u), HI_THD = %u, LOW_THD = %u\n",
		__func__, data->uProxCanc, data->uCrosstalk,
		data->uProxHiThresh, data->uProxLoThresh);
	return SUCCESS;
}

static void change_proximity_default_threshold(struct ssp_data *data)
{
	switch (data->chLcdLdi[1]) {
	case LDI_GRAY:
		data->uProxHiThresh_default = TBD_HIGH_THRESHOLD;
		data->uProxLoThresh_default = TBD_LOW_THRESHOLD;
		break;
	case LDI_WHITE:
		data->uProxHiThresh_default = WHITE_HIGH_THRESHOLD;
		data->uProxLoThresh_default = WHITE_LOW_THRESHOLD;
		break;
	case LDI_OTHERS:
		data->uProxHiThresh_default = DEFUALT_HIGH_THRESHOLD;
		data->uProxLoThresh_default = DEFUALT_LOW_THRESHOLD;
		break;
	default:
		data->uProxHiThresh_default = DEFUALT_HIGH_THRESHOLD;
		data->uProxLoThresh_default = DEFUALT_LOW_THRESHOLD;
		break;
	}

	data->uProxHiThresh = data->uProxHiThresh_default;
	data->uProxLoThresh = data->uProxLoThresh_default;
}

int proximity_open_lcd_ldi(struct ssp_data *data)
{
	int iRet = 0;
	mm_segment_t old_fs;
	struct file *cancel_filp = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cancel_filp = filp_open(LCD_LDI_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cancel_filp)) {
		iRet = PTR_ERR(cancel_filp);
		if (iRet != -ENOENT)
			pr_err("[SSP] %s - Can't open lcd ldi file\n",
				__func__);
		set_fs(old_fs);
		data->chLcdLdi[0] = 0;
		data->chLcdLdi[1] = 0;
		goto exit;
	}

	iRet = cancel_filp->f_op->read(cancel_filp,
		(u8 *)data->chLcdLdi, sizeof(u8) * 2, &cancel_filp->f_pos);
	if (iRet != (sizeof(u8) * 2)) {
		pr_err("[SSP] %s - Can't read the lcd ldi data\n", __func__);
		iRet = -EIO;
	}

	ssp_dbg("[SSP] %s - %c%c\n", __func__,
		data->chLcdLdi[0], data->chLcdLdi[1]);

	filp_close(cancel_filp, current->files);
	set_fs(old_fs);

exit:
	change_proximity_default_threshold(data);
	return iRet;
}

int proximity_open_calibration(struct ssp_data *data)
{
	int iRet = 0;
	mm_segment_t old_fs;
	struct file *cancel_filp = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cancel_filp = filp_open(CANCELATION_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cancel_filp)) {
		iRet = PTR_ERR(cancel_filp);
		if (iRet != -ENOENT)
			pr_err("[SSP] %s - Can't open cancelation file\n",
				__func__);
		set_fs(old_fs);
		goto exit;
	}

	iRet = cancel_filp->f_op->read(cancel_filp,
		(u8 *)&data->uProxCanc, sizeof(unsigned int), &cancel_filp->f_pos);
	if (iRet != sizeof(u8)) {
		pr_err("[SSP] %s - Can't read the cancel data\n", __func__);
		iRet = -EIO;
	}

	if (data->uProxCanc != 0) {
		/*If there is an offset cal data. */
		data->uProxHiThresh =
			data->uProxHiThresh_default + data->uProxCanc;
		data->uProxLoThresh =
			data->uProxLoThresh_default + data->uProxCanc;
	}

	pr_info("%s: proximity ps_canc = %d, ps_thresh hi - %d lo - %d\n",
		__func__, data->uProxCanc, data->uProxHiThresh,
		data->uProxLoThresh);

	filp_close(cancel_filp, current->files);
	set_fs(old_fs);

exit:
	return iRet;
}

static int proximity_store_cancelation(struct ssp_data *data, int iCalCMD)
{
	int iRet = 0;
	mm_segment_t old_fs;
	struct file *cancel_filp = NULL;

	if (iCalCMD) {
		data->uCrosstalk = get_proximity_rawdata(data);
		iRet = get_proximity_threshold(data);
	} else {
		data->uProxHiThresh = data->uProxHiThresh_default;
		data->uProxLoThresh = data->uProxLoThresh_default;
		data->uProxCanc = 0;
	}

	if (iRet != ERROR)
		set_proximity_threshold(data, data->uProxHiThresh,
			data->uProxLoThresh);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cancel_filp = filp_open(CANCELATION_FILE_PATH,
			O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, 0666);
	if (IS_ERR(cancel_filp)) {
		pr_err("%s: Can't open cancelation file\n", __func__);
		set_fs(old_fs);
		iRet = PTR_ERR(cancel_filp);
		return iRet;
	}

	iRet = cancel_filp->f_op->write(cancel_filp, (u8 *)&data->uProxCanc,
		sizeof(unsigned int), &cancel_filp->f_pos);
	if (iRet != sizeof(unsigned int)) {
		pr_err("%s: Can't write the cancel data to file\n", __func__);
		iRet = -EIO;
	}

	filp_close(cancel_filp, current->files);
	set_fs(old_fs);

	return iRet;
}

static ssize_t proximity_cancel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ssp_data *data = dev_get_drvdata(dev);

	ssp_dbg("[SSP] uProxThresh : hi : %u lo : %u, uProxCanc = %u\n",
		data->uProxHiThresh, data->uProxLoThresh, data->uProxCanc);

	return sprintf(buf, "%u,%u,%u\n", data->uProxCanc,
		data->uProxHiThresh, data->uProxLoThresh);
}

static ssize_t proximity_cancel_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int iCalCMD = 0, iRet = 0;
	struct ssp_data *data = dev_get_drvdata(dev);

	if (sysfs_streq(buf, "1")) /* calibrate cancelation value */
		iCalCMD = 1;
	else if (sysfs_streq(buf, "0")) /* reset cancelation value */
		iCalCMD = 0;
	else {
		pr_debug("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	iRet = proximity_store_cancelation(data, iCalCMD);
	if (iRet < 0) {
		pr_err("[SSP] - %s proximity_store_cancelation() failed\n",
			__func__);
		return iRet;
	}

	ssp_dbg("[SSP] %s - %u\n", __func__, iCalCMD);
	return size;
}

static ssize_t proximity_thresh_high_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ssp_data *data = dev_get_drvdata(dev);

	ssp_dbg("[SSP] uProxThresh = hi - %u, lo - %u\n",
		data->uProxHiThresh, data->uProxLoThresh);

	return sprintf(buf, "%u,%u\n", data->uProxHiThresh,
		data->uProxLoThresh);
}

static ssize_t proximity_thresh_high_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u16 uNewThresh;
	int iRet = 0;
	struct ssp_data *data = dev_get_drvdata(dev);

	iRet = kstrtou16(buf, 10, &uNewThresh);
	if (iRet < 0)
		pr_err("[SSP] %s - kstrtoint failed.(%d)\n", __func__, iRet);
	else {
		if(uNewThresh & 0xfc00)
			pr_err("[SSP] %s - allow 10bits.(%d)\n", __func__, uNewThresh);
		else {
			uNewThresh &= 0x03ff;
			data->uProxHiThresh = uNewThresh;
			set_proximity_threshold(data, data->uProxHiThresh,
				data->uProxLoThresh);
		}
	}

	ssp_dbg("[SSP] %s - new prox threshold : hi - %u, lo - %u\n",
		__func__, data->uProxHiThresh, data->uProxLoThresh);

	return size;
}

static ssize_t proximity_thresh_low_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ssp_data *data = dev_get_drvdata(dev);

	ssp_dbg("[SSP] uProxThresh = hi - %u, lo - %u\n",
		data->uProxHiThresh, data->uProxLoThresh);

	return sprintf(buf, "%u,%u\n", data->uProxHiThresh,
		data->uProxLoThresh);
}

static ssize_t proximity_thresh_low_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u16 uNewThresh;
	int iRet = 0;
	struct ssp_data *data = dev_get_drvdata(dev);

	iRet = kstrtou16(buf, 10, &uNewThresh);
	if (iRet < 0)
		pr_err("[SSP] %s - kstrtoint failed.(%d)\n", __func__, iRet);
	else {
		if(uNewThresh & 0xfc00)
			pr_err("[SSP] %s - allow 10bits.(%d)\n", __func__, uNewThresh);
		else {
			uNewThresh &= 0x03ff;
			data->uProxLoThresh = uNewThresh;
			set_proximity_threshold(data, data->uProxHiThresh,
				data->uProxLoThresh);
		}
	}

	ssp_dbg("[SSP] %s - new prox threshold : hi - %u, lo - %u\n",
		__func__, data->uProxHiThresh, data->uProxLoThresh);

	return size;
}

static ssize_t proximity_cancel_pass_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ssp_data *data = dev_get_drvdata(dev);

	pr_info("[SSP] %s, %u\n", __func__, data->uProxCalResult);
	return snprintf(buf, PAGE_SIZE, "%u\n", data->uProxCalResult);
}

static ssize_t barcode_emul_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ssp_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%u\n", data->bBarcodeEnabled);
}

static ssize_t barcode_emul_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int iRet;
	int64_t dEnable;
	struct ssp_data *data = dev_get_drvdata(dev);

	iRet = kstrtoll(buf, 10, &dEnable);
	if (iRet < 0)
		return iRet;

	if (dEnable)
		set_proximity_barcode_enable(data, true);
	else
		set_proximity_barcode_enable(data, false);

	return size;
}

static DEVICE_ATTR(vendor, S_IRUGO, prox_vendor_show, NULL);
static DEVICE_ATTR(name, S_IRUGO, prox_name_show, NULL);
static DEVICE_ATTR(state, S_IRUGO, proximity_state_show, NULL);
static DEVICE_ATTR(raw_data, S_IRUGO, proximity_raw_data_show, NULL);
static DEVICE_ATTR(barcode_emul_en, S_IRUGO | S_IWUSR | S_IWGRP,
	barcode_emul_enable_show, barcode_emul_enable_store);
static DEVICE_ATTR(prox_avg, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_avg_show, proximity_avg_store);
static DEVICE_ATTR(prox_cal, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_cancel_show, proximity_cancel_store);
static DEVICE_ATTR(thresh_high, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_thresh_high_show, proximity_thresh_high_store);
static DEVICE_ATTR(thresh_low, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_thresh_low_show, proximity_thresh_low_store);
static DEVICE_ATTR(prox_offset_pass, S_IRUGO, proximity_cancel_pass_show, NULL);

static struct device_attribute *prox_attrs[] = {
	&dev_attr_vendor,
	&dev_attr_name,
	&dev_attr_state,
	&dev_attr_raw_data,
	&dev_attr_prox_avg,
	&dev_attr_prox_cal,
	&dev_attr_thresh_high,
	&dev_attr_thresh_low,
	&dev_attr_barcode_emul_en,
	&dev_attr_prox_offset_pass,
	NULL,
};

void initialize_prox_factorytest(struct ssp_data *data)
{
	sensors_register(data->prox_device, data,
		prox_attrs, "proximity_sensor");
}

void remove_prox_factorytest(struct ssp_data *data)
{
	sensors_unregister(data->prox_device, prox_attrs);
}
