/* Documentation tag for Doxygen
 */

/*! \mainpage Informations générales
 *
 * \section intro_sec Introduction
 * Ceci est un Space Invaders sous la forme d'une application temps-réel soft tournant sur un
 * Linux/Xenomai embarqué
 * sur une plateforme à base de micro-contrôleur Freescale i.MX21 avec processeur ARM9. <br>
 * Cette dernière dispose d'un écran tactile et d'une carte d'interface
 * comprenant différents composants comme boutons-poussoir, LEDs. oscillateur, etc.
 *
 * Contexte 	: Laboratoire d'informatique embarquée (IEM) à la HEIG-VD d'Yverdon-les-Bains <br>
 *
 * Auteurs		: Michel Favaretto, Yannick Lanz, Romain Maffina, Mohamed Regaya <br>
 * Professeur 	: Daniel Rossier <br>
 * Assistant 	: Lionel Sambuc <br>
 *
 * \section install_sec Installation
 * Pré-requis : Utiliser les plateformes embarqués EMB ainsi que les postes de travail de
 * l'institut ReDS.
 * - Démarrer le poste sous Linux
 * - Connecter la carte avec le câble Ethernet ainsi que le câble série et l'alimenter
 * - Générer les deux modules rt-app.ko ainsi que pca9554.ko
 * - Les copier dans le dossier "share_freescale" présent sur le bureau de redsuser
 * - Démarrer minicom dans un terminal et lancer le script startlinux
 * - Une fois le linux démarré saisir les commandes suivantes :
 * 		- cd var/
 * 		- tftp -g -r pca9554.ko 10.0.0.1
 * 		- tftp -g -r rt-app.ko 10.0.0.1
 *
 * \subsection running Lancer l'application
 * Pré-requis : Avoir suivi les instructions de la section "Installation"
 * - Dans le linux embarqué saisir les commandes suivantes :
 * 		- insmod pca9554.ko
 * 		- insmod rt-app.ko
 *
 * Attention il est important de faire les commandes dans cet ordre (rt-app.ko est dépendant de pca9554.ko)!
 *
 * \subsection stopping Arrêter l'application
 * Pré-requis : Avoir suivi les instructions de la section "Lancer l'application"
 * - Dans le linux embarqué saisir les commandes suivantes :
 * 		- rmmod rt-app.ko
 * 		- rmmod pca9554.ko
 *
 * Attention il est important de faire les commandes dans cet ordre (rt-app.ko est dépendant de pca9554.ko)!
 */

/*!
 * \file rt-app-m.c
 * \brief Fichier (body) principal de l'application space invader
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (body) principal de l'application space invader.
 * L'application space invader est un module linux basé
 * sur Xenomai. Ce fichier principal est le point d'entrée du module
 * et appelle ensuite toutes les initialisations des tâches démarrant
 * ainsi l'application.
 * Ce module dépend du module de gestion du PCA9554 (io expander) basé
 * sur le protocole série I2C et doit donc être inséré après le module
 * pca9554.ko.
 */
#include <linux/module.h>
#include "rt-app-m.h"
#include "pca9554-m.h"
#include "xeno-i2c.h"
#include "xeno-ts.h"
#include "lcdlib.h"
#include "global.h"
// Inclusion des headers pour les tâches
#include "invaders_task.h"
#include "fb_task.h"
#include "io_task.h"
#include "hit_task.h"
#include "ship_task.h"
#include "lcdlib.h"

RT_INTR isrDesc;				/*!< Descripteur d'ISR pour l'ISR de l'I2C */

/*! Descripteur de heap pour le double buffering
 * Nous créons la heap ici car dans un cas générale, elle pourrait être utilisée pour
 * d'autre fonctions que seulement le double buffering
 */
RT_HEAP heap;
uint8_t heap_created = 0;		/*!< Flag pour savoir si la heap à pu être crée */
uint8_t heap_allocated = 0;		/*!< Flag pour savoir si la heap à pu être allouée */

/*! \brief Fonction de création de l'application
 *
 *  Fonction appelée lorsque l'initialisation de l'I2C est faite par le module_init
 *  (point d'entrée de l'application)
 */
static int space_invader(void)
{
	int err;

	err = rt_intr_enable(&isrDesc);

	if (err != 0) {
		printk("rt-app: Could not enable I2C ISR\n");
		goto fail;
	}

	printk("rt-app: Enabled ISR\n");

	err = rt_heap_create(&heap, "rt_heap", 240*320*2, 0);

	if(err != 0){
		printk("rt-app: Could not create the rt heap\n");
		goto fail;
	}else{
		heap_created = 1;
	}
	printk("rt-app: RT-Heap created\n");

	// On essaie de créer le tas pour le double buffering
	err = rt_heap_alloc(&heap, 240*320*2, TM_NONBLOCK, &fb_mem_rt);
	if(err != 0){
		printk("rt-app: Could not allocate the rt heap\n");
		goto fail;
	}else{
		heap_allocated = 1;
	}
	printk("rt-app: RT-Heap allocated\n");

	// On crée la tâche pour les invaders
	if(invaders_task_start() != 0){
		goto fail;
	}

	// On crée la tâche pour les collisions
	if(hit_task_start() != 0){
		goto fail;
	}

	// On crée la tâche pour la gestion des entrées/sorties
	if(io_task_start() != 0){
		goto fail;
	}

	// On crée la tâche pour la gestion du frame buffer
	if(fb_task_start() != 0){
		goto fail;
	}

	// On crée la tâche pour le vaisseau
	if(ship_task_start() != 0){
		goto fail;
	}

	return 0;

	// En cas d'échec de création de l'ISR ou d'une tâche
fail:
	cleanup_module();
	return -1;

}

/*! \brief Routine d'interruption pour gérer les interruption I2C
 *  \param _idesc Structure contenant les données de l'interrupt
 */
static int imx_i2c_handler(struct xnintr* _idesc) {

	/* safe status register */
	set_i2c_imx_i2sr(get_i2c_imx_reg()->i2sr);

	/* if data transfer is complete set ok */
	if (get_i2c_imx_i2sr() & (u32)0x80)  /* [I2SR:ICF] TX complete */
		set_i2c_imx_irq_ok(1);

	/* clear irq */
	get_i2c_imx_reg()->i2sr = get_i2c_imx_reg()->i2sr & ~(u32)0x02; /* clear [I2SR:IIF] Interrupt */

	return RT_INTR_HANDLED; /* Ne propage pas l'irq dans le domaine linux */
}

int __init init_module(void) {
	int err;

	/* Initialisation du timer */
	if (TIMER_PERIODIC)
		err = rt_timer_set_mode(MS);
	else
		err = rt_timer_set_mode(TM_ONESHOT);

	if (err != 0) {
		printk("rt-app: %s: Error timer: %d\n", __func__, err);
		return -1;
	}

	/* Initialize FB */
	fb_init();
	printk("rt-app: Framebuffer initialized\n");

	xeno_ts_init();
	printk("rt-app: Touchscreen initialized\n");

	/* Initializing IRQ */
	err = rt_intr_create(&isrDesc, "IMX_I2C", INT_I2C, imx_i2c_handler, NULL, 0);
	if (err != 0) {
		printk("rt-app: %s: Error interrupt registration: %d\n", __func__, err);
		goto fail_intr;
	}
	printk("rt-app: ISR initialized\n");

	printk("rt-app: i2c driver call\n");

	printk("rt_app: i2c driver ok\n");

	return space_invader();

fail_intr:
	xeno_ts_exit();

	return -1;
}

void __exit cleanup_module(void) {

	// On nettoie toutes les tâches
	invaders_task_cleanup_task();
	fb_task_cleanup_task();
	io_task_cleanup_task();
	hit_task_cleanup_task();
	ship_task_cleanup_task();

	// On nettoie tous les objets
	invaders_task_cleanup_objects();
	fb_task_cleanup_objects();
	io_task_cleanup_objects();
	hit_task_cleanup_objects();
	ship_task_cleanup_objects();

	// On désalloue le tas
	if(heap_allocated){
		heap_allocated = 0;
		rt_heap_free(&heap, &fb_mem_rt);
	}

	// On supprime le tas
	if(heap_created){
		heap_created = 0;
		rt_heap_delete(&heap);
	}

	rt_intr_delete(&isrDesc);

	xeno_ts_exit();
}

MODULE_LICENSE("GPL");
