#include <common.h>
#include <config.h>
#include <command.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <dm.h>

static int do_at24c08(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	unsigned char write_str[] = "Hello world\n";
	unsigned char read_str[15] = {0};
	struct udevice *dev, *chip;

	ret = uclass_get_device(UCLASS_I2C, 0, &dev);
	if (ret)
		goto exit;
	ret = dm_i2c_probe(dev, 0x50, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS, &chip);
	if (ret)
		goto exit;

	printf("device: %s\n", chip->name);
	printf("write_str : %s\n", write_str);
	dm_i2c_write(chip, 2, write_str, sizeof(write_str));
	dm_i2c_read(chip, 2, read_str, 13);
	printf("read_str: %s\n", read_str);
	printf("Enter: %s\n", __func__);
	return 0;
exit:
	printf("%s, error\n", __func__);
	return 0;
}

U_BOOT_CMD(
	at24c08, 8,	1,	do_at24c08,
	"at24c08 read and write",
	""
)
