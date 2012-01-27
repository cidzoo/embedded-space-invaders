/*
 * fichier ship_task.c
 * \brief fichier (entetes) pour la gestion de la tache spaceship
 * \author Mohamed Regaya
 * \version 1.0
 * \date 12 janvier 2012
 *  
 */
 
#ifndef SHIP_TASK_H_
#define SHIP_TASK_H_
#include "global.h"

/*!
 * \struc spaceship_t 
 * \ type representant le vaisseau 
 * un vaisseau un nombre de point de vie hp, une zone de collision 
 * (hitbox) et une vitesse courant (current_speed_x)
 */
typedef struct{
	uint8_t hp;
	hitbox_t hitbox;
	int16_t current_speed_x; // -x : 0 : x
}spaceship_t;

// variable de type spaceship_t qui represente le vaisseau
extern spaceship_t ship;

// fonctions pour la gestion de la tache ship 
extern int ship_task_start(void);
extern void ship_task_cleanup_task(void);
extern void ship_task_cleanup_objects(void);
extern void ship_task_init(void);

// fonctions utlitaire pour le mutex de la tache ship
extern int ship_lock(void);
extern int ship_unlock(void);

#endif 
/* SHIP_TASK_H_ */
