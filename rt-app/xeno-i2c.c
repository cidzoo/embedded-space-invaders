/*
    xeno_i2c.c - Xenomai wrapper functions for using i2c driver.

    (c) 2007 D. Rossier, REDS Institute, HEIG-VD

*/

/* Note that this is a complete rewrite of Simon Vogl's i2c-dev module.
   But I have used so much of his original code and ideas that it seems
   only fair to recognize him as co-author -- Frodo */

/* The I2C_RDWR ioctl code is written by Kolja Waschk <waschk@telos.de> */

/* The devfs code is contributed by Philipp Matthias Hahn
   <pmhahn@titan.lahn.de> */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
/*#include <linux/devfs_fs_kernel.h>*/
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <asm/uaccess.h>

struct i2c_dev {
  int minor;
  struct i2c_adapter *adap;
  struct class_device class_dev;
  struct completion released;
};

static struct i2c_client i2cdev_client_template;
struct i2c_client *client;

extern struct i2c_dev *i2c_dev_get_by_minor(unsigned index);
/*extern struct i2c_dev *i2c_dev_get_by_adapter(struct i2c_adapter *adap);*/

#define to_i2c_dev(d) container_of(d, struct i2c_dev, class_dev)

ssize_t xeno_i2c_read(char *buf, size_t count)
{
  char *tmp;
  int ret;

  if (count > 8192)
    count = 8192;

  tmp = kmalloc(count,GFP_KERNEL);
  if (tmp==NULL)
    return -ENOMEM;

  ret = i2c_master_recv(client,tmp,count);
  memcpy(buf, tmp, count);

  kfree(tmp);
  return ret;
}

ssize_t xeno_i2c_write (const char *buf, size_t count)
{
  int ret;
  char *tmp;

  if (count > 8192)
    count = 8192;

  tmp = kmalloc(count,GFP_KERNEL);
  if (tmp==NULL)
    return -ENOMEM;

  memcpy(tmp, buf, count);

  ret = i2c_master_send(client,tmp,count);
  kfree(tmp);
  return ret;
}

int xeno_i2c_ioctl(unsigned int cmd, unsigned long arg) {

  switch ( cmd ) {
    case I2C_SLAVE:
      if ((arg > 0x3ff) ||
	(((client->flags & I2C_M_TEN) == 0) && arg > 0x7f))
	return -EINVAL;
      if ((cmd == I2C_SLAVE) && i2c_check_addr(client->adapter,arg))
	return -EBUSY;
      client->addr = arg;
      return 0;
    default:
      return i2c_control(client,cmd,arg);
  }
  return 0;
}

/* Initializing i2c driver from the kernel space */
int xeno_i2c_init(void)
{
  unsigned int minor = 0;

  struct i2c_adapter *adap;
  struct i2c_dev *i2c_dev;

  i2c_dev = i2c_dev_get_by_minor(minor);
  if (!i2c_dev)
    return -ENODEV;

  adap = i2c_get_adapter(i2c_dev->adap->nr);

  if (!adap)
    return -ENODEV;

  client = kmalloc(sizeof(*client), GFP_KERNEL);
  if (!client) {
    i2c_put_adapter(adap);
    return -ENOMEM;
  }
  memcpy(client, &i2cdev_client_template, sizeof(*client));

  /* registered with adapter, passed as client to user */
  client->adapter = adap;

  return 0;
}

int xeno_i2c_exit(void)
{
  i2c_put_adapter(client->adapter);
  kfree(client);

  return 0;
}

