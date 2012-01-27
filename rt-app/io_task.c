/*!
 * \file io_task.c
 * \brief Fichier (body) pour la gestion de la tâche des entrées/sorties
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (body) pour la gestion de la tâche des entrées/sorties
 * Cette tâche gère les entrées/sorties (boutons et leds). Lorsqu'un bouton
 * est pressé, c'est cette tâche qui signale à la git task qu'elle doit ajouter
 * des bullets via la foncction hit_task_fire_weapon().
 */
#include "io_task.h"
#include "hit_task.h"
#include "pca9554-m.h"
#include "rt-app-m.h"
#include "ship_task.h"

/**
 * Variables privées
 */
//! Descripteur de la tâche io
RT_TASK io_task_handle;
//! Flag pour savoir si la tâche à été crée correctement
static uint8_t io_task_created = 0;

/**
 * Fonctions privées
 */

/*! \brief Handler utilisé et représentant la tâche de gestion des entrées/sorties
 *  \param cookie Paramètre factultatif et non utilisé (besion de Xenomai)
 *
 *  Cette fonction représente le corps proprement dit de la tâche de gestion
 *  des entrées/sorties. C'est dans cette fonction que toute la gestion s'effectue
 *  et que les autre fonction sont appelées.
 *  Cette fonction ne doit pas être appelée directement mais seulement lors de
 *  l'init (io_task_start()) à l'aide de rt_task_create().
 */
static void io_task(void *cookie);

int io_task_start(){
	int err;

	// On essai de créer la tâche pour les entrées/sorties
	err = rt_task_create(&io_task_handle,
	                     "task_io",
	                      TASK_STKSZ,
	                      TASK_IO_PRIO,
	                      0);
	// On test si erreur
	if (err == 0){
		// Si pas d'erreur, on la démarre
		err = rt_task_start(&io_task_handle, &io_task, NULL);
		io_task_created = 1;
		if(err != 0){
			printk("rt-app: Task IO starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task IO starting succeed\n");
		}
	}else{
		printk("rt-app: Task IO creation failed\n");
		goto fail;
	}
	return 0;

	// En cas d'erreur lors de la création/initialisation
fail:
	io_task_cleanup_task();
	io_task_cleanup_objects();
	return -1;
}

void io_task_cleanup_task(){
	if(io_task_created){
		// On la nettoie
		printk("rt-app: Task IO cleanup task\n");
		io_task_created = 0;
		rt_task_delete(&io_task_handle);
	}
}

void io_task_cleanup_objects(){
	// Nous n'avons pas d'obejt à nettoyer
	printk("rt-app: Task IO cleanup objects\n");
}

static void io_task(void *cookie)
{
	int i;
	uint8_t rebond = 0;
	uint8_t counter_io = 0;
	char modif;

	// Tableau pour mémoriser les valeur des switches en fonctionnement normal
	char old_val_sw[4], new_val_sw[4];

	(void)cookie;
	// On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 50*MS);

    for (;;) {
    	rt_task_wait_period(NULL);

    	// On test si pause
		if(!game_break){
			// Léger anti-rebond
			if(counter_io == 1){
				counter_io= 0;

				// On lance la lecture depuis le pca9554
				pca9554_receive();

				modif = 0;
				for(i = 0; i < 4; i++){

					// Gestion de la recharge
					weapons[i+1].timing_charge.time_current++;	// On incrémente le compteur de chargepour toutes les armes
					// Si compteur de charge >= valeur de recharge
					if(weapons[i+1].timing_charge.time_current >= weapons[i+1].timing_charge.time_total){
						weapons[i+1].timing_charge.time_current = 0;
						if(weapons[i+1].timing_charge.now < weapons[i+1].timing_charge.max){
							// On charge l'arme
							weapons[i+1].timing_charge.now++;
						}
					}

					// On lit le switch i
					pca9554_get_switch(i, &new_val_sw[i]);

					// Si changement de valeur
					if(old_val_sw[i] != new_val_sw[i]){
						rebond = 0;
						// Si valeur courante à 1
						if(new_val_sw[i] == 1){
							if(weapons[i+1].timing_charge.now > 0){
								// On tire si encore possible
								weapons[i+1].timing_charge.now--;
								weapons[i+1].timing_charge.last = weapons[i+1].timing_charge.now;
								hit_task_fire_weapon(ship.hitbox,(weapontype_t)(i+1));
							}
						}
						// On met à jour la valeur courante
						old_val_sw[i] = new_val_sw[i];
					}else if(new_val_sw[i] == 1){
						// La valeur n'a pas changée (appui prolongé ?)
						if(rebond > 2){
							// On tir en mode automatique
							if(weapons[i+1].timing_charge.now > 0 &&
							   weapons[i+1].timing_charge.now >= weapons[i+1].timing_charge.last){
								weapons[i+1].timing_charge.now--;
								hit_task_fire_weapon(ship.hitbox,(weapontype_t)(i+1));
							}
						}else{
							rebond++;
						}
					}
				}
			}else{
				counter_io++;
			}

			// Gestion des leds
			for(i = 0; i < 4; i++){
				// Si vide, on eteint la led
				weapons[i+1].timing_led.ratio = (weapons[i+1].timing_led.max*(weapons[i+1].timing_charge.now*100/weapons[i+1].timing_charge.max)/100);

				if(weapons[i+1].timing_led.now++ >= (weapons[i+1].timing_led.max - weapons[i+1].timing_led.ratio) ){
					pca9554_en_led(i);
				}else{
					pca9554_dis_led(i);
				}
				if(weapons[i+1].timing_led.now >= weapons[i+1].timing_led.ratio){
					weapons[i+1].timing_led.now = 0;
				}
			}
			// On envoit les valeur de leds vers le pca9554
			pca9554_send();
		}
    }
}

