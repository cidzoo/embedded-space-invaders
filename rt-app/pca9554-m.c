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
#include <linux/kdev_t.h>
#include <asm/uaccess.h>


#include "pca9554-m.h"
#include "xeno-i2c.h"

#define MAJOR_NUM			52

#define MINOR_NUM_LED0		0
#define MINOR_NUM_LED1		1
#define MINOR_NUM_LED2		2
#define MINOR_NUM_LED3		3
#define MINOR_NUM_SW0		4
#define MINOR_NUM_SW1		5
#define MINOR_NUM_SW2		6
#define MINOR_NUM_SW3		7

#define MINOR_COUNT			8

#define TYPE_SWITCH			0
#define TYPE_LED			1

// Exportations
EXPORT_SYMBOL(pca9554_read);
EXPORT_SYMBOL(pca9554_write);

dev_t dev;

#define DEVICE_NAME "io_expander"	/* Dev name as it appears in /proc/devices   */

#define I2C_SLAVE 0x0703 			/* IOCTL CMD value to be passed to xeno_i2c_ioctl */

struct file_operations fops = {
	.read = pca9554_read,
	.write = pca9554_write
};

struct cdev *my_dev;

ssize_t pca9554_read(struct file *file, char __user *buff, size_t len, loff_t *off) {
	char datas[2];

	// On récupère le inode
	struct inode *inode = file->f_dentry->d_inode;
	int type;	// Switch (0) ou led (1)?
	int val;	// Nombre du switch ou de la led

	// On determine le type et le minor
	if(iminor(inode) >= MINOR_NUM_LED0 && iminor(inode) <= MINOR_NUM_LED3){
		type = TYPE_LED;
		val = iminor(inode);
	} else if(iminor(inode) >= MINOR_NUM_SW0 && iminor(inode) <= MINOR_NUM_SW3){
		type = TYPE_SWITCH;
		val = iminor(inode);
	} else {
		printk(KERN_ERR "PCA9554: bad minor number (%d) while read accessing\n", iminor(inode));
		return -1;
	}

	// Récupèration des données
	if(type == TYPE_LED){
		datas[0] = 1;		// Output register
	}else{
		datas[0] = 0;		// Input register
	}
	xeno_i2c_write(datas, 1);	// On écrit la valeur de registre
	xeno_i2c_read(datas, 1);	// On lit le résultat

	// Traitement des données
	datas[0] = (~(datas[0] >> val)) & 0x01;

	// On envoie les données vers l'espace USER
	if(copy_to_user(buff, datas, 1) != 0)
	{
		printk(KERN_ERR "PCA9554: error while copying datas to USER\n");
		return -1;
	}

	return 1;
}

ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *off) {
	char cmd[2], datas[2];
	// On récupère le inode
	struct inode *inode = file->f_dentry->d_inode;
	int type;	// Switch (0) ou led (1)?
	int val;	// Nombre du switch ou de la led

	if(file->private_data == NULL){
		// L'appel vient du USER space
		printk(KERN_INFO "USER SPACE CALL\n");
		// On determine le type et le minor
		if(iminor(inode) >= MINOR_NUM_LED0 && iminor(inode) <= MINOR_NUM_LED3){
			type = TYPE_LED;
			val = iminor(inode);
		} else if(iminor(inode) >= MINOR_NUM_SW0 && iminor(inode) <= MINOR_NUM_SW3){
			type = TYPE_SWITCH;
			val = iminor(inode);
		} else {
			printk(KERN_ERR "PCA9554: bad minor number (%d) while read accessing\n", iminor(inode));
			return -1;
		}

		// Récupèration des données
		if(type == TYPE_SWITCH)
			return 1;


		cmd[0] = 1;					// Output register
		xeno_i2c_write(cmd, 1);		// On écrit la valeur de registre
		xeno_i2c_read(datas, 1);	// On lit le résultat du registre de sortie
		datas[0] = ~datas[0];

		// On fait le masquage des nos sortie
		if(copy_from_user(&datas[1], buff, 1) != 0)
		{
			printk(KERN_ERR "PCA9554: error while copying datas to USER\n");
			return -1;
		}
		if(datas[1] == 0){
			datas[0] &= ~(0x01 << val);
		}else{
			datas[0] |= (0x01 << val);
		}

		cmd[0] = 1;					// Output register
		cmd[1] = ~datas[0];
		xeno_i2c_write(cmd, 2);		// On écrit la valeur de registre

		return 1;
	}else{
		printk(KERN_INFO "KERNEL SPACE CALL\n");
	}
	return 1;
}

int __init init_module(void) {
	char datas[2];	// Donnée à envoyé
	dev_t dev;
	// Initialisation de l'i2c
	if(xeno_i2c_init() == 0){
		printk(KERN_INFO "PCA9554: i2c init success\n");
	}else{
		printk(KERN_ERR "PCA9554: i2c init error\n");
		return -1;
	}
	// Configuration de l'adresse de l'esclave
	if(xeno_i2c_ioctl(I2C_SLAVE, 0x0020) == 0){
		printk(KERN_INFO "PCA9554: i2c configuration slave address success\n");
	}else{
		printk(KERN_ERR "PCA9554: i2c configuration slave address error\n");
		return -1;
	}

	// On configure le PCA9554 pour avoir les 4 switch en entrée et les 4 leds en sortie

	// On définit le sens des I/Os (entrée ou sortie)
	datas[0] = 3;		// Configuration register
	datas[1] = 0xF0;	// Valeurs
	if(xeno_i2c_write(datas, 2) == 0){
		printk(KERN_INFO "PCA9554: i2c setup I/Os success\n");
	}else{
		printk(KERN_ERR "PCA9554: i2c setup I/Os error\n");
		return -1;
	}

	// On inverse la polarité pour les leds
	datas[0] = 2;		// Polarity register
	datas[1] = 0x00;	// Valeurs
	if(xeno_i2c_write(datas, 2) == 0){
		printk(KERN_INFO "PCA9554: i2c setup polarity success\n");
	}else{
		printk(KERN_ERR "PCA9554: i2c setup polarity error\n");
		return -1;
	}

	datas[0] = 1;		// Output register
	datas[1] = 0x0F;	// On eteind les leds
	xeno_i2c_write(datas, 2);

	// Insertion du module avec c 52 0
	dev = MKDEV(MAJOR_NUM, 0);

	my_dev = cdev_alloc();
	cdev_init(my_dev, &fops);

	my_dev->owner = THIS_MODULE;
	cdev_add(my_dev, dev, 8);
	printk(KERN_INFO "PCA9554: module insertion terminated\n");
	return 0;
}

void __exit cleanup_module(void) {
	char datas[2];
	datas[0] = 1;		// Output register
	datas[1] = 0x0F;	// On eteind les leds
	xeno_i2c_write(datas, 2);

	printk(KERN_INFO "PCA9554: ic2 cleanup module\n");
	xeno_i2c_exit();
	cdev_del(my_dev);
	printk(KERN_INFO "PCA9554: cleanup module terminated\n");
}

MODULE_LICENSE("GPL");
