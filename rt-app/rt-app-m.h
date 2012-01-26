/*!
 * \file rt-app-m.h
 * \brief Fichier (header) principal de l'application space invader
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (header) principal de l'application space invader.
 * L'application space invader est un module linux basé
 * sur Xenomai. Ce fichier principal est le point d'entrée du module
 * et appelle ensuite toutes les initialisations des tâches démarrant
 * ainsi l'application.
 * Ce module dépend du module de gestion du PCA9554 (io expander) basé
 * sur le protocole série I2C et doit donc être inséré après le module
 * pca9554.ko.
 */

#ifndef __RT_APP_M_H__
#define __RT_APP_M_H__
/******************************************************************************
 *                           Xenomai Definitions
 *****************************************************************************/
#define TIMER_PERIODIC	0           /**< 1:Periodic timer, 0:Aperiodic timer */

#define STACK_SIZE		8192        /**< Default stack size */
#define MS				1000000		/**< 1 ms in ns */

/*! \brief Point d'entrée du module rt-app
 *  Fonction appelée lorsque l'on insère le module dans le noyau
 */
int __init init_module(void);

/*! \brief Point de sortie du module rt-app
 *  Fonction appelée lorsque l'on ressort le module du noyau
 */
void __exit cleanup_module(void);

#endif /* __RT_APP_M_H__ */
