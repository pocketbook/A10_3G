/*
 *
 * Digital Accelerometer characteristics are highly application specific
 * and may vary between boards and models. The platform_data for the
 * device's "struct device" holds this information.
 */

#ifndef _ADXL345_H_
#define _ADXL345_H_

#ifdef CONFIG_GSENSOR_AUTO_ROTATION_TEST
#define _ADXL345_AUTO_ROTATION_
#endif


struct adxl345_platform_data {

	/*
	 * X,Y,Z Axis Offset:
	 * offer user offset adjustments in twoscompliment
	 * form with a scale factor of 15.6 mg/LSB (i.e. 0x7F = +2 g)
	 */

	char x_axis_offset;
	char y_axis_offset;
	char z_axis_offset;

	/*
	 * TAP_X/Y/Z Enable: Setting TAP_X, Y, or Z Enable enables X,
	 * Y, or Z participation in Tap detection. A '0' excludes the
	 * selected axis from participation in Tap detection.
	 * Setting the SUPPRESS bit suppresses Double Tap detection if
	 * acceleration greater than tap_threshold is present between
	 * taps.
	 */

#define ADXL_SUPPRESS 	(1 << 3)
#define ADXL_TAP_X_EN 	(1 << 2)
#define ADXL_TAP_Y_EN 	(1 << 1)
#define ADXL_TAP_Z_EN 	(1 << 0)

	unsigned char tap_axis_control;

	/*
	 * tap_threshold:
	 * holds the threshold value for tap detection/interrupts.
	 * The data format is unsigned. The scale factor is 62.5 mg/LSB
	 * (i.e. 0xFF = +16 g). A zero value may result in undesirable
	 * behavior if Tap/Double Tap is enabled.
	 */

	unsigned char tap_threshold;

	/*
	 * tap_duration:
	 * is an unsigned time value representing the maximum
	 * time that an event must be above the tap_threshold threshold
	 * to qualify as a tap event. The scale factor is 625 us/LSB. A zero
	 * value will prevent Tap/Double Tap functions from working.
	 */

	unsigned char tap_duration;

	/*
	 * tap_latency:
	 * is an unsigned time value representing the wait time
	 * from the detection of a tap event to the opening of the time
	 * window tap_window for a possible second tap event. The scale
	 * factor is 1.25 ms/LSB. A zero value will disable the Double Tap
	 * function.
	 */

	unsigned char tap_latency;

	/*
	 * tap_window:
	 * is an unsigned time value representing the amount
	 * of time after the expiration of tap_latency during which a second
	 * tap can begin. The scale factor is 1.25 ms/LSB. A zero value will
	 * disable the Double Tap function.
	 */

	unsigned char tap_window;

	/*
	 * act_axis_control:
	 * X/Y/Z Enable: A '1' enables X, Y, or Z participation in activity
	 * or inactivity detection. A '0' excludes the selected axis from
	 * participation. If all of the axes are excluded, the function is
	 * disabled.
	 * AC/DC: A '0' = DC coupled operation and a '1' = AC coupled
	 * operation. In DC coupled operation, the current acceleration is
	 * compared with activity_threshold and inactivity_threshold directly
	 * to determine whether activity or inactivity is detected. In AC
	 * coupled operation for activity detection, the acceleration value
	 * at the start of activity detection is taken as a reference value.
	 * New samples of acceleration are then compared to this
	 * reference value and if the magnitude of the difference exceeds
	 * activity_threshold the device will trigger an activity interrupt. In
	 * AC coupled operation for inactivity detection, a reference value
	 * is used again for comparison and is updated whenever the
	 * device exceeds the inactivity threshold. Once the reference
	 * value is selected, the device compares the magnitude of the
	 * difference between the reference value and the current
	 * acceleration with inactivity_threshold. If the difference is below
	 * inactivity_threshold for a total of inactivity_time, the device is
	 * considered inactive and the inactivity interrupt is triggered.
	 */

#define ADXL_ACT_ACDC   	(1 << 7)
#define ADXL_ACT_X_EN   	(1 << 6)
#define ADXL_ACT_Y_EN   	(1 << 5)
#define ADXL_ACT_Z_EN   	(1 << 4)
#define ADXL_INACT_ACDC 	(1 << 3)
#define ADXL_INACT_X_EN 	(1 << 2)
#define ADXL_INACT_Y_EN 	(1 << 1)
#define ADXL_INACT_Z_EN 	(1 << 0)

	unsigned char act_axis_control;

	/*
	 * activity_threshold:
	 * holds the threshold value for activity detection.
	 * The data format is unsigned. The scale factor is
	 * 62.5 mg/LSB. A zero value may result in undesirable behavior if
	 * Activity interrupt is enabled.
	 */

	unsigned char activity_threshold;

	/*
	 * inactivity_threshold:
	 * holds the threshold value for inactivity
	 * detection. The data format is unsigned. The scale
	 * factor is 62.5 mg/LSB. A zero value may result in undesirable
	 * behavior if Inactivity interrupt is enabled.
	 */

	unsigned char inactivity_threshold;

	/*
	 * inactivity_time:
	 * is an unsigned time value representing the
	 * amount of time that acceleration must be below the value in
	 * inactivity_threshold for inactivity to be declared. The scale factor
	 * is 1 second/LSB. Unlike the other interrupt functions, which
	 * operate on unfiltered data, the inactivity function operates on the
	 * filtered output data. At least one output sample must be
	 * generated for the inactivity interrupt to be triggered. This will
	 * result in the function appearing un-responsive if the
	 * inactivity_time register is set with a value less than the time
	 * constant of the Output Data Rate. A zero value will result in an
	 * interrupt when the output data is below inactivity_threshold.
	 */

	unsigned char inactivity_time;

	/*
	 * free_fall_threshold:
	 * holds the threshold value for Free-Fall detection.
	 * The data format is unsigned. The root-sum-square(RSS) value
	 * of all axes is calculated and compared to the value in
	 * free_fall_threshold to determine if a free fall event may be
	 * occurring.  The scale factor is 62.5 mg/LSB. A zero value may
	 * result in undesirable behavior if Free-Fall interrupt is
	 * enabled. Values between 300 and 600 mg (0x05 to 0x09) are
	 * recommended.
	 */

	unsigned char free_fall_threshold;

	/*
	 * free_fall_time:
	 * is an unsigned time value representing the minimum
	 * time that the RSS value of all axes must be "less" than
	 * free_fall_threshold to generate a Free-Fall interrupt. The
	 * scale factor is 5 ms/LSB. A zero value may result in
	 * undesirable behavior if Free-Fall interrupt is enabled.
	 * Values between 100 to 350 ms (0x14 to 0x46) are recommended.
	 */

	unsigned char free_fall_time;

	/*
	 * data_rate:
	 * Selects device bandwidth and output data rate.
	 * RATE = 3200 Hz / (2^(15 - x)). Default value is 0x0A, or 100 Hz
	 * Output Data Rate. An Output Data Rate should be selected that
	 * is appropriate for the communication protocol and frequency
	 * selected. Selecting too high of an Output Data Rate with a low
	 * communication speed will result in samples being discarded.
	 */

	unsigned char data_rate;

	/*
	 * data_format:
	 * FULL_RES: When this bit is set with the device is
	 * in Full-Resolution Mode, where the output resolution increases
	 * with RANGE to maintain a 4 mg/LSB scale factor. When this
	 * bit is cleared the device is in 10-bit Mode and RANGE determine the
	 * maximum g-Range and scale factor.
	 */

#define ADXL_FULL_RES    	(1 << 3)
#define ADXL_RANGE_PM_2g	0
#define ADXL_RANGE_PM_4g	1
#define ADXL_RANGE_PM_8g	2
#define ADXL_RANGE_PM_16g	3

	unsigned char data_format;

	/*
	 * low_power_mode:
	 * A '0' = Normal operation and a '1' = Reduced
	 * power operation with somewhat higher noise.
	 */

	unsigned char low_power_mode;

	/*
	 * power_mode:
	 * LINK: A '1' with both the activity and inactivity functions
	 * enabled will delay the start of the activity function until
	 * inactivity is detected. Once activity is detected, inactivity
	 * detection will begin and prevent the detection of activity. This
	 * bit serially links the activity and inactivity functions. When '0'
	 * the inactivity and activity functions are concurrent. Additional
	 * information can be found in the Application section under Link
	 * Mode.
	 * AUTO_SLEEP: A '1' sets the ADXL34x to switch to Sleep Mode
	 * when inactivity (acceleration has been below inactivity_threshold
	 * for at least inactivity_time) is detected and the LINK bit is set.
	 * A '0' disables automatic switching to Sleep Mode. See SLEEP
	 * for further description.
	 */

#define ADXL_LINK    	(1 << 5)
#define ADXL_AUTO_SLEEP (1 << 4)

	unsigned char power_mode;

	/*
	 * fifo_mode:
	 * BYPASS The FIFO is bypassed
	 * FIFO   FIFO collects up to 32 values then stops collecting data
	 * STREAM FIFO holds the last 32 data values. Once full, the FIFO's
	 * 	  oldest data is lost as it is replaced with newer data
	 *
	 * DEFAULT should be ADXL_FIFO_STREAM
	 */

#define ADXL_FIFO_BYPASS	0
#define ADXL_FIFO_FIFO		1
#define ADXL_FIFO_STREAM	2

	unsigned char fifo_mode;

	/*
	 * watermark:
	 * The Watermark feature can be used to reduce the interrupt load
	 * of the system. The FIFO fills up to the value stored in watermark
	 * [1..32] and then generates an interrupt.
	 * A '0' disables the watermark feature.
	 */

	unsigned char watermark;

	unsigned int ev_type;	  /* EV_ABS or EV_REL */

	unsigned int ev_code_x;	/* ABS_X,Y,Z or REL_X,Y,Z */
	unsigned int ev_code_y;	/* ABS_X,Y,Z or REL_X,Y,Z */
	unsigned int ev_code_z;	/* ABS_X,Y,Z or REL_X,Y,Z */

	/*
	 * A valid BTN or KEY Code; use tap_axis_control to disable
	 * event reporting
	 */

	unsigned int ev_code_tap_x;	/* EV_KEY */
	unsigned int ev_code_tap_y;	/* EV_KEY */
	unsigned int ev_code_tap_z;	/* EV_KEY */

	/*
	 * A valid BTN or KEY Code for Free-Fall or Activity enables
	 * input event reporting. A '0' disables the Free-Fall or
	 * Activity reporting.
	 */

	unsigned int ev_code_ff;	/* EV_KEY */
	unsigned int ev_code_activity;	/* EV_KEY */
	unsigned int ev_code_inactivity;
	
};
#endif



struct adxl345;



struct axis_triple {
	short int x;
	short int y;
	short int z;
};



struct adxl345 {
	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;
	struct mutex mutex;	 /* reentrant protection for struct */
	struct adxl345_platform_data pdata;
	struct axis_triple swcal;
	struct axis_triple hwcal;
	struct axis_triple saved;
	char phys[32];
	unsigned enabled:1;	/* P: mutex */
	unsigned opened:1;	  /* P: mutex */
	int delaytime;
	int bstoppolling;
	unsigned fifo_delay:1;
	unsigned model;
	unsigned int_mask;
	int (*read) (struct i2c_client *client, unsigned char reg, unsigned char * buf);
	int (*read_block) (struct i2c_client *client, unsigned char reg, int count, unsigned char *buf);
	int (*write) (struct i2c_client *client, unsigned char reg, unsigned char val);
	#ifdef _ADXL345_AUTO_ROTATION_
	int auto_rotation;	/* P: mutex */
	struct timer_list adx_auto_rotation_timeout;
	int rotation;
	#endif
	#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
	#endif	
	int cancel_work;
};

