/*
 * enemies.c
 *
 *  Created on: Dec 22, 2011
 *      Author: michael
 */
#include <linux/types.h>
#include "lcdlib.h"
#include "invaders_task.h"

/**
 * Variables public
 */
invader_t invaders[NB_INVADERS];


/**
 * Variables privées
 */
RT_MUTEX invaders_task_mutex;
uint8_t invaders_task_mutex_created = 0;

RT_TASK invaders_task_handle;
uint8_t invaders_task_created = 0;

static int level_finish = 0;

/**
 * Fonctions privées
 */
static void invaders_task(void *cookie);
static void invaders_init(void);
static void invaders_move(void);
static void invaders_get_wave_box(hitbox_t *wave_hitbox);

int invaders_task_start(){
	int err;

	err = rt_mutex_create(&invaders_task_mutex, "task_invader_mutex");
	if(err == 0){
		invaders_task_mutex_created = 1;
		printk("rt-app: Task INVADERS create mutex succeed\n");
	}else{
		printk("rt-app: Task INVADERS create mutex failed\n");
		goto fail;
	}

	err = rt_task_create(&invaders_task_handle,
	                     "task_invaders",
	                      TASK_STKSZ,
	                      TASK_INVADERS_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&invaders_task_handle, &invaders_task, NULL);
		invaders_task_created = 1;
		if(err != 0){
			printk("rt-app: Task INVADERS starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task INVADERS starting succeed\n");
		}
	}else{
		printk("rt-app: Task INVADERS creation failed\n");
		goto fail;
	}
	return 0;
fail:
	invaders_task_cleanup_task();
	invaders_task_cleanup_objects();

	return -1;
}

void invaders_task_cleanup_task(){
	if(invaders_task_created){
		printk("rt-app: Task INVADERS cleanup task\n");
		invaders_task_created = 0;
		rt_task_delete(&invaders_task_handle);
	}
}

void invaders_task_cleanup_objects(){
	if(invaders_task_mutex_created){
		printk("rt-app: Task INVADERS cleanup objects\n");
		invaders_task_mutex_created = 0;
		rt_mutex_delete(&invaders_task_mutex);
	}
}

static void invaders_task(void *cookie){

	rt_task_set_periodic(NULL, TM_NOW, 100000000);
	invaders_lock();
	invaders_init();
	invaders_unlock();

	for (;;) {
		rt_task_wait_period(NULL);

		if (level_finish){
			level_up();
			level_finish=0;
			invaders_lock();
			invaders_init();
			invaders_unlock();
		}

		invaders_lock();
		invaders_move();
		invaders_unlock();
	}
}

 void invaders_init(){
	 int i;


	 for (i=0;i<7;i++){
	 			 invaders[i].hp=HP_INVADER;
	 			 invaders[i].hitbox.height=INVADER_HEIGHT;
	 			 invaders[i].hitbox.width=INVADER_WIDTH;
	 			 invaders[i].hitbox.type = G_INVADER;

	 }

	 for (i=0;i<4;i++){
			 invaders[i].hitbox.x = (3*SPACE_BETWEEN_INVADER)+(i*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
			 invaders[i].hitbox.y = 30;
	 }


	 for (i=0;i<3;i++){
			 invaders[i+4].hitbox.x = (3*SPACE_BETWEEN_INVADER+(WIDTH_INVADER))+(i*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
			 invaders[i+4].hitbox.y = 60;
	 }



 }

 void invaders_move(){
	 int i;
	 int x = 0;
	 static int moving_right = 1;
	 hitbox_t dimension;
	 int invader_dead=0;

	 invaders_get_wave_box(&dimension);


	 if (moving_right){
		 if ((dimension.x + dimension.width + current_wave.invader_speed) < LCD_MAX_X){
			 x = current_wave.invader_speed;
		 }else{
			 x = LCD_MAX_X - (dimension.x + dimension.width);
			 moving_right = 0;
		 }
	 }
	 else{
		 if (((int32_t)dimension.x - (int32_t)current_wave.invader_speed) > 0){
			 x = -current_wave.invader_speed;
		 }else{
			 x = -dimension.x;
			 moving_right = 1;
		 }
	 }

	 for (i=0; i<NB_INVADERS;i++){
		 invaders[i].hitbox.x += x;
		 //Count the number of invader dead
		 if (invaders[i].hp<=0)
			 invader_dead++;
	 }

	 // test if level finish
	 if (invader_dead == NB_INVADERS)
		 level_finish = 1;
 }

 //return  hitboxes from wave
 void invaders_get_wave_box(hitbox_t *wave_hitbox){

	 int i;

	 wave_hitbox->x = LCD_MAX_X;
	 wave_hitbox->y = LCD_MAX_Y;
	 wave_hitbox->width = 0;
	 wave_hitbox->height = 0;

	 //Detection hitbox top
	 for (i=0;i<NB_INVADERS;i++){
		 if(invaders[i].hp > 0){
			 //Detection x
			 if (invaders[i].hitbox.x < wave_hitbox->x){
				 wave_hitbox->x = invaders[i].hitbox.x;
			 }
			 //Detection y
			 if (invaders[i].hitbox.y < wave_hitbox->y){
				 wave_hitbox->y = invaders[i].hitbox.y+1;
			 }

			 //Detection width
			 if (invaders[i].hitbox.x+invaders[i].hitbox.width >
			 	 wave_hitbox->x + wave_hitbox->width){
				 wave_hitbox->width = (invaders[i].hitbox.x+invaders[i].hitbox.width)-wave_hitbox->x-1;
			 }
			 //Detection height
			 if (invaders[i].hitbox.y+invaders[i].hitbox.height >
				 wave_hitbox->y + wave_hitbox->height){
				 wave_hitbox->height = (invaders[i].hitbox.y+invaders[i].hitbox.height)-wave_hitbox->y;
			 }
		 }
	 }
}

int invaders_lock(){
	if(invaders_task_mutex_created){
		return rt_mutex_lock(&invaders_task_mutex, TM_INFINITE);
	}
	return -1;
}

int invaders_unlock(){
	if(invaders_task_mutex_created){
		return rt_mutex_unlock(&invaders_task_mutex);
	}
	return -1;
}



