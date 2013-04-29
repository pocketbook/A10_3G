/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>

//&*&*&*20101201_Peter ++
#if (CONFIG_STORAGE_EMMC)
#include <mmc.h>
#include "../cpu/omap3/mmc_host_def.h"
#include "../cpu/omap3/mmc_protocol.h"
extern mmc_card_data cur_card_data[2];
#endif
//&*&*&*20101201_Peter --

//&*&*&*SJ1_20110419, Implement display software and hardware version info. 
#if defined (CONFIG_SHARE_REGION)
#include <share_region.h>
extern int mmc_flag[];
#endif
//&*&*&*SJ2_20110419, Implement display software and hardware version info.

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/lan91c96.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#ifdef ENV_IS_VARIABLE
extern u8 is_nand;
#endif
void nand_init (void);
#endif

#if (CONFIG_COMMANDS & CFG_CMD_FLASH)
#ifdef ENV_IS_VARIABLE
extern u8 is_flash;
#endif
#endif

#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)
#ifdef ENV_IS_VARIABLE
extern u8 is_onenand;
#endif
void onenand_init(void);
#endif

ulong monitor_flash_len;

#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#if (CONFIG_STORAGE_EMMC)
extern void board_mmc_init(void);
#endif

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")"CONFIG_IDENT_STRING;

#ifdef CONFIG_DRIVER_CS8900
extern void cs8900_get_enetaddr (uchar * addr);
#endif

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

//&*&*&*HC1_20110304, Add offmode charging
extern int off_mode_start( void );
//&*&*&*HC2_20110304, Add offmode charging

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start = 0;
static ulong mem_malloc_end = 0;
static ulong mem_malloc_brk = 0;

static
void mem_malloc_init (ulong dest_addr)
{
	mem_malloc_start = dest_addr;
	mem_malloc_end = dest_addr + CFG_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start, 0,
			mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;

	return ((void *) old);
}

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}

static int display_banner (void)
{
	printf ("\n\n%s\n\n", version_string);
	debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _armboot_start, _bss_start, _bss_end);
#ifdef CONFIG_MODEM_SUPPORT
	debug ("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug ("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug ("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config (void)
{
	int i;

#ifdef DEBUG
	puts ("RAM Configuration:\n");

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return (0);
}

#ifndef CFG_NO_FLASH
static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}
#endif /* CFG_NO_FLASH */


/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo (void); /* test-only */

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
	interrupt_init,		/* set up exceptions */
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
//	checkboard,		/* display board info */
#endif
	dram_init,		/* configure available RAM banks */
	display_dram_config,
//&*&*&*20110225_Peter ++
#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	i2c_init_r,
#endif
//&*&*&*20110225_Peter --
//&*&*&*20110413_Herry ++
	gpio_init,
//&*&*&*20110413_Herry --	
	NULL,
};

void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	char *s;
#ifndef CFG_NO_FLASH
	ulong size = 0;
#endif
#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
	unsigned long addr;
#endif

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t*)(_armboot_start - CFG_MALLOC_LEN - sizeof(gd_t));
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));
	gd->bd = (bd_t*)((char*)gd - sizeof(bd_t));
	memset (gd->bd, 0, sizeof (bd_t));

	monitor_flash_len = _bss_start - _armboot_start;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

#ifndef CFG_NO_FLASH
	/* configure available FLASH banks */
#ifdef ENV_IS_VARIABLE
	if (is_flash) 
#endif
	size = flash_init ();
	display_flash_config (size);
#endif /* CFG_NO_FLASH */

#ifdef CONFIG_VFD
#	ifndef PAGE_SIZE
#	  define PAGE_SIZE 4096
#	endif
	/*
	 * reserve memory for VFD display (always full pages)
	 */
	/* bss_end is defined in the board-specific linker script */
	addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	size = vfd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_VFD */

#ifdef CONFIG_LCD
#	ifndef PAGE_SIZE
#	  define PAGE_SIZE 4096
#	endif
	/*
	 * reserve memory for LCD display (always full pages)
	 */
	/* bss_end is defined in the board-specific linker script */
	addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	size = lcd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_LCD */

	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init (_armboot_start - CFG_MALLOC_LEN);

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#ifdef ENV_IS_VARIABLE
	if (is_nand) 
#endif
	{
		puts ("NAND:  ");
		nand_init();		/* go init the NAND */
	}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)
#ifdef ENV_IS_VARIABLE
	if (is_onenand)
#endif
	onenand_init();
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif

#if (CONFIG_STORAGE_EMMC)
	/* puts("EMMC partitons init:"); */
	board_mmc_init();
//&*&*&*20101201_Peter ++
	mmc_init(CFG_FASTBOOT_MMC_NO);
	mmc_flag[1] = 1;
	printf("EMMC:  %u MB (%d sectors)\n", (cur_card_data[CFG_FASTBOOT_MMC_NO].size/2048), cur_card_data[CFG_FASTBOOT_MMC_NO].size);
//&*&*&*20101201_Peter --
#endif

	/* initialize environment */
	env_relocate ();

#ifdef CONFIG_VFD
	/* must do this after the framebuffer is allocated */
	drv_vfd_init();
#endif /* CONFIG_VFD */

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	/* MAC Address */
	{
		int i;
		ulong reg;
		char *s, *e;
		char tmp[64];

		i = getenv_r ("ethaddr", tmp, sizeof (tmp));
		s = (i > 0) ? tmp : NULL;

		for (reg = 0; reg < 6; ++reg) {
			gd->bd->bi_enetaddr[reg] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}

#ifdef CONFIG_HAS_ETH1
		i = getenv_r ("eth1addr", tmp, sizeof (tmp));
		s = (i > 0) ? tmp : NULL;

		for (reg = 0; reg < 6; ++reg) {
			gd->bd->bi_enet1addr[reg] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
#endif
	}

	devices_init ();	/* get the devices list going. */

#ifdef CONFIG_CMC_PU2
	load_sernum_ethaddr ();
#endif /* CONFIG_CMC_PU2 */

	jumptable_init ();

	console_init_r ();	/* fully init console as a device */

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

//&*&*&*SJ1_20101004, display board info.
#ifdef CONFIG_DISPLAY_BOARDINFO
	checkboard();
#endif
//&*&*&*SJ2_20101004, display board info.

	/* enable exceptions */
	enable_interrupts ();

	/* Perform network card initialisation if necessary */
#ifdef CONFIG_DRIVER_CS8900
	cs8900_get_enetaddr (gd->bd->bi_enetaddr);
#endif

#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	if (getenv ("ethaddr")) {
		smc_set_mac_addr(gd->bd->bi_enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif	/* CFG_CMD_NET */

#ifdef BOARD_LATE_INIT
	board_late_init ();
#endif
#if (CONFIG_COMMANDS & CFG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
#endif

	g_BatteryID = GetBatteryID(); //&*&*&*SJ_20110704, add get battery id.
//&*&*&*HC1_20110304, Add offmode charging
#if defined(CONFIG_CHECK_SECOND_BOOTLOADER)
	if (FLAG_SKIP_CHECK_DIAG != share_region->flags) {
	/* off mode charging */
	off_mode_start();
  }
#else
	/* off mode charging */
	off_mode_start();
#endif
//&*&*&*SJ1_20110408, Add bootup logo.
#if defined (CONFIG_BOOTUP_LOGO)
	{
		char cmd[64];
		unsigned int *pMem = (unsigned int *)(LCD_FB_PHY_ADDR-0x40);
				
		mmc_init(1);
		sprintf(cmd, "mmc 1 read %x 1400000 190000", (LCD_FB_PHY_ADDR-0x40));	
		run_command (cmd, 0);
		if (0x56190527 != pMem[0])
			run_command("logo_black", 0);
				
		run_command("panel", 0);
	}
#endif
//&*&*&*SJ2_20110408, Add bootup logo.
//&*&*&*HC2_20110304, Add offmode charging

//&*&*&*SJ1_20110419, Implement display software and hardware version info. 
#if defined (CONFIG_SHARE_REGION)
{
	int n;
	share_region_handle();
	share_region->hardware_id = g_BoardID;
	share_region->status_3G_exist = check_3G_exist();
	share_region->battery_id  = g_BatteryID;
	memcpy(share_region->battery_barcode, g_szBatteryBarCode, sizeof(share_region->battery_barcode));	
	mmc_init(1);
	run_command("mmc 1 read 82000000 2e00000 400", 0);
	memcpy(share_region->software_version, (unsigned char *)0x82000000, sizeof(share_region->software_version));
	share_region->software_version[48] = '\0';
	n = strlen(share_region->software_version);
	if (n > 0)
	{
		if (share_region->software_version[n-1] == '\n')
			share_region->software_version[n-1] = '\0';
	}
	printf("\n3g_modem=%d,hw=%03d,sw=%s\n", share_region->status_3G_exist, share_region->hardware_id, share_region->software_version);
}
#endif
//&*&*&*SJ2_20110419, Implement display software and hardware version info.

//&*&*&*SJ1_20110620
{
	u32 boot_device = __raw_readl(0x480029c0) & 0xff;
	char *result_env=NULL;
	int PB_env, Debug, INITRAMFS_env;
	char *p;
	char szBootargs[256];

  memset(szBootargs, 0, sizeof(szBootargs));
	result_env = getenv("PB");
	PB_env = result_env ? simple_strtoul (result_env, NULL, 3) : 0;
	
	result_env = getenv("debug");
	Debug = result_env ? simple_strtoul (result_env, NULL, 6) : 1; //default '1'

	result_env = getenv("INITRAMFS");
	INITRAMFS_env = result_env ? simple_strtoul (result_env, NULL, 10) : 0;
	printf("[env]PB_env=%d,Debug=%d,INITRAMFS=%d\n", PB_env, Debug, INITRAMFS_env);

	switch(boot_device) {
		case BOOTING_TYPE_NAND: /** NAND **/
			setenv("bootargs", CONFIG_BOOTARGS_TFT_NAND_BOOT);
			break;

		case BOOTING_TYPE_eMMC: /** eMMC  (MMC2) **/
			if (PB_env == 1) {
				break;
			}
#if defined (CONFIG_GPS_DEBUG_UART_SWITCH)
			if (g_BoardID < BOARD_ID_EVT3) { //&*&*&*SJ_20110607, add check HW id.
				if (Debug) setenv("bootargs", CONFIG_INITRAMFS_BOOTARGS_TFT_EMMC_BOOT);
				else       setenv("bootargs", CONFIG_INITRAMFS_BOOTARGS_TFT_EMMC_GPS_UART_BOOT);
			    
				setenv("bootcmd", CONFIG_INITRAMFS_BOOTCOMMAND_EMMC);
			} else {
//&*&*&*SJ1_20111123, check 'debug' environment to turn on/off debug message.
#if defined (CONFIG_DEBUG_SWITCH)
			if (0 == Debug) {
			 p = strstr(CONFIG_INITRAMFS_BOOTARGS_TFT_EMMC_BOOT, ",115200n8");
			 sprintf(szBootargs, "console=null%s", p);
			 setenv("bootargs", szBootargs);
			 setenv("bootcmd", CONFIG_INITRAMFS_BOOTCOMMAND_EMMC);
			} else {
			 setenv("bootargs", CONFIG_INITRAMFS_BOOTARGS_TFT_EMMC_BOOT);
			 setenv("bootcmd", CONFIG_INITRAMFS_BOOTCOMMAND_EMMC);
			}
#else /* else CONFIG_DEBUG_SWITCH */
			 setenv("bootargs", CONFIG_INITRAMFS_BOOTARGS_TFT_EMMC_BOOT);
			 setenv("bootcmd", CONFIG_INITRAMFS_BOOTCOMMAND_EMMC);
#endif
//&*&*&*SJ2_20111123, check 'debug' environment to turn on/off debug message.
			};;
#else /* else CONFIG_GPS_DEBUG_UART_SWITCH */ 
			setenv("bootargs", CONFIG_INITRAMFS_BOOTARGS_TFT_EMMC_BOOT);
			setenv("bootcmd", CONFIG_INITRAMFS_BOOTCOMMAND_EMMC);
#endif
			break;	
		};; /* End switch() */
}
//&*&*&*SJ2_20110620

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}

#ifdef CONFIG_MODEM_SUPPORT
static inline void mdm_readline(char *buf, int bufsiz);

/* called from main loop (common/main.c) */
extern void  dbg(const char *fmt, ...);
int mdm_init (void)
{
	char env_str[16];
	char *init_str;
	int i;
	extern char console_buffer[];
	extern void enable_putc(void);
	extern int hwflow_onoff(int);

	enable_putc(); /* enable serial_putc() */

#ifdef CONFIG_HWFLOW
	init_str = getenv("mdm_flow_control");
	if (init_str && (strcmp(init_str, "rts/cts") == 0))
		hwflow_onoff (1);
	else
		hwflow_onoff(-1);
#endif

	for (i = 1;;i++) {
		sprintf(env_str, "mdm_init%d", i);
		if ((init_str = getenv(env_str)) != NULL) {
			serial_puts(init_str);
			serial_puts("\n");
			for(;;) {
				mdm_readline(console_buffer, CFG_CBSIZE);
				dbg("ini%d: [%s]", i, console_buffer);

				if ((strcmp(console_buffer, "OK") == 0) ||
					(strcmp(console_buffer, "ERROR") == 0)) {
					dbg("ini%d: cmd done", i);
					break;
				} else /* in case we are originating call ... */
					if (strncmp(console_buffer, "CONNECT", 7) == 0) {
						dbg("ini%d: connect", i);
						return 0;
					}
			}
		} else
			break; /* no init string - stop modem init */

		udelay(100000);
	}

	udelay(100000);

	/* final stage - wait for connect */
	for(;i > 1;) { /* if 'i' > 1 - wait for connection
				  message from modem */
		mdm_readline(console_buffer, CFG_CBSIZE);
		dbg("ini_f: [%s]", console_buffer);
		if (strncmp(console_buffer, "CONNECT", 7) == 0) {
			dbg("ini_f: connected");
			return 0;
		}
	}

	return 0;
}

/* 'inline' - We have to do it fast */
static inline void mdm_readline(char *buf, int bufsiz)
{
	char c;
	char *p;
	int n;

	n = 0;
	p = buf;
	for(;;) {
		c = serial_getc();

		/*		dbg("(%c)", c); */

		switch(c) {
		case '\r':
			break;
		case '\n':
			*p = '\0';
			return;

		default:
			if(n++ > bufsiz) {
				*p = '\0';
				return; /* sanity check */
			}
			*p = c;
			p++;
			break;
		}
	}
}
#endif	/* CONFIG_MODEM_SUPPORT */
