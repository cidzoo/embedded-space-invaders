/*!
 * \file global.h
 * \brief Fichier (header) pour les define, variables et types persos globaux
 * \author Romain Maffina
 * \version 0.1
 * \date decembre 2012
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <linux/types.h>
#include <native/task.h>
#include <native/intr.h>
#include <native/event.h>
#include <native/alarm.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <native/heap.h>

#include "bitmaps.h"

//!Nombre d'invaders maximum
#define NB_INVADERS_MAX 24
//!Nombre de bullets maximum
#define NB_MAX_BULLETS 50
//!Nombre de bombes maximum
#define NB_MAX_BOMBS 20

//!Uses FPU, bound to CPU #0
#define TASK_MODE  T_FPU|T_CPU(0)
//!Stack size (in bytes)
#define TASK_STKSZ 8192

//!Priorité de la tâche framebuffer
#define TASK_FB_PRIO  70
//!Priorité de la tâche I/O
#define TASK_IO_PRIO  60
//!Priorité de la tâche de collisions
#define TASK_HIT_PRIO 20
//!Priorité de la tâche de gestion des invaders
#define TASK_INVADERS_PRIO	50
//!Priorité de la tâche de gestion du vaisseau
#define TASK_SHIP_PRIO 70

//!Coordonnée X min de la zone de jeu
#define GAME_ZONE_X_MIN		0
//!Coordonnée X max de la zone de jeu
#define GAME_ZONE_X_MAX		240
//!Coordonnée Y min de la zone de jeu
#define GAME_ZONE_Y_MIN		35
//!Coordonnée Y max de la zone de jeu
#define GAME_ZONE_Y_MAX		320

//!Vie de départ du vaisseau du joueur
#define LIFE_SHIP			3

//!Nombre d'armes au total
#define NB_WEAPONS			5

/*!
 * \enum difficulty_t
 * \brief Difficulté possibles
 */
typedef enum{EASY=1, NORMAL=2, HARD=3}difficulty_t;

/*!
 * \struct coord_t
 * \brief représente une coordonnée
 */
typedef struct{
	uint16_t x,y;
}coord_t;

/*!
 * \struct hitbox_t
 * \brief représente n'importe quel type d'objet physique dans le jeu
 */
typedef struct{
	uint16_t x,y;
	uint16_t width, height;
	uint16_t *bitmap; /*!< Pointeur sur le bitmap graphique */
}hitbox_t;


/*!
 * \enum weapontype_t
 * \brief représente les armes disponibles
 */
typedef enum{BOMB, GUN, RAIL, ROCKET, WAVE} weapontype_t;

/*!
 * \enum damage_t
 * \brief représente les dégats possibles
 */
typedef enum{ONE=1, TWO=2, THREE=3, MAX=10}damage_t;

/*!
 * \enum speed_t
 * \brief représente les vitesses de déplacement possibles
 */
typedef enum{STATIC=0, SLOW=1, MEDIUM=3, FAST=7, SUPERFAST = 10}speed_t;

/*!
 * \struct weapon_timing_charge_t
 * \brief permet de gérer la recharge des armes
 */
typedef struct{
	uint16_t max;			/*!< Valeur maximum de charge  */
	uint16_t now;			/*!< Valeur actuelle de charge */
	uint16_t last;			/*!< Dernière valeur lors du dernier tir en mode non automatique */
	uint16_t time_total;	/*!< Valeur courante pour le temps de recharge */
	uint16_t time_current;	/*!< Valeur maximum pour le temps de recharge */
} weapon_timing_charge_t;

/*!
 * \struct weapon_timing_led_t
 * \brief permet de gérer les leds et le clignotement
 */
typedef struct{
	uint16_t max;			/*!< Valeur maximum pour l'affichage */
	uint16_t ratio;			/*!< Coheficiant */
	uint16_t now;			/*!< Valeur courante pour le clignotement */
}weapon_timing_led_t;

/*!
 * \struct weapon_t
 * \brief représente une arme
 */
typedef struct{
	weapontype_t weapon_type;		/*!< Type de l'arme */
	uint8_t damage;					/*!< Dommage provoqué par l'arme */
	speed_t speed;					/*!< Vitesse de tir de l'are */
	weapon_timing_charge_t timing_charge;
	weapon_timing_led_t timing_led;
}weapon_t;

/*!
 * \struct bullet_t
 * \brief représente une bullet
 */
typedef struct{
	weapon_t *weapon;				/*!< Arme */
	hitbox_t hitbox;				/*!< Zone d'action du projectile */
}bullet_t;

//!Variable globale au projet pour choisir la difficulté générale du jeu
extern difficulty_t difficulty;

//!Variable globale au projet pour indiquer un game over
extern uint8_t game_over;

//!Variable globale au projet pour stocker le score actuel du joueur
extern uint32_t game_points;
//!Variable globale au projet pour indiquer le nombre de balles qui ont touché
extern uint32_t game_bullet_kill;
//!Variable globale au projet pour indiquer le nombre de balles qui sont sorties de l'écran
extern uint32_t game_bullet_used;

//!Variable globale au projet pour indiquer une pause générale
extern uint8_t game_break;
//!Variable globale au projet pour indiquer que la partie a commencé
extern uint8_t game_started;
//!Variable globale au projet pour indiquer que la vague courante a été détruite entièrement
extern uint8_t game_level_up;

//!Variable globale au projet pour indiquer que que l'écran a été touché
extern uint8_t screen_pressed;
//!Variable globale au projet pour indiquer la coordonnée x touchée
extern uint16_t screen_x;
//!Variable globale au projet pour indiquer la coordonnée y touchée
extern uint16_t screen_y;

#endif /* __GLOBAL_H__ */
