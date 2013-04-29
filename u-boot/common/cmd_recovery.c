#include <asm/byteorder.h>
#include <common.h>
#include <command.h>
#include <nand.h>
#include <fastboot.h>
#include <environment.h>

//&*&*&*20110304_Peter ++
int do_recovery_mode (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int boot_device = __raw_readl(0x480029c0) & 0xff;
	int result=0;

	setenv("bootargs", CONFIG_BOOTARGS_UPDATE_MODE);
	
	switch(boot_device) {
	case BOOTING_TYPE_NAND: /** NAND **/
		puts("/** NAND **/\n");
		run_command (CONFIG_BOOTCOMMAND_NAND_UPDATE_MODE, 0);
		break;

	case BOOTING_TYPE_eMMC: /** eMMC **/
		puts("/** eMMC **/\n");
		run_command (CONFIG_BOOTCOMMAND_EMMC_UPDATE_MODE, 0);
		break;
		
	case BOOTING_TYPE_SD1:  /** SD1 **/
		puts("/** SD1 **/");
		run_command("mmcinit 0; fatload mmc 0 82000000 recovery.img", 0);
		result = getenv("filesize");
		if (result) {
			run_command("bootm 82000000", 0);
		} else {
			run_command (CONFIG_BOOTCOMMAND_EMMC_UPDATE_MODE, 0);
		}
		break;	
	}
}

U_BOOT_CMD(
	recoverymode,	2,	1,	do_recovery_mode,
	"recovery - eMMC/NAND recovery mode\n",
);
//&*&*&*20110304_Peter --

