/*******************************************************************
 * pca9554.c - HEIG-VD 2008, Cours IEM
 *
 * Author: DRE
 * Date: December 2008
 *******************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#include "pca9554-m.h"
#include "xeno-i2c.h"

#define I2C_SLAVE 0x0703 	/* IOCTL CMD value to be passed to xeno_i2c_ioctl */

int pca9554_open(struct inode *inode, struct file *file) {

	/* A compléter ... */

	return 0;
}

int pca9554_ioctl(struct inode * inode, struct file *file, unsigned int cmd, unsigned long arg) {

	/* A compléter ... */

	return 0;
}

int pca9554_close(struct inode *inode, struct file *file) {

	/* A compléter ... */

	return 0;
}

ssize_t pca9554_read(struct file *file, char __user *buff, size_t len, loff_t *off) {

	/* A compléter ... */

	return 0;
}

ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *off) {

	/* A compléter ... */

	return 0;
}

int __init init_module(void) {

	/* A compléter ... */

	return 0;
}

void __exit cleanup_module(void) {

	/* A compléter ... */

}

MODULE_LICENSE("GPL");
