/*
 * io_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include "io_task.h"
#include "pca9554-m.h"

/**
 * Variables privées
 */
RT_TASK io_task_handle;
static uint8_t io_task_created = 0;
static struct file tmp_file;
char tmp_buf;
loff_t tmp_loff;
pca_io_data_t io_data;

/**
 * Fonctions privées
 */
static void io_task(void *cookie);

int io_task_start(){
	int err;
	err = rt_task_create(&io_task_handle,
	                     "task_io",
	                      TASK_STKSZ,
	                      TASK_IO_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&io_task_handle, &io_task, NULL);
		io_task_created = 1;
		if(err != 0){
			printk("rt-app: Task IO starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task IO starting succeed\n");
		}
	}else{
		printk("rt-app: Task IO creation failed\n");
		goto fail;
	}
	return 0;
fail:
	io_task_cleanup_task();
	io_task_cleanup_objects();
	return -1;
}

void io_task_cleanup_task(){
	if(io_task_created){
		io_task_created = 0;
		rt_task_delete(&io_task_handle);
	}
}

void io_task_cleanup_objects(){
}

void io_task(void *cookie)
{
	int i;
	char modif;
	char old_val_sw[4], new_val_sw[4];
	tmp_file.private_data = (void *)&io_data;
	rt_task_set_periodic(NULL, TM_NOW, 10000000);

    for (;;) {
    	rt_task_wait_period(NULL);

		modif = 0;
		for(i = 0; i < 4; i++){
			io_data.io_num = i;
			io_data.io_type = io_switch;

			//pca9554_read(&tmp_file, NULL, 0, NULL);
			pca9554_get(&io_data);
			new_val_sw[i] = io_data.value;
			if(old_val_sw[i] != new_val_sw[i]){

				if(new_val_sw[i] == 1){
					//printk("sw[%d] pressed\n", i);
					// On s'occupe des leds
					io_data.io_num = i;
					io_data.io_type = io_led;
					io_data.value = 1;
					pca9554_set(&io_data);
					//pca9554_write(&tmp_file, NULL, 1, NULL);
					//write(fd_led[i], &new_val_sw[i], 1);
					//printk("led[%d] turned on\n", i);
				}else{
					//printk(KERN_INFO "sw[%d] released\n", i);
					// On s'occupe des leds
					io_data.io_num = i;
					io_data.io_type = io_led;
					io_data.value = 0;
					pca9554_set(&io_data);
					//pca9554_write(&tmp_file, NULL, 1, NULL);
					//write(fd_led[i], &new_val_sw[i], 1);
					//printk("led[%d] turned off\n", i);
				}
				modif = 1;
			}
		}
		for(i = 0; i < 4; i++){
			if(old_val_sw[i] == new_val_sw[i] && modif && new_val_sw[i] == 1){
				printk("sw[%d] hold\n", i);
			}
			old_val_sw[i] = new_val_sw[i];
		}
    }
}

