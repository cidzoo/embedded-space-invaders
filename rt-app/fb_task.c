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
#include "rt-app-m.h"

/**
 * Variables privées
 */
RT_TASK fb_task_handle;
static uint8_t fb_task_created = 0;


/**
 * Fonctions privées
 */
static void draw_bitmap(hitbox_t hb);
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

	rt_task_set_periodic(NULL, TM_NOW, 50*MS);

	for (;;) {
		rt_task_wait_period(NULL);

		invaders_lock();
		memcpy(invader_loc, invaders, sizeof(invader_loc));
		invaders_unlock();

		ship_lock();
		memcpy(&ship_loc, &ship, sizeof(ship_loc));
		ship_unlock();

		// On dessine le background
		fb_rect_fill(0, 319, 0, 239, LU_BLACK);

		// On dessine les invaders
		for(i = 0; i < NB_INVADERS; i++){
			if(invader_loc[i].hp > 0){
				draw_bitmap(invader_loc[i].hitbox);
			}
		}

		// On dessine le vaisseau
		draw_bitmap(ship_loc.hitbox);


		// On dessine les bullets
		for(i = 0; i < NB_MAX_BULLETS; i++){
			if (bullets[i].weapon != NULL){
				if( (bullets[i].weapon->weapon_type != RAIL) &&
					(bullets[i].hitbox.y-1 == 0) ){
					remove_bullet(i);
				}
				bullets[i].hitbox.y -= bullets[i].weapon->speed;

				draw_bitmap(bullets[i].hitbox);
			}
		}


		rt_task_set_priority(NULL, 90);
		fb_display();
		rt_task_set_priority(NULL, 50);
	}
}

static void draw_bitmap(hitbox_t hb){
	int i, j;
	//printk("%d %d %d %d\n", hb.x, hb.y, hb.width, hb.height);
	for(i = 0; i < hb.height; i++){
		for(j = 0; j < hb.width; j++){
			//printk("%p %p\n", bmp, bmp_ship);
			//printk("%d\n", *((*(bmp+i))+j));
			switch(hb.type){
			case G_SHIP:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_ship+i))+j)));
				break;
			case G_INVADER:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_invader+i))+j)));
				break;
			case G_BOMB:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_bomb+i))+j)));
				break;
			case G_GUN:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_gun+i))+j)));
				break;
			case G_RAIL:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_rail+i))+j)));
				break;
			case G_ROCKET:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_rocket+i))+j)));
				break;
			case G_WAVE:
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_wave+i))+j)));
				break;
			}

			//fb_set_pixel(hb.y + hb.height-i, hb.x + j, bmp[i][j]);
		}
	}
}

