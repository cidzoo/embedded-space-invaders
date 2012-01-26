/*!
 * \file invaders_task.h
 * \brief Fichier (header) pour la gestion de la tâche des invaders
 * \author Michael Favaretto
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (header) pour la tâche de gestion des invaders.
 * Initialise les invaders et gère leur déplacement dans
 * la zone de jeux. Gère la gestion des tirs des bombes des invaders
 * Permet de déterminer la fin d'un niveau lorsque tous les invaders
 * sont morts.
 *
 */

#ifndef INVADERS_H_
#define INVADERS_H_

#include "global.h"

//! Const pour la largeur d'un invader
#define WIDTH_INVADER 	16
//! Const pour la hauteur d'un invader
#define HEIGT_INVADER	16
//! Const pour le nombre de points de vie d'un invader
#define HP_INVADER		10
//! Const pour l'espacement entre chaque invader
#define SPACE_BETWEEN_INVADER	20
//! Const pour la marge entre la zone de jeux et un invader
#define MARGE 10

/*!
 * \struct invader_t
 * \brief Objet représentant un invader
 *
 * invader_t représente un invader. Il contient son nombre de
 * point de vie et sa position sur l'écran (x, y, hauteur, largeur)
 */
typedef struct invader_t {
	uint8_t hp;
	hitbox_t hitbox;
} invader_t;

/*!
 * \struct wave_t
 * \brief Objet représentant une wave
 *
 * wave_t représente une wave. Il contient le niveau de la wave,
 * la vitesse de déplacement verticale des invaders, le tableau contenant les invaders
 * et le nombre d'invader pour la wave.
 */
typedef struct {
	uint8_t level;
	uint16_t invader_speed;
	invader_t invaders[NB_INVADERS_MAX];
	uint8_t invaders_count;
} wave_t;

//! Var de type wave_t pour la vague courante des invaders
extern wave_t wave;

/*! \brief Fonction à appelé au début pour lancer la tâche de gestion des invaders
 */
extern int invaders_task_start(void);

/*! \brief Fonction à appelé en fin (ou en cas de problème de création) pour détruire
 *  la tâche de gestion des invaders (en principe avant la fonction invaders_task_cleanup_objects()).
 */
extern void invaders_task_cleanup_task(void);

/*! \brief Fonction à appelé en fin (ou en cas de problème de création) pour détruire
 *  les objets de la tâche de gestion des invaders (en principe avant la fonction invaders_task_cleanup_objects()).
 */
extern void invaders_task_cleanup_objects(void);


/*! \brief Fonction à appelé au début de chaque partie pour initialiser la vague et les invaders au premier niveau
 */
extern void invaders_task_init(void);

/*! \brief Fonction à appelé avant d'accéder aux invaders qui va essayé de prendre le mutex
 */
extern int invaders_lock(void);

/*! \brief Fonction pour libérer le mutex des invaders
 */
extern int invaders_unlock(void);

/*! \brief Fonction à appeler pour passer au niveau suivant
 */
extern void level_up(void);

#endif /*ENEMIES_H_*/
