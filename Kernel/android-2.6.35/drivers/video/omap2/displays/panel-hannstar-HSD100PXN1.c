/*
 * HannStar HSD100PXN1 LDVS panel support
 *
 * Copyright (C) 2010 HON HAI Technology Group
 * Author: JJ
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
//#include <linux/i2c/twl.h>
//#include <linux/spi/spi.h>

#include <plat/gpio.h>
#include <plat/mux.h>
#include <asm/mach-types.h>
#include <plat/control.h>

#include <plat/display.h>

#include <linux/wakelock.h>

#define LCD_XRES		1024
#define LCD_YRES		768

#define LCD_PIXCLOCK_MIN      	55000 /* Min. PIX Clock is 55MHz */
#define LCD_PIXCLOCK_TYP      	65000 /* Typ. PIX Clock is 65MHz */
#define LCD_PIXCLOCK_MAX      	75000 /* Max. PIX Clock is 75MHz */

/* Current Pixel clock */
//&*&*&*JJ1_20110831 for hannstar display interface timing
//#define LCD_PIXEL_CLOCK		LCD_PIXCLOCK_TYP
#define LCD_PIXEL_CLOCK		57600//LCD_PIXCLOCK_TYP
//&*&*&*JJ1_20110831 for hannstar display interface timing

#define EDP_LCD_LVDS_SHTDN_GPIO 	37
static int EDP_LCD_PWR_EN_GPIO = 88;  //20110812, JimmySu modify LCD power-on sequence
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume
struct delayed_work delay_work_enablepanel;
struct wake_lock panel_wake_lock;
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume
/*HannStar HSD100PXN1 Manual
 * defines HFB, HSW, HBP, VFP, VSW, VBP as shown below
 */
/*
========== Detailed Timing Parameters ==========
Timming Name			= 1024x768@60Hz;
Hor Pixels				= 1024;				//pixels
Ver Pixels				= 768;				//Lines
Hor Frequency 		= 48.363;			//kHz = 20.7 usec/line
Ver Frequency 		= 60.004;			//Hz	= 16.7 msec/frame
Pixel Clock				= 65.000;			//MHz	=	15.4 nsec	��0.5%
Character Width 	= 8;					//Pixel	=	123.1nsec
Scan Type					= NONINTERLACED;	//H Phase = 5.1%
Hor Sync Polarity	= NEGATIVE;		//HBlank = 23.8% of HTotal
Ver Sync Polarity	= NEGATIVE;		//VBlank =  4.7% of VTotal
Hor Total Time 		= 20.677;			//(usec)	=	168 chars	=	1344 Pixels
Hor Addr Time 		= 15.754;			//(usec)	=	128 chars	=	1024 Pixels
Hor Blank Start		= 15.754;			//(usec)	=	128 chars	=	1024 Pixels
Hor Blank Time 		= 4.923;			//(usec)	=	 40 chars	=	 320 Pixels
Hor Sync Start		=	16.123			//(usec)	=	131 chars	=	1048 Pixels
H Right Border		=	0.000;			//(usec)	=		0 chars = 	 0 Pixels
H Front Porch			= 0.369;			//(usec)	=		3 chars =		24 Pixels
Hor Sync Time			=	2.092;			//(usec)	=	 17 chars =  136 Pixels
H Back Porch 			= 2.462;			//(usec)	=	 20	chars =  160 Pixels
H Left Border			= 0.000;			//(usec)	=	  0 chars = 	 0 Pixels
Ver Total Time		= 16.666;			//(msec)	=	806 lines			HT-(1.06xHA)=3.98
Ver Addr Time			=	15.880;			//(msec)	=	768 lines
Ver Blank Start		= 15.880;			//(msec)	=	768 lines
Ver Blank Time		= 0.786;			//(msec)	=	 38 lines
Ver Sync Start		= 15.942;			//(msec)	=	771 lines
V Bottom Border		= 0.000;			//(msec)	=		0 lines
V Front Porch			= 0.062;			//(msec)	= 	3 lines
Ver Sync Time			= 0.124;			//(msec)	=		6 lines
V Back Porch 			= 0.600;			//(msec)	=	 29 lines
V Top Border			=	0.000;			//(msec)	=		0 lines
*/
static struct omap_video_timings hannstar_panel_timings = {
	/* 1024 x 768 @ 60 Hz   */
	.x_res          = LCD_XRES,
	.y_res          = LCD_YRES,
	.pixel_clock    = LCD_PIXEL_CLOCK,
//&*&*&*JJ1_20110831 for hannstar display interface timing
//	.hfp            = 24,		//320
//	.hsw            = 136,
//	.hbp            = 160,
//	.vfp            = 3,
//	.vsw            = 6,
//	.vbp            = 29,
	
	/* 1024 x 768 @ 55.7 Hz   */
	.hfp            = 20,	//280
	.hsw            = 120,
	.hbp            = 140,
	.vfp            = 2,	//25
	.vsw            = 5,
	.vbp            = 18,
//&*&*&*JJ1_20110831 for hannstar display interface timing
};
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume
void LCD_3V3_enable(int enable)
{
	if(enable)
	gpio_direction_output(EDP_LCD_PWR_EN_GPIO, 1);
	else
	gpio_direction_output(EDP_LCD_PWR_EN_GPIO, 0);

}

static void omap_delay_work_enablepanel(struct work_struct *work)
{
	 gpio_request(EDP_LCD_LVDS_SHTDN_GPIO, "lcd LVDS SHTN");
	 gpio_direction_output(EDP_LCD_LVDS_SHTDN_GPIO, 1);	
}
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume

static int hannstar_panel_probe(struct omap_dss_device *dssdev)
{
	printk("[CES-demo]:hannstar_panel_probe()++++++\n");
	
	dssdev->panel.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_IVS | OMAP_DSS_LCD_IHS ;
	//dssdev->panel.config = OMAP_DSS_LCD_TFT /*| OMAP_DSS_LCD_IPC | OMAP_DSS_LCD_IVS | OMAP_DSS_LCD_IHS*/ ;
			
	dssdev->panel.timings = hannstar_panel_timings;

	dssdev->ctrl.pixel_size = 16;
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume
	INIT_DELAYED_WORK(&delay_work_enablepanel, omap_delay_work_enablepanel);
	wake_lock_init(&panel_wake_lock, WAKE_LOCK_SUSPEND, "panel_wakelock");
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume	

	printk("[CES-demo]:hannstar_panel_probe()------\n");
	
	return 0;
}

static void hannstar_panel_remove(struct omap_dss_device *dssdev)
{
}

static int hannstar_panel_enable(struct omap_dss_device *dssdev)
{
	int r = 0;
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume	
//	 gpio_request(EDP_LCD_LVDS_SHTDN_GPIO, "lcd LVDS SHTN");
//&*&*&*JJ
//	 gpio_direction_output(EDP_LCD_LVDS_SHTDN_GPIO, 0);
//	 msleep(10);
//	 gpio_direction_output(EDP_LCD_LVDS_SHTDN_GPIO, 1);	
//	 msleep(100);
//	 gpio_direction_output(EDP_LCD_LVDS_SHTDN_GPIO, 0);
//	 msleep(10);
//&*&*&*JJ
//++++++20110812, JimmySu modify LCD power-on sequence
	gpio_direction_output(EDP_LCD_PWR_EN_GPIO, 1);
	//gpio_direction_output(EDP_LCD_LVDS_SHTDN_GPIO, 1);
	msleep(100);
//-----20110812, JimmySu modify LCD power-on sequence
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume	
	r = omapdss_dpi_display_enable(dssdev);
	if (r)
		goto err0;

	/* Delay recommended by panel DATASHEET */
	mdelay(4);
	if (dssdev->platform_enable) {
		r = dssdev->platform_enable(dssdev);
		if (r)
			goto err1;
	}
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume
	schedule_delayed_work(&delay_work_enablepanel, 10);
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume	
	return 0;
err1:
	omapdss_dpi_display_disable(dssdev);
err0:
	return r;
}

static void hannstar_panel_disable(struct omap_dss_device *dssdev)
{
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume	
	cancel_delayed_work(&delay_work_enablepanel);
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume	
	if (dssdev->platform_disable)
		dssdev->platform_disable(dssdev);
	 gpio_direction_output(EDP_LCD_LVDS_SHTDN_GPIO, 0);

	/* Delay recommended by panel DATASHEET */
	mdelay(4);

	omapdss_dpi_display_disable(dssdev);
//&*&*&*BC1_110817: fix the issue that user can see screen switch when device resume
	wake_lock_timeout(&panel_wake_lock, HZ/2);
//&*&*&*BC2_110817: fix the issue that user can see screen switch when device resume
}

static int hannstar_panel_suspend(struct omap_dss_device *dssdev)
{
	hannstar_panel_disable(dssdev);
	return 0;
}

static int hannstar_panel_resume(struct omap_dss_device *dssdev)
{
	return hannstar_panel_enable(dssdev);
}

static void hannstar_panel_get_timings(struct omap_dss_device *dssdev,
		struct omap_video_timings *timings)
{
	*timings = dssdev->panel.timings;
}

static struct omap_dss_driver hannstar_driver = {
	.probe		= hannstar_panel_probe,
	.remove		= hannstar_panel_remove,

	.enable		= hannstar_panel_enable,
	.disable	= hannstar_panel_disable,
	.suspend	= hannstar_panel_suspend,
	.resume		= hannstar_panel_resume,

	.get_timings	= hannstar_panel_get_timings,

	.driver         = {
		.name   = "hannstar_panel",
		.owner  = THIS_MODULE,
	},
};

static int __init hannstar_panel_drv_init(void)
{
	return omap_dss_register_driver(&hannstar_driver);
}

static void __exit hannstar_panel_drv_exit(void)
{
	omap_dss_unregister_driver(&hannstar_driver);
}

module_init(hannstar_panel_drv_init);
module_exit(hannstar_panel_drv_exit);
MODULE_LICENSE("GPL");
