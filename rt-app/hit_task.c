/*!
 * \file hit_task.c
 * \brief Fichier (body) pour la gestion des tirs et de la tâche de gestion des collisions
 * \author Romain Maffina
 * \version 0.1
 * \date decembre 2012
 *
 * Fichier (body) pour la gestion des tirs et de la tâche de gestion des collisions.
 */
#include "hit_task.h"
#include "invaders_task.h"
#include "ship_task.h"

#include "lcdlib.h"
#include "vga_lookup.h"
#include "rt-app-m.h"

/**
 * Variables publiques
 */
weapon_t weapons[NB_WEAPONS] = {
	{BOMB, ONE, SLOW,
		{0, 0, 0, 0, 0},
		{0, 0, 0},
	},
	{GUN, ONE, FAST,
		{15, 15, 0, 4, 0},
		{10, 0, 0},
	},
	{RAIL, THREE, STATIC,
		{1, 0, 0, 100, 0},
		{10, 0, 0},
	},
	{ROCKET, MAX, MEDIUM,
		{1, 0, 0, 100, 0},
		{10, 0, 0},
	},
	{WAVE, MAX, SUPERFAST,
		{1, 0, 0, 250, 0},
		{10, 0, 0},
	}
};

bullet_t bullets[NB_MAX_BULLETS];
bullet_t bombs[NB_MAX_BOMBS];

/**
 * Variables privées
 */
RT_MUTEX hit_task_mutex;
static uint8_t hit_task_mutex_created = 0;

RT_TASK hit_task_handle;
static uint8_t hit_task_created = 0;

//for rail use
static int rail_id = -1;
static int rail_timeout = 0;


/**
 * Prototypes fonctions privées
 */

/*! \brief Fonction permettant de tester la collision entre 2 objets du jeu
 *  \param a première hitbox_t
 *  \param b deuxième hitbox_t
 *  \return un entier qui indique s'il y a eu collision
 *  \retval 0 il y eu collision
 *  \retval -1 il n'y a pas eu de collisions
 */
static int hit_test(hitbox_t a, hitbox_t b);

/*! \brief Fonction permettant d'ajouter une bullet au jeu en cours
 *  \param b de type bullet_t qui la bullet à ajouter
 *  \return un entier qui indique si l'opération s'est bien passée ou non
 *  \retval 0 OK
 *  \retval -1 KO
 */
static int add_bullet(bullet_t b);

/*! \brief Fonction permettant de supprimer une bullet au jeu en cours
 *  \param b de type bullet_t qui la bullet à supprimer
 *  \param id de type int qui indique l'index de la bullet à supprimer
 */
static void remove_bullet(bullet_t b, int id);

/*! \brief Handler utilisé et représentant la tâche de gestion des collisions
 *  \param cookie Paramètre factultatif et non utilisé (besion de Xenomai)
 *
 *  Cette fonction représente le corps proprement dit de la tâche de gestion
 *  des collisions. Elle se charge de faire le déplacement des bullets et de
 *  tester si il y a eu des collisions entre les objets (représentés par des
 *  objets de type hitbox_t) présents dans la partie.
 *  Cette fonction ne doit pas être appelée directement mais seulement lors de
 *  l'init (fb_task_start()) à l'aide de rt_task_create().
 */
static void hit_task(void *cookie);

/* Task start */
int hit_task_start(){
	int err;

	err = rt_mutex_create(&hit_task_mutex, "task_hit_mutex");
	if(err == 0){
		hit_task_mutex_created = 1;
		printk("rt-app: Task HIT create mutex succeed\n");
	}else{
		printk("rt-app: Task HIT create mutex failed\n");
		goto fail;
	}

	err = rt_task_create(&hit_task_handle,
	                     "task_hit",
	                      TASK_STKSZ,
	                      TASK_HIT_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&hit_task_handle, &hit_task, NULL);
		hit_task_created = 1;
		if(err != 0){
			printk("rt-app: Task HIT starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task HIT starting succeed\n");
		}
	}else{
		printk("rt-app: Task HIT creation failed\n");
		goto fail;
	}
	return 0;
fail:
	hit_task_cleanup_task();
	hit_task_cleanup_objects();
	return -1;
}

/* Task cleanup */
void hit_task_cleanup_task(){
	if(hit_task_created){
		hit_task_created = 0;
		rt_task_delete(&hit_task_handle);
	}
}

/* Task objects cleanup */
void hit_task_cleanup_objects(){
	if(hit_task_mutex_created){
		hit_task_mutex_created = 0;
		rt_mutex_delete(&hit_task_mutex);
	}
}

void hit_task_init(){
	int i;
	for(i = 0; i < NB_WEAPONS; i++){
		if(weapons[i].weapon_type == GUN)
			weapons[i].timing_charge.now = weapons[i].timing_charge.max;
		else
			weapons[i].timing_charge.now = 0;

		weapons[i].timing_charge.last = 0;
		weapons[i].timing_charge.time_current = 0;

		weapons[i].timing_led.now = 0;
	}

	for(i = 0; i < NB_MAX_BULLETS; i++){
		bullets[i].weapon = NULL;
	}
	for(i = 0; i < NB_MAX_BOMBS; i++){
		bombs[i].weapon = NULL;
	}
}

//!*****************HIT TASK***********************/
void hit_task(void *cookie){

	invader_t *invader;
	bullet_t *bullet;
	int i, j;
	int16_t y;
	uint8_t removed; //TODO remove this and try
	uint8_t impact = 0; //!flag de collision pour la tache hit

	(void)cookie;
	//! On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 50*MS);

	//! init
	hit_task_init();


	for (;;) {
		rt_task_wait_period(NULL);

		if(!game_break){
			//! On verrouille les bullets
			hit_lock();
			//! On verrouille le vaisseau
			ship_lock();
			//! On verrouille les invaders
			invaders_lock();

			//pour chaque bullet on va tester les collisions
			for (i=0;i<NB_MAX_BULLETS;i++){
				removed = 0;
				if(bullets[i].weapon != NULL){
					impact = 0;

					//!current object
					bullet = &bullets[i];

					// On déplace la bullet
					bullet->hitbox.y -= bullet->weapon->speed;
					y = bullet->hitbox.y;

					//suppression des bullets en haut de l'écran
					if(y <= 0){
						if(bullet->weapon->weapon_type != RAIL){
							// Gestion des points lors de la sortie d'un bullet
							if(bullet->weapon->weapon_type != WAVE && game_points >= 1){
								//game_points -= 1;
							}
							if(!removed){
								remove_bullet(*bullet, i);
								removed = 1;
							}
							// On met a jour les spef concernant la precision de tir
							game_bullet_used++;
							continue;
						}
					}

					//bullet : hit test avec les invaders
					for (j=0;j<wave.invaders_count;j++){
						if(wave.invaders[j].hp > 0){

							//!current object
							invader = &wave.invaders[j];

							//hit test
							if(hit_test(invader->hitbox, bullet->hitbox) == 0){
								impact = 1;

								//applique les dégats à l'invader
								if(invader->hp >= bullet->weapon->damage){
									// On met a jour les points de vie de l'invader
									invader->hp-= bullet->weapon->damage;
									// On met a jour les points
									game_points += 1;
								}
								else{
									// On met a jour les points de vie de l'invader
									invader->hp = 0;
									// On met a jour les points
									game_points += 10;
								}
								// On met a jour les spef concernant la precision de tir
								game_bullet_used++;
								game_bullet_kill++;	// La bullet à touché sa cible
							}//if hit test
						}
					}//pour chaque invader

					//bullet : hit test avec les bombes
					for(j=0;j<NB_MAX_BOMBS;j++){
						if(bombs[j].weapon != NULL){
							//hit test
							if(hit_test(bombs[j].hitbox, bullet->hitbox) == 0){
								impact = 1;
								//détruit la bombe
								remove_bullet(bombs[j], j);
							}
						}
					}

					//bullet : hit test avec les autres bullets
					for(j=0;j<NB_MAX_BULLETS;j++){
						if( 	(bullets[j].weapon != NULL) &&
								(&bullets[j] != bullet) && //Si différent de la bullet actuelle
								(bullets[j].weapon->weapon_type != RAIL) && //Si ce n'est pas le laser
								(bullets[j].weapon->weapon_type != WAVE) //Si ce n'est pas l'onde de choc
								){
							//hit test
							if(hit_test(bullets[j].hitbox, bullet->hitbox) == 0){
								impact = 1;
								//détruit la bullet
								if(!removed){
									remove_bullet(bullets[j], j);
									removed = 1;
								}
							}
						}
					}

					//si un impact a été detecté pour la bullet principale, on la supprime
					if(		impact &&
							!(bullet->weapon->weapon_type == WAVE) &&
							!(bullet->weapon->weapon_type == RAIL) ){
						if(!removed){
							remove_bullet(*bullet, i);
							removed = 1;
						}
					}

				}//if non null
			}//pour chaque bullet pirncipale

			//pour chaque bombe on va tester les collisions
			for(i=0;i<NB_MAX_BOMBS;i++){
				if(bombs[i].weapon != NULL){

					// On déplace les bombes
					bombs[i].hitbox.y += bombs[i].weapon->speed;

					//hit test avec le vaisseau
					if(hit_test(ship.hitbox, bombs[i].hitbox) == 0){
						//applique les dommages au vaisseau et supprime la bombe
						if(ship.hp > 0){
							ship.hp--;
							if(ship.hp==0){
								game_over = 1;
							}
						}
						remove_bullet(bombs[i], i);
					}
				}
			}

			//invaders : hit test avec le vaisseau
			for (i=0;i<wave.invaders_count;i++){
				if(wave.invaders[i].hp > 0 &&
				   hit_test(ship.hitbox, wave.invaders[i].hitbox) == 0){
					ship.hp = 0;
					game_over = 1;
				}
			}

			//traitement spécial pour le rail (animation)
			if(rail_id != -1 && rail_timeout <= 0){
				remove_bullet(bullets[rail_id],rail_id);
				rail_id = -1;
			}else
				rail_timeout--;

			// On deverrouille les invaders
			invaders_unlock();
			// On deverrouille le vaisseau
			ship_unlock();
			// On deverrouille les bullets
			hit_unlock();
		}else{

		}
	}

}//hit_task


void hit_task_fire_weapon(hitbox_t shooter, weapontype_t w){
	uint16_t start_x=0, start_y=0;
	bullet_t b;

	//Définit la position de départ selon l'emplacement du tireur
	if(w == BOMB){
		//Position en bas au milieu de l'invader qui tire
		start_x = shooter.x + shooter.width/2;
		start_y = shooter.y+shooter.height+1;
	}
	else{
		//Position en haut au milieu du vaisseau
		start_x = shooter.x + shooter.width/2;
		start_y = shooter.y-1;
	}

	//Tir de la bullet
	switch(w){
	case BOMB:
		b.weapon = &weapons[BOMB];
		b.hitbox.x = start_x-BOMB_WIDTH/2;
		b.hitbox.y = start_y;
		b.hitbox.width = BOMB_WIDTH;
		b.hitbox.height = BOMB_HEIGHT;
		b.hitbox.bitmap = (uint16_t *)bmp_bomb;
		break;
	case GUN:
		b.weapon = &weapons[GUN];
		b.hitbox.x = start_x-GUN_WIDTH/2;
		b.hitbox.y = start_y-GUN_HEIGHT;
		b.hitbox.width = GUN_WIDTH;
		b.hitbox.height = GUN_HEIGHT;
		b.hitbox.bitmap = (uint16_t *)bmp_gun;
		break;
	case RAIL:
		b.weapon = &weapons[RAIL];
		b.hitbox.x = start_x-RAIL_WIDTH/2;
		b.hitbox.y = GAME_ZONE_Y_MIN;
		b.hitbox.width = RAIL_WIDTH;
		b.hitbox.height = start_y - GAME_ZONE_Y_MIN;
		b.hitbox.bitmap = (uint16_t *)bmp_rail;
		break;
	case ROCKET:
		b.weapon = &weapons[ROCKET];
		b.hitbox.x = start_x-ROCKET_WIDTH/2;
		b.hitbox.y = start_y-ROCKET_HEIGHT;
		b.hitbox.width = ROCKET_WIDTH;
		b.hitbox.height = ROCKET_HEIGHT;
		b.hitbox.bitmap = (uint16_t *)bmp_rocket;
		break;
	case WAVE:
		b.weapon = &weapons[WAVE];
		b.hitbox.x = 0;
		b.hitbox.y = start_y-WAVE_HEIGHT;
		b.hitbox.width = WAVE_WIDTH;
		b.hitbox.height = WAVE_HEIGHT;
		b.hitbox.bitmap = (uint16_t *)bmp_wave;
		break;
	}

	//ajoute la bullet à la liste des bullets en jeu
	add_bullet(b);

}//fire_weapon()

static int hit_test(hitbox_t a, hitbox_t b){

	if (	(a.x + a.width >= b.x) &&
			(a.x <= b.x + b.width) &&
			(a.y <= b.y + b.height) &&
			(a.y + a.height >= b.y) ) {
		return 0;
	}
	return -1;

}//hit_test()

static int add_bullet(bullet_t b){
	int i=0;

	//bullet normale
	if(b.weapon->weapon_type != BOMB){
		//trouve le premier slot libre et y mettre la bullet
		for(i=0;i<NB_MAX_BULLETS;i++){
			if(bullets[i].weapon == NULL){
				bullets[i] = b;
				if(bullets[i].weapon->weapon_type == RAIL){
					rail_id = i;
					rail_timeout = 10;
				}
				return 0;
			}
		}
	//pour les bombes
	}else{
		//trouve le premier slot libre et y mettre la bullet
		for(i=0;i<NB_MAX_BOMBS;i++){
			if(bombs[i].weapon == NULL){
				bombs[i] = b;
				return 0;
			}
		}
	}
	return -1;
}

static void remove_bullet(bullet_t b, int id){
	if(b.weapon != NULL){
		if(b.weapon->weapon_type != BOMB)
			bullets[id].weapon = NULL;
		else
			bombs[id].weapon = NULL;
	}
}

int hit_lock(){
	if(hit_task_mutex_created){
		return rt_mutex_lock(&hit_task_mutex, TM_INFINITE);
	}
	return -1;
}

int hit_unlock(){
	if(hit_task_mutex_created){
		return rt_mutex_unlock(&hit_task_mutex);
	}
	return -1;
}
