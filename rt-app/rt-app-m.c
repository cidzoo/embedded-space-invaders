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

#include "xeno-i2c.h"
#include "xeno-ts.h"
#include "lcdlib.h"
#include "pca9554-m.h"
#include "rt-app-m.h"
#include "global.h"

// Inclusion des headers pour les tâches
#include "invaders_task.h"
#include "fb_task.h"
#include "io_task.h"
#include "hit_task.h"
#include "ship_task.h"
#include "lcdlib.h"

RT_INTR isrDesc;

RT_HEAP heap;
uint8_t heap_created = 0;
uint8_t heap_allocated = 0;

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

	err = rt_heap_create(&heap, "rt_heap", 240*320*2, 0);

	if(err != 0){
		printk("rt-app: Could not create the rt heap\n");
		goto fail;
	}else{
		heap_created = 1;
	}
	printk("rt-app: RT-Heap created\n");


	err = rt_heap_alloc(&heap, 240*320*2, TM_NONBLOCK, &fb_mem_rt);
	if(err != 0){
		printk("rt-app: Could not allocate the rt heap\n");
		goto fail;
	}else{
		heap_allocated = 1;
	}
	printk("rt-app: RT-Heap allocated\n");

	if(invaders_task_start() != 0){
		goto fail;
	}

	if(hit_task_start() != 0){
		goto fail;
	}

	if(io_task_start() != 0){
		goto fail;
	}

	if(fb_task_start() != 0){
		goto fail;
	}

	if(ship_task_start() != 0){
		goto fail;
	}

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
	/*err = pca9554_open(NULL, NULL);
	if (err != 0) {
		printk("rt-app: %s: I2C slave open error: %d\n", __func__, err);
		goto fail_open;
	}*/

	/* Initializing IRQ */
	err = rt_intr_create(&isrDesc, "IMX_I2C", INT_I2C, imx_i2c_handler, NULL, 0);
	if (err != 0) {
		printk("rt-app: %s: Error interrupt registration: %d\n", __func__, err);
		goto fail_intr;
	}
	printk("rt-app: ISR initialized\n");

	printk("rt-app: i2c driver call\n");

	printk("rt_app: i2c driver ok\n");
	/* To Be Completed */


	return space_invader();

//fail_open:
	//pca9554_close(NULL, NULL);

fail_intr:
	xeno_ts_exit();

	return -1;
}

void __exit cleanup_module(void) {

	// Cleanup tasks
	invaders_task_cleanup_task();
	fb_task_cleanup_task();
	io_task_cleanup_task();
	hit_task_cleanup_task();
	ship_task_cleanup_task();

	// Cleanup objects
	invaders_task_cleanup_objects();
	fb_task_cleanup_objects();
	io_task_cleanup_objects();
	hit_task_cleanup_objects();
	ship_task_cleanup_objects();

	if(heap_allocated){
		heap_allocated = 0;
		rt_heap_free(&heap, &fb_mem_rt);
	}

	if(heap_created){
		heap_created = 0;
		rt_heap_delete(&heap);
	}

	rt_intr_delete(&isrDesc);

	//pca9554_close(NULL, NULL);

	xeno_ts_exit();
}

MODULE_LICENSE("GPL");
