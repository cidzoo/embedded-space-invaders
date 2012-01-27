/*!
 * \file io_task.h
 * \brief Fichier (header) pour la gestion de la tâche des entrées/sorties
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (header) pour la gestion de la tâche des entrées/sorties
 * Cette tâche gère les entrées/sorties (boutons et leds). Lorsqu'un bouton
 * est pressé, c'est cette tâche qui signale à la git task qu'elle doit ajouter
 * des bullets via la foncction hit_task_fire_weapon().
 */
#ifndef IO_TASK_H_
#define IO_TASK_H_

#include "global.h"

/*! \brief Fonction à appeler au début pour lancer la tâche de gestion des entrées/sorties
 * ainsi que la création d'éventuels objets.
 */
extern int io_task_start(void);

/*! \brief Fonction à appeler en fin (ou en cas de problème de création) pour détruire
 *  la tâche de gestion des entrées/sorties.
 */
extern void io_task_cleanup_task(void);

/*! \brief Fonction à appeler en fin (ou en cas de problème de création) pour détruire
 *  les objets de la tâche de gestion des entrées/sorties (en principe avant la fonction io_task_cleanup_objects()).
 *  Elle ne fait en principe rien puisqu'il n'y pas d'objet pour le moment dans la tâche io.
 */
extern void io_task_cleanup_objects(void);

#endif /* IO_TASK_H_ */
