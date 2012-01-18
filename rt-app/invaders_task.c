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
static RT_MUTEX invaders_task_mutex;
static uint8_t invaders_task_mutex_created = 0;

static RT_TASK invaders_task_handle;
static uint8_t invaders_task_created = 0;

static int moving_invader = 10;
static int wave_row=1;
static int nb_invaders_per_line[NB_INVADERS];

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
	invaders_init();
	return 0;
fail:
	invaders_task_cleanup();
	return -1;
}

void invaders_task_cleanup(){
	if(invaders_task_created){
		invaders_task_created = 0;
		rt_task_delete(&invaders_task_handle);
	}
	if(invaders_task_mutex_created){
		invaders_task_mutex_created = 0;
		rt_mutex_delete(&invaders_task_mutex);
	}
}

static void invaders_task(void *cookie){
	rt_task_set_periodic(NULL, TM_NOW, 10000000);

	invaders_lock();
	invaders_init();
	invaders_unlock();

	for (;;) {
		rt_task_wait_period(NULL);

		invaders_lock();
		invaders_move();
		invaders_unlock();
		/** todo */
	}
}

 void invaders_init(){

	 /*
	 int i;
	 int nb_max_invaders_per_line=0;
	 int id_invader=0;

	 //calculate the max number of invaders per lines
	 do
	 {
		 nb_max_invaders_per_line++;
	 }while ((LCD_MAX_X-(nb_max_invaders_per_line*WIDTH_INVADER)-(nb_max_invaders_per_line*SPACE_BETWEEN_INVADER))>0);


	 for (i=0;i<NB_INVADERS;i++){
		 invaders[i].hp=HP_INVADER;
		 invaders[i].hitbox.height=HEIGT_INVADER;
		 invaders[i].hitbox.width=WIDTH_INVADER;

		 if((NB_INVADERS-nb_max_invaders_per_line)<nb_max_invaders_per_line)
			 wave_row++;

		 nb_invaders_per_line[wave_row-1]++;
	 }

	 for (i=0 ; i<wave_row; i++){

		 int j;
		 for (j=0; j< nb_invaders_per_line[j]; j++){

			 if (nb_invaders_per_line[j] < nb_max_invaders_per_line){
				 invaders[id_invader].hitbox.x = (SPACE_BETWEEN_INVADER+WIDTH_INVADER)*(j+1);
				 invaders[id_invader].hitbox.y = (2*SPACE_BETWEEN_INVADER)*(i+1);
			 }
			 else{
				 invaders[id_invader].hitbox.x = (SPACE_BETWEEN_INVADER)+(j*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
				 invaders[id_invader].hitbox.y = (2*SPACE_BETWEEN_INVADER)*(i+1);
			 }

			 id_invader++;
		 }
	 }*/

	 int i;

	 for (i=0;i<4;i++){
			 invaders[i].hp=HP_INVADER;
			 invaders[i].hitbox.height=HEIGT_INVADER;
			 invaders[i].hitbox.width=WIDTH_INVADER;

			 invaders[i].hitbox.x = (SPACE_BETWEEN_INVADER)+(i*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
			 invaders[i].hitbox.y = 30;
	 }

	 for (i=0;i<3;i++){
			 invaders[i].hp=HP_INVADER;
			 invaders[i].hitbox.height=HEIGT_INVADER;
			 invaders[i].hitbox.width=WIDTH_INVADER;

			 invaders[i].hitbox.x = (SPACE_BETWEEN_INVADER+(WIDTH_INVADER/2))+(i*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
			 invaders[i].hitbox.y = 60;
	 }


 }

 void invaders_move(){

	 int i;
	 hitbox_t dimension;

	 invaders_get_wave_box(&dimension);

	 if ((dimension.x + dimension.width) >= LCD_MAX_X)
		 moving_invader *= 1;
	 if (dimension.x <= 0)
		 moving_invader *= -1;

	 for (i=0;i<NB_INVADERS;i++){
		 	invaders[i].hitbox.x += moving_invader;
	 }


 }


 //return  hitboxes from wave
 void invaders_get_wave_box(hitbox_t *wave_hitbox){

	 int i;

	 coord_t top_left,top_right,bot_left,bot_right;
	 top_left.x=LCD_MAX_X;
	 top_left.y=0;
	 top_right.x=0;
	 top_right.y=0;
	 bot_left.x=LCD_MAX_X;
	 bot_left.y=LCD_MAX_Y;
	 bot_right.x=0;
	 bot_right.y=LCD_MAX_Y;




	 //Detection hitbox top
	 for (i=0;i<NB_INVADERS;i++){

		 if ((invaders[i].hitbox.x < top_left.x)&&(invaders[i].hp<=0)){
			 top_left.x=invaders[i].hitbox.x;
			 top_left.y=invaders[i].hitbox.y;
		 }

		 else if ((invaders[i].hitbox.x > top_right.x)&&(invaders[i].hp<=0)){
			 top_right.x=invaders[i].hitbox.x;
			 top_right.y=invaders[i].hitbox.y;
		 }

	 }

	 //Detection hitbox bot
	 for (i=NB_INVADERS;i>0;i--){

		 if ((invaders[i].hitbox.x < bot_left.x)&&(invaders[i].hp<=0)){
			 bot_left.x=invaders[i].hitbox.x;
			 bot_left.y=invaders[i].hitbox.y;
		 }

		 else if ((invaders[i].hitbox.x > bot_right.x)&&(invaders[i].hp<=0)){
			 bot_right.x=invaders[i].hitbox.x;
			 bot_right.y=invaders[i].hitbox.y;
		 }

	 }

	 wave_hitbox->x  = top_left.x;
	 wave_hitbox->y  = top_left.y;
	 wave_hitbox->x = top_right.x;
	 wave_hitbox->y = top_right.y;
	 wave_hitbox->x  = bot_left.x;
	 wave_hitbox->y  = bot_left.y;
	 wave_hitbox->x = bot_right.x;
	 wave_hitbox->y = bot_right.y;
}

int invaders_lock(){
	if(invaders_task_mutex_created){
		return rt_mutex_lock(&invaders_task_mutex, TM_INFINITE);
	}
	return -1;
}

int invaders_unlock(){
	if(invaders_task_mutex_created){
		rt_mutex_unlock(&invaders_task_mutex);
		return 0;
	}
	return -1;
}

void invaders_refresh(void){
#include "vga_lookup.h"
	invader_t invader_loc[NB_INVADERS];
	int i;

	invaders_lock();
	memcpy(invader_loc, invaders, sizeof(invader_loc));
	invaders_unlock();

	for(i = 0; i < NB_INVADERS; i++){
		fb_rect_fill(invader_loc[i].hitbox.y,
					 invader_loc[i].hitbox.y + invader_loc[i].hitbox.height,
					 invader_loc[i].hitbox.x,
					 invader_loc[i].hitbox.x + invader_loc[i].hitbox.width,
					 LU_BLACK);
	}
}






