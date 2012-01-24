/*
 * enemies.c
 *
 *  Created on: Dec 22, 2011
 *      Author: michael
 */
#include <linux/types.h>
#include "lcdlib.h"
#include "invaders_task.h"
#include "hit_task.h"

/**
 * Variables public
 */
wave_t wave;


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
int invaders_random(int a, int b);


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

	// On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 100000000);
	invaders_lock();
	wave.invader_speed = 1;
	wave.level = 0;
	wave.invaders_count = 15;
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
	 int nb_invaders_per_line[NB_INVADERS_MAX]={0};
	 int line=0;
	 int invader_id=0;
	 int i,j;

     //Détermine le nombre d'ennemi par ligne
     for(i=0;i<wave.invaders_count;i++){

         //Initialise les valeurs chaque ennemi
         wave.invaders[i].hp=10;
         wave.invaders[i].hitbox.height=HEIGT_INVADER;
         wave.invaders[i].hitbox.width=WIDTH_INVADER;
         wave.invaders[i].hitbox.type=G_INVADER;

         nb_invaders_per_line[line]++;

         //Détermine si la ligne est complète
         if((GAME_ZONE_X_MAX - GAME_ZONE_X_MIN -MARGE - WIDTH_INVADER - (nb_invaders_per_line[line]*(SPACE_BETWEEN_INVADER+WIDTH_INVADER))) < 0)
             line++;


     }



     //Placement des invaders
     for (i=0; i<=line;i++){

         for(j=0;j<nb_invaders_per_line[i];j++){

        	 //Si dernière ligne invaders -> décallage
        	 if(i==line){
				 wave.invaders[invader_id].hitbox.x = (GAME_ZONE_X_MIN+MARGE+((WIDTH_INVADER)*(nb_invaders_per_line[0]-nb_invaders_per_line[i])))+(j*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
				 wave.invaders[invader_id].hitbox.y = (GAME_ZONE_Y_MIN+SPACE_BETWEEN_INVADER)+(i*(SPACE_BETWEEN_INVADER+HEIGT_INVADER));
        	 }
        	 else{
        		 wave.invaders[invader_id].hitbox.x = (GAME_ZONE_X_MIN+MARGE)+(j*(SPACE_BETWEEN_INVADER+WIDTH_INVADER));
        		 wave.invaders[invader_id].hitbox.y = (GAME_ZONE_Y_MIN+SPACE_BETWEEN_INVADER)+(i*(SPACE_BETWEEN_INVADER+HEIGT_INVADER));
        	 }
             invader_id++;
         }



     }
 }

 void invaders_move(){
	 int i;
	 int x = 0;
	 static int move_up=0;
	 static int moving_right = 1;
	 hitbox_t dimension;
	 int invader_dead=0;



	 //Check la dimmension de la wave
	 invaders_get_wave_box(&dimension);

	 //Déplacement à droite
	 if (moving_right){
		 if ((dimension.x + dimension.width + wave.invader_speed) < GAME_ZONE_X_MAX){
			 x = wave.invader_speed;
		 }else{
			 x = LCD_MAX_X - (dimension.x + dimension.width);
			 moving_right = 0;
			 move_up=1;
		 }
	 }
	 //Déplacement à gauche
	 else{
		 if (((int32_t)dimension.x - (int32_t)wave.invader_speed) > GAME_ZONE_X_MIN){
			 x = -wave.invader_speed;
		 }else{
			 x = -dimension.x;
			 moving_right = 1;
			 move_up=1;
		 }
	 }

	 for (i=0; i<wave.invaders_count;i++){
		 wave.invaders[i].hitbox.x += x;

		 //Avance les invaders en avant
		 if(move_up)
			 wave.invaders[i].hitbox.y+=1;

		 //check si un invader atteint la terre
		 if(wave.invaders[i].hitbox.y+wave.invaders[i].hitbox.height == GAME_ZONE_Y_MAX)
			 game_over=1;

		 //Count the number of invader dead
		 if (wave.invaders[i].hp<=0)
			 invader_dead++;
	 }

	 if(move_up)
		 move_up=0;

	 // test if level finish
	 if (invader_dead == wave.invaders_count)
		 level_finish = 1;

/*
	 for (i=0;i<2+wave.level;i++){
		 fire_weapon(wave.invaders[invaders_random(wave.invaders_count)].hitbox,BOMB);
	 }
*/

	 fire_weapon(wave.invaders[wave.invaders_count].hitbox,BOMB);


 }



 //return  hitboxes from wave
 void invaders_get_wave_box(hitbox_t *wave_hitbox){

	 int i;

	 wave_hitbox->x = LCD_MAX_X;
	 wave_hitbox->y = LCD_MAX_Y;
	 wave_hitbox->width = 0;
	 wave_hitbox->height = 0;

	 //Detection hitbox top
	 for (i=0;i<wave.invaders_count;i++){
		 if(wave.invaders[i].hp > 0){
			 //Detection x
			 if (wave.invaders[i].hitbox.x < wave_hitbox->x){
				 wave_hitbox->x = wave.invaders[i].hitbox.x;
			 }
			 //Detection y
			 if (wave.invaders[i].hitbox.y < wave_hitbox->y){
				 wave_hitbox->y = wave.invaders[i].hitbox.y+1;
			 }

			 //Detection width
			 if (wave.invaders[i].hitbox.x+wave.invaders[i].hitbox.width >
			 	 wave_hitbox->x + wave_hitbox->width){
				 wave_hitbox->width = (wave.invaders[i].hitbox.x+wave.invaders[i].hitbox.width)-wave_hitbox->x-1;
			 }
			 //Detection height
			 if (wave.invaders[i].hitbox.y+wave.invaders[i].hitbox.height >
				 wave_hitbox->y + wave_hitbox->height){
				 wave_hitbox->height = (wave.invaders[i].hitbox.y+wave.invaders[i].hitbox.height)-wave_hitbox->y;
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

//To call each time the current invaders wave is finished to init a new one
void level_up(){
	//static uint8_t lvl = 0;

	//current_wave->level = ++lvl;
	wave.level++;

	if(wave.invaders_count<NB_INVADERS_MAX)
		wave.invaders_count++;
	wave.invader_speed = difficulty;

	//init_invaders(current_wave->invaders);
}

int invaders_random(int range){
	return get_random()%range;
}




