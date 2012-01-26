/*!
 * \file fb_task.h
 * \brief Fichier (header) pour la gestion de la tâche de gestion du frame buffer
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (header) pour la tâche de gestion du frame buffer.
 * A chaque refraichissement de l'écran (définit par la période de cette tâche),
 * récupère les données des invaders, du vaisseau, des bombes et des bullets
 * puis les affiche à l'écran.
 * Déclare les fonctions public que le tâche fb fournit. Elle ne fournit aucune
 * donnée ni setter/getter, seulement les fonctions pour sa création ainsi que
 * sa destruction.
 */
#ifndef TASK_FB_H_
#define TASK_FB_H_

#include "global.h"

/*! \brief Fonction à appelé au début pour lancer la tâche de gestion du frame buffer
 * ainsi que la création d'éventuels objets.
 */
extern int fb_task_start(void);

/*! \brief Fonction à appelé en fin (ou en cas de problème de création) pour détruire
 *  la tâche de gestion du frame buffer (en principe avant la fonction fb_task_cleanup_objects()).
 */
extern void fb_task_cleanup_task(void);

/*! \brief Fonction à appelé en fin (ou en cas de problème de création) pour détruire
 *  les objets de la tâche de gestion du frame buffer (en principe avant la fonction fb_task_cleanup_objects()).
 *  Elle ne fait en principe rien puisqu'il n'y pas d'objet pour le moment dans la tâche fb.
 */
extern void fb_task_cleanup_objects(void);

#endif /* TASK_FB_H_ */
