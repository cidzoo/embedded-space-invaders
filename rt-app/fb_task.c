/*
 * task_fb.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include "fb_task.h"
#include "vga_lookup.h"
#include "lcdlib.h"

#include "invaders_task.h"
#include "ship_task.h"
#include "hit_task.h"

/**
 * Variables privées
 */
RT_TASK fb_task_handle;
static uint8_t fb_task_created = 0;

#define FC	LU_WHITE
#define BC	LU_BLACK

void draw_invader(unsigned int x, unsigned int y);

static unsigned short int invader_bmp[16][16] = {
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
	{FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC, FC},
	{BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC, BC},
};


/**
 * Fonctions privées
 */
static void fb_task(void *cookie);

int fb_task_start(){
	int err;
	err = rt_task_create(&fb_task_handle,
	                     "task_fb",
	                      TASK_STKSZ,
	                      TASK_FB_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&fb_task_handle, &fb_task, NULL);
		fb_task_created = 1;
		if(err != 0){
			printk("rt-app: Task FB starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task FB starting succeed\n");
		}
	}else{
		printk("rt-app: Task FB creation failed\n");
		goto fail;
	}
	return 0;
fail:
	fb_task_cleanup_task();
	fb_task_cleanup_objects();
	return -1;
}

void fb_task_cleanup_task(){
	if(fb_task_created){
		printk("rt-app: Task FB cleanup task\n");
		fb_task_created = 0;
		rt_task_delete(&fb_task_handle);
	}
}

void fb_task_cleanup_objects(){
}

static void fb_task(void *cookie){
	int  i;
	invader_t invader_loc[NB_INVADERS];
	spaceship_t ship_loc;

	rt_task_set_periodic(NULL, TM_NOW, 40000000);
<<<<<<< HEAD
	fb_rect_fill(0, 319, 0, 239, LU_BRT_BLUE);

	invader_loc[0].hp = 3;
=======
	//fb_rect_fill(0, 319, 0, 239, LU_BRT_BLUE);
/*
>>>>>>> f5aff8c1e90801f1c0f4c50679819744e4fa1183
	invader_loc[0].hitbox.x = 20;
	invader_loc[0].hitbox.y = 20;
	invader_loc[0].hitbox.width = 20;
	invader_loc[0].hitbox.height = 20;

	invader_loc[1].hp = 3;
	invader_loc[1].hitbox.x = 50;
	invader_loc[1].hitbox.y = 20;
	invader_loc[1].hitbox.width = 20;
	invader_loc[1].hitbox.height = 20;
*/


	for (;;) {
		rt_task_wait_period(NULL);

		//invaders_refresh();
<<<<<<< HEAD
		hit_refresh();
=======

		//hit_refresh();
>>>>>>> f5aff8c1e90801f1c0f4c50679819744e4fa1183
		//ship_refresh();

		invaders_lock();
		//memcpy(invader_loc, invaders, sizeof(invader_loc));
		invaders_unlock();

		ship_lock();
		memcpy(&ship_loc, &ship, sizeof(ship_loc));
		ship_unlock();

		fb_rect_fill(0, 319, 0, 239, LU_BRT_BLUE);

<<<<<<< HEAD
		for(i = 0; i < 2; i++){
			if(invader_loc[i].hp > 0){
				fb_rect_fill(invader_loc[i].hitbox.y,
							 invader_loc[i].hitbox.y + invader_loc[i].hitbox.height,
							 invader_loc[i].hitbox.x,
							 invader_loc[i].hitbox.x + invader_loc[i].hitbox.width,
							 LU_BLACK);
			}
=======
		// On dessine les invaders
		for(i = 0; i < NB_INVADERS; i++){
			/*fb_rect_fill(invader_loc[i].hitbox.y,
						 invader_loc[i].hitbox.y + invader_loc[i].hitbox.height,
						 invader_loc[i].hitbox.x,
						 invader_loc[i].hitbox.x + invader_loc[i].hitbox.width,
						 LU_BLACK);*/
			if(invader_loc[i].hp > 0){
				draw_invader(invader_loc[i].hitbox.y, invader_loc[i].hitbox.x);
			}
		}

		// On dessine le vaisseau
		fb_rect_fill(ship_loc.hitbox.y,
					 ship_loc.hitbox.y + ship_loc.hitbox.height,
					 ship_loc.hitbox.x,
					 ship_loc.hitbox.x + ship_loc.hitbox.width,
					 LU_BRT_YELLOW);

		rt_task_set_priority(NULL, 90);
		fb_display();
		rt_task_set_priority(NULL, 50);
	}
}

void draw_invader(unsigned int y, unsigned int x){
	int i, j;
	for(i = 0; i < 16; i++){
		for(j = 0; j < 16; j++){
			fb_set_pixel(y + i, x + j, invader_bmp[i][j]);
>>>>>>> f5aff8c1e90801f1c0f4c50679819744e4fa1183
		}
	}
}

