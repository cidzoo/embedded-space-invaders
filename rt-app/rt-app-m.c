/******************************************************************************
 * File		: rt-app-m.c
 *
 * Authors	: (DRE), (LSU) 2011
 *
 * Comment	:
 *  The lab goal is to implement a space invader clone on the laboratory boards,
 *  as a hard realtime Xenomai kernel application.
 *
 ******************************************************************************/
#include <linux/module.h>

#include <native/task.h>
#include <native/intr.h>
#include <native/event.h>
#include <native/alarm.h>
#include <native/timer.h>
#include <native/mutex.h>

#include "xeno-i2c.h"
#include "xeno-ts.h"
#include "lcdlib.h"
#include "pca9554-m.h"
#include "rt-app-m.h"

static RT_INTR isrDesc;

static int space_invader(void)
{
	int err;


	/* To Be Completed */


	err = rt_intr_enable(&isrDesc);

	if (err != 0) {
		printk("rt-app: Could not enable I2C ISR\n");
		goto fail;
	}

	printk("rt-app: Enabled ISR\n");

	return 0;

fail:
	cleanup_module();
	return -1;

}

/* Interrupt service routine to handle the I2C interrupt */
static int imx_i2c_handler(struct xnintr* _idesc) {

	/* safe status register */
	set_i2c_imx_i2sr(get_i2c_imx_reg()->i2sr);

	/* if data transfer is complete set ok */
	if (get_i2c_imx_i2sr() & (u32)0x80)  /* [I2SR:ICF] TX complete */
		set_i2c_imx_irq_ok(1);

	/* clear irq */
	get_i2c_imx_reg()->i2sr = get_i2c_imx_reg()->i2sr & ~(u32)0x02; /* clear [I2SR:IIF] Interrupt */

	return RT_INTR_HANDLED; /* Ne propage pas l'irq dans le domaine linux */
}

int __init init_module(void) {
	int err;

	/* Initialisation du timer */
	if (TIMER_PERIODIC)
		err = rt_timer_set_mode(MS);
	else
		err = rt_timer_set_mode(TM_ONESHOT);

	if (err != 0) {
		printk("rt-app: %s: Error timer: %d\n", __func__, err);
		return -1;
	}

	/* Initialize FB */
	fb_init();
	printk("rt-app: Framebuffer initialized\n");

	xeno_ts_init();
	printk("rt-app: Touchscreen initialized\n");

	/* Open Philips controller */
	err = pca9554_open(NULL, NULL);
	if (err != 0) {
		printk("rt-app: %s: I2C slave open error: %d\n", __func__, err);
		goto fail_open;
	}

	/* Initializing IRQ */
	err = rt_intr_create(&isrDesc, "IMX_I2C", INT_I2C, imx_i2c_handler, NULL, 0);
	if (err != 0) {
		printk("rt-app: %s: Error interrupt registration: %d\n", __func__, err);
		goto fail_intr;
	}
	printk("rt-app: ISR initialized\n");


	/* To Be Completed */


	return space_invader();

fail_open:
	pca9554_close(NULL, NULL);

fail_intr:
	xeno_ts_exit();

	return -1;
}

void __exit cleanup_module(void) {


	/* To Be Completed */


	rt_intr_delete(&isrDesc);

	pca9554_close(NULL, NULL);

	xeno_ts_exit();
}

MODULE_LICENSE("GPL");
