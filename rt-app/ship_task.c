/*
 * ship_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include <linux/types.h>
#include "ship_task.h"
#include "vga_lookup.h"
#include "xeno-ts.h"
#include "lcdlib.h"


/**
 * Variables public
 */
spaceship_t ship;

/**
 * Variables privées
 */
static RT_MUTEX ship_task_mutex;
static uint8_t ship_task_mutex_created = 0;

static RT_TASK ship_task_handle;
static uint8_t ship_task_created = 0;

/**
 * Fonctions privées
 */
static void ship_task(void *cookie);
static void ship_init(void);
static void ship_move(int x, int y);
static void ship_display (void);
static int ship_comp_to_zone_vess(void);


int ship_task_start(){
	int err;

		err = rt_mutex_create(&ship_task_mutex, "task_ship_mutex");
		if(err == 0){
			ship_task_mutex_created = 1;
			printk("rt-app: Task SHIP create mutex succeed\n");
		}else{
			printk("rt-app: Task SHIP create mutex failed\n");
			goto fail;
		}
	err = rt_task_create(&ship_task_handle,
	                     "task_ship",
	                      TASK_STKSZ,
	                      TASK_SHIP_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&ship_task_handle, &ship_task, NULL);
		ship_task_created = 1;
		if(err != 0){
			printk("rt-app: Task SHIP starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task SHIP starting succeed\n");
		}
	}else{
		printk("rt-app: Task SHIP creation failed\n");
		goto fail;
	}
	return 0;
fail:
	ship_task_cleanup();
	return -1;
}

void ship_task_cleanup(){
	if(ship_task_created){
		ship_task_created = 0;
		rt_task_delete(&ship_task_handle);
	}
	if(ship_task_mutex_created){
		ship_task_mutex_created = 0;
		rt_mutex_delete(&ship_task_mutex);
	}
}

static void ship_task(void *cookie){

	rt_task_set_periodic(NULL, TM_NOW, 10000000);

	ship_init();

	for (;;) {
		rt_task_wait_period(NULL);
	}
}

/**
 * init the vessel's params
 */
static void ship_init(){
	ship_lock();
	ship.hitbox.height=20;
	ship.hitbox.width=15;
	ship.hitbox.x= 105;
	ship.hitbox.y= 300;
	ship_unlock();
}

static void ship_move(int x, int y){

	fb_rect_fill(ship.hitbox.y, ship.hitbox.y + ship.hitbox.height,
			ship.hitbox.x, ship.hitbox.x+ship.hitbox.height, LU_BLACK);

	ship.hitbox.x= x;
	ship.hitbox.y= y;
}

static void ship_display (){
	int i= 0;
	// first we draw a pyramid
	int pos_pix_bef = 0;
	int pos_pix_aft = 0;
	int half_widht = ship.hitbox.width/2 + 1;
	int j;

	// initialize the frame buffer
	fb_init ();

	// display the vessel

	for (;i<ship.hitbox.height-10; i++){

		if (i==0){
			pos_pix_bef= half_widht -1; // one pixel before half
			pos_pix_aft= half_widht +1; // one pixel after half
		}


		// pixel to draw per line
		for (j=pos_pix_bef; j<pos_pix_aft +1; j++){
			fb_set_pixel(ship.hitbox.x+j, ship.hitbox.y+i, LU_BLUE);
		}
		// update the limit position of the pixels to draw
		pos_pix_bef--;
		pos_pix_aft++;
	}

	// draw a rectangle
	fb_rect_fill(ship.hitbox.y +i, ship.hitbox.y + ship.hitbox.height,
			ship.hitbox.x, ship.hitbox.x+ship.hitbox.height, LU_BLUE);
}

/**
 * return true whether the zone pressed is in the vessel area zone
 */
static int ship_comp_to_zone_vess(){
	return 1;//(y >= VESS_Y_MIN) && (y < VESS_X_MAX);
}

int ship_lock(){
	if(ship_task_mutex_created){
		return rt_mutex_lock(&ship_task_mutex, TM_INFINITE);
	}
	return -1;
}

int ship_unlock(){
	if(ship_task_mutex_created){
		rt_mutex_unlock(&ship_task_mutex);
		return 0;
	}
	return -1;
}
