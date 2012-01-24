/*
 * io_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include "io_task.h"
#include "hit_task.h"
#include "pca9554-m.h"
#include "rt-app-m.h"
#include "ship_task.h"

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
	uint8_t rebond = 0;
	uint8_t counter_io = 0;
	char modif;
	char old_val_sw[4], new_val_sw[4];
	tmp_file.private_data = (void *)&io_data;

	(void)cookie;
	// On d�finit la p�riode de la tache
	rt_task_set_periodic(NULL, TM_NOW, 50*MS);

    for (;;) {
    	rt_task_wait_period(NULL);

    	if(counter_io == 1){
    		counter_io = 0;

			modif = 0;
			for(i = 0; i < 4; i++){

				// Gestion de la recharge
				weapons[i+1].timing_charge.time_current++;
				if(weapons[i+1].timing_charge.time_current >= weapons[i+1].timing_charge.time_total){
					weapons[i+1].timing_charge.time_current = 0;
					if(weapons[i+1].timing_charge.now < weapons[i+1].timing_charge.max){
						weapons[i+1].timing_charge.now++;
					}
				}

				io_data.io_num = i;
				io_data.io_type = io_switch;

				//pca9554_read(&tmp_file, NULL, 0, NULL);
				pca9554_get(&io_data);
				new_val_sw[i] = io_data.value;
				if(old_val_sw[i] != new_val_sw[i]){
					rebond = 0;
					if(new_val_sw[i] == 1){

						if(weapons[i+1].timing_charge.now > 0){
							weapons[i+1].timing_charge.now--;
							weapons[i+1].timing_charge.last = weapons[i+1].timing_charge.now;
							fire_weapon(ship.hitbox,(weapontype_t)(i+1));

						}

					}
					old_val_sw[i] = new_val_sw[i];
				}else if(new_val_sw[i] == 1){
					if(rebond > 2){
						if(weapons[i+1].timing_charge.now > 0 &&
						   weapons[i+1].timing_charge.now >= weapons[i+1].timing_charge.last){
							weapons[i+1].timing_charge.now--;
							fire_weapon(ship.hitbox,(weapontype_t)(i+1));
						}
					}else{
						rebond++;
					}
				}
			}
		}else{
			counter_io++;
		}
    	for(i = 0; i < 4; i++){
			// Gestion de la led
			io_data.io_num = i;
			io_data.io_type = io_led;
			// Si vide, on eteint la led
			weapons[i+1].timing_led.ratio = (weapons[i+1].timing_led.max*(weapons[i+1].timing_charge.now*100/weapons[i+1].timing_charge.max)/100);

			if(weapons[i+1].timing_led.now++ >= (weapons[i+1].timing_led.max - weapons[i+1].timing_led.ratio) ){
				io_data.value = 1;
			}else{
				io_data.value = 0;
			}
			if(weapons[i+1].timing_led.now >= weapons[i+1].timing_led.ratio){
				weapons[i+1].timing_led.now = 0;
			}

			// On définit la led
			//pca9554_set(&io_data);
    	}
    }
}

