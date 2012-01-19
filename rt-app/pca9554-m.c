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

// Fonctions prototype pour les accès USER
ssize_t pca9554_write(struct file *, const char __user *, size_t, loff_t *);
ssize_t pca9554_read(struct file *, char __user *, size_t, loff_t *);

// Exportations
EXPORT_SYMBOL(pca9554_get);
EXPORT_SYMBOL(pca9554_set);

dev_t dev;

#define I2C_SLAVE 0x0703 			/* IOCTL CMD value to be passed to xeno_i2c_ioctl */

struct file_operations fops = {
	.read = pca9554_read,
	.write = pca9554_write
};

struct cdev *my_dev;

ssize_t pca9554_get(pca_io_data_t *io_data){
	struct file tmp_file;
	tmp_file.private_data = (void *)io_data;
	return pca9554_read(&tmp_file, NULL, 0, NULL);
}

ssize_t pca9554_set(pca_io_data_t *io_data){
	struct file tmp_file;
	tmp_file.private_data = (void *)io_data;
	return pca9554_write(&tmp_file, NULL, 0, NULL);
}

ssize_t pca9554_read(struct file *file, char __user *buff, size_t len, loff_t *off) {
	char datas[2];

	// On récupère le inode
	struct inode *inode;
	int type;	// Switch (0) ou led (1)?
	int val;	// Nombre du switch ou de la led
	pca_io_data_t *pca_data = NULL;

	if(file->private_data == NULL){
		// Accès depuis le USER SPACE
		inode = file->f_dentry->d_inode;
		// On determine le type et le minor
		if(iminor(inode) >= MINOR_NUM_LED0 && iminor(inode) <= MINOR_NUM_LED3){
			type = io_led;
			val = iminor(inode);
		} else if(iminor(inode) >= MINOR_NUM_SW0 && iminor(inode) <= MINOR_NUM_SW3){
			type = io_switch;
			val = iminor(inode);
		} else {
			printk(KERN_ERR "PCA9554: bad minor number (%d) while read accessing\n", iminor(inode));
			return -1;
		}
	}else{
		// Accès depuis le KERNEL SPACE
		pca_data = (pca_io_data_t *)file->private_data;
		pca_data->value = -1;

		// On determine le type et le minor
		val = pca_data->io_num;
		if(val >= 0 && val <= 7){
			switch(pca_data->io_type){
			case io_led:
				// Si les, on reogranise l'ordre
				val = 3 - val;
				type = io_led;
				break;
			case io_switch:
				type = io_switch;
				val += 4;
				break;
			default:
				printk(KERN_ERR "PCA9554: bad io type while read accessing\n");
				return -1;
			}
		} else {
			printk(KERN_ERR "PCA9554: bad minor number (%d) while read accessing\n", pca_data->io_num);
			return -1;
		}
	}

	// Récupèration des données
	if(type == io_led){
		datas[0] = 1;		// Output register
	}else{
		datas[0] = 0;		// Input register
	}
	xeno_i2c_write(datas, 1);	// On écrit la valeur de registre
	xeno_i2c_read(datas, 1);	// On lit le résultat

	// Traitement des données
	datas[0] = (~(datas[0] >> val)) & 0x01;

	if(file->private_data == NULL){
		// Renvoi vers le USER SPACE
		if(copy_to_user(buff, datas, 1) != 0)
		{
			printk(KERN_ERR "PCA9554: error while copying datas to USER\n");
			return -1;
		}
	}else{
		// Renvoi vers le KERNEL SPACE
		pca_data->value = datas[0];
	}
	return 1;
}

ssize_t pca9554_write(struct file *file, const char __user *buff, size_t len, loff_t *off) {
	char cmd[2], datas[2];
	// On récupère le inode
	struct inode *inode;
	int type;	// Switch (0) ou led (1)?
	int val;	// Nombre du switch ou de la led
	pca_io_data_t *pca_data = NULL;

	if(file->private_data == NULL){
		// Accès depuis le USER SPACE
		inode = file->f_dentry->d_inode;
		// On determine le type et le minor
		if(iminor(inode) >= MINOR_NUM_LED0 && iminor(inode) <= MINOR_NUM_LED3){
			type = io_led;
			val = iminor(inode);
		} else if(iminor(inode) >= MINOR_NUM_SW0 && iminor(inode) <= MINOR_NUM_SW3){
			type = io_switch;
			val = iminor(inode);
		} else {
			printk(KERN_ERR "PCA9554: bad minor number (%d) while write accessing\n", iminor(inode));
			return -1;
		}

		// Si accès en écriture sur les switchs,on quitte avec erreur
		if(type == io_switch)
			return 1;
	}else{
		// Accès depuis le KERNEL SPACE
		pca_data = (pca_io_data_t *)file->private_data;

		// On determine le type et le minor
		val = pca_data->io_num;
		//printk(KERN_INFO "IO num while write call: %d\n", val);
		if(val >= 0 && val <= 7){
			switch(pca_data->io_type){
			case io_led:
				// Si led, on reorganise l'ordre des leds
				val = 3 - val;
				type = io_led;
				break;
			case io_switch:
				// Si accès en écriture sur les switchs,on quitte avec erreur
				return -1;
			default:
				printk(KERN_ERR "PCA9554: bad io type while write accessing\n");
				return -1;
			}
		} else {
			printk(KERN_ERR "PCA9554: bad minor number (%d) while write accessing\n", pca_data->io_num);
			return -1;
		}

	}

	/**
	 * On lit les valeurs contenues dans les registres du PCA9554 pour
	 * pouvoir les masqué et les renvoyé
	 */
	cmd[0] = 1;					// Output register
	xeno_i2c_write(cmd, 1);		// On écrit la valeur de registre
	xeno_i2c_read(datas, 1);	// On lit le résultat du registre de sortie
	datas[0] = ~datas[0];

	if(file->private_data == NULL){
		// On prend les données depuis le USER SPACE
		if(copy_from_user(&datas[1], buff, 1) != 0)
		{
			printk(KERN_ERR "PCA9554: error while copying datas from USER\n");
			return -1;
		}
	}else{
		// On prend les données depuis le KERNEL SPACE
		datas[1] = pca_data->value;
	}

	// On fait le masquage des nos sortie
	if(datas[1] == 0){
		datas[0] &= ~(0x01 << val);
	}else{
		datas[0] |= (0x01 << val);
	}

	/**
	 * On réécrit nos données dans le PCA9554
	 */
	cmd[0] = 1;					// Output register
	cmd[1] = ~datas[0];
	xeno_i2c_write(cmd, 2);		// On écrit la valeur de registre

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
