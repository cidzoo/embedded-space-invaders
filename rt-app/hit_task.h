/*!
 * \file hit_task.h
 * \brief Fichier (header) pour la gestion des tirs et de la tâche de gestion des collisions
 * \author Romain Maffina
 * \version 0.1
 * \date decembre 2012
 *
 * Fichier (header) pour la gestion des tirs et de la tâche de gestion des collisions.
 */

#ifndef HIT_TASK_H_
#define HIT_TASK_H_

#include "global.h"

extern weapon_t weapons[5]; /*!< Tableau contenant les armes */
extern bullet_t bullets[NB_MAX_BULLETS]; /*!< Tableau contenant les bullets */
extern bullet_t bombs[NB_MAX_BOMBS]; /*!< Tableau contenant les bombes */

/*! \brief Fonction à appelé au début pour lancer la tâche de gestion des collisions
 * ainsi que la création d'éventuels objets.
 */
extern int hit_task_start(void);

/*! \brief Fonction à appeler en fin (ou en cas de problème de création) pour détruire
 *  la tâche de gestion des collisions.
 */
extern void hit_task_cleanup_task(void);

/*! \brief Fonction à appeler en fin (ou en cas de problème de création) pour détruire
 *  les objets de la tâche de gestion des collisions (en principe avant la fonction hit_task_cleanup_task()).
 */
extern void hit_task_cleanup_objects(void);

/*! \brief Fonction à appeler afin de vérouiller par un mutex l'accès aux bullets
 */
extern int hit_lock(void);

/*! \brief Fonction à appeler afin de dévérouiller par un mutex l'accès aux bullets
 */
extern int hit_unlock(void);

/*! \brief Fonction permettant de tirer avec l'arme choisie
 *  \param shooter de type hitbox_t qui représente le tireur
 *  \param w de type weapontype_t qui indique l'arme choisie
 */
extern void hit_task_fire_weapon(hitbox_t shooter, weapontype_t w);

/*! \brief Fonction permettant d'initialiser/vider toutes les bullets du jeu
 */
extern void hit_task_init(void);

#endif /* HIT_TASK_H_ */
