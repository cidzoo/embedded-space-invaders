/*!
 * \file invaders_task.c
 * \brief Corps de fichier pour la gestion de la tâche des invaders
 * \author Michael Favaretto
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Corps de fichier pour la tâche de gestion des invaders.
 * Initialise les invaders et gère leur déplacement dans
 * la zone de jeux. Gère la gestion des tirs des bombes des invaders
 * Permet de déterminer la fin d'un niveau lorsque tous les invaders
 * sont morts.
 *
 */

#include <linux/types.h>
#include "lcdlib.h"
#include "invaders_task.h"
#include "hit_task.h"
#include "rt-app-m.h"

/**
 * Variables public
 */
//! Var de type wave_t pour la vague courante des invaders
wave_t wave;

/**
 * Variables privées
 */

//! Mutex afin de protéger l'accès au tableau des invaders
RT_MUTEX invaders_task_mutex;
//! Flag pour savoir si le mutex pour les invaders à été crée avec succès
uint8_t invaders_task_mutex_created = 0;
//! Descripteur de la tâche invaders
RT_TASK invaders_task_handle;
//! Flag pour savoir si la tâche invaders à été crée avec succès
uint8_t invaders_task_created = 0;

//! Flag pour savoir si le niveau est terminé
static int level_finish = 0;

/**
 * Fonctions privées
 */

/*! \brief Handler utilisé et représentant la tâche de gestion des invaders
 *  \param cookie Paramètre factultatif et non utilisé (besion de Xenomai)
 *
 *  Cette fonction représente le corps proprement dit de la tâche de gestion
 *  des invaders. C'est dans cette fonction que toute la gestion s'effectue
 *  et que les autre fonction sont appelées.
 *  Cette fonction ne doit pas être appelée directement mais seulement lors de
 *  l'init (invaders_task_start()) à l'aide de rt_task_create().
 */
static void invaders_task(void *cookie);

/*! \brief Initialise la position des invaders pour un début de niveau
 *
 *  Place les invaders dans la zone de jeux pour le début d'un niveau.
 *  Les invaders sont placés en fonction du nombre d'invader, de leur
 *  taille et de l'espacement désiré entre chacun. Permet de définir
 *  Initialise aussi les caractéristique de chaque invaders (vie,taille,..)
 */
static void invaders_update(void);

/*! \brief Gére le déplacement et le tir des invaders
 *
 *  Permet de déplacer les invaders horizontalement et verticalement.
 *  Définit aussi quand un invaders peut tirer une bombes.
 *  Permet aussi de vérifier si tous les invaders ont été tués et
 *  ainsi indiqué que le niveau est fini.
 */
static void invaders_move(void);

/*! \brief Trouve les dimmension de la vague d'invaders
 *  \param wave_hitbox pointeur de type hitbox_t contenant les dimmension de la wave.
 *
 *  Trouve les invaders se trouvant au 4 extrémités de la vague et qui sont encore
 *  vivants. Ces 4 invaders permettent de déterminer les dimmension de la vague.
 */
static void invaders_get_wave_box(hitbox_t *wave_hitbox);

/*! \brief Fonction retournant une valeur aléatoire entre 0 et un nombre donné en paramètre
 *  \param range Var de type int définissant la valeur maximum du nombre aléatoire.
 *  \return Resultat du nombre aléatoire
 *
 *  Renvoit le nombre aléatoire se trouvant dans entre 0 et le nombre donné
 */
static int invaders_random(int range);

int invaders_task_start() {
	int err;

	// On essai de créer le mutex pour les invaders
	err = rt_mutex_create(&invaders_task_mutex, "task_invader_mutex");

	// On test si erreur
	if (err == 0) {
		// On met à jour le flag de création
		invaders_task_mutex_created = 1;
		printk("rt-app: Task INVADERS create mutex succeed\n");
	} else {
		printk("rt-app: Task INVADERS create mutex failed\n");
		goto fail;
	}

	// On essai de créer la tâche pour les invaders
	err = rt_task_create(&invaders_task_handle, "task_invaders", TASK_STKSZ,
			TASK_INVADERS_PRIO, 0);
	// On test si erreur
	if (err == 0) {
		// Si pas d'erreur, on la démarre
		err = rt_task_start(&invaders_task_handle, &invaders_task, NULL);
		// On met à jour le flag de création
		invaders_task_created = 1;
		if (err != 0) {
			printk("rt-app: Task INVADERS starting failed\n");
			goto fail;
		} else {
			printk("rt-app: Task INVADERS starting succeed\n");
		}
	} else {
		printk("rt-app: Task INVADERS creation failed\n");
		goto fail;
	}
	return 0;
	// En cas d'erreur lors de la création/initialisation
	fail: invaders_task_cleanup_task();
	invaders_task_cleanup_objects();

	return -1;
}

void invaders_task_cleanup_task() {
	// Si tache créée
	if (invaders_task_created) {
		// On la nettoie
		printk("rt-app: Task INVADERS cleanup task\n");
		invaders_task_created = 0;
		rt_task_delete(&invaders_task_handle);
	}
}

void invaders_task_cleanup_objects() {
	// Si mutex créé
	if (invaders_task_mutex_created) {
		//Mise à jour du flag de création et suppresion du mutex
		printk("rt-app: Task INVADERS cleanup objects\n");
		invaders_task_mutex_created = 0;
		rt_mutex_delete(&invaders_task_mutex);
	}
}

static void invaders_task(void *cookie) {
	// On initialise les invaders
	invaders_task_init();

	// On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 100 * MS);

	for (;;) {
		rt_task_wait_period(NULL);

		//Test si le jeu est en pause
		if (!game_break) {
			//Test si le niveau est terminé
			if (level_finish) {
				//On level up les invaders
				level_up();
				level_finish = 0;

				//Informe la tâche fb_task que le niveau est terminé
				game_level_up = 1;

				//On met à jour les invaders pour le nouveau niveau
				invaders_lock();
				invaders_update();
				invaders_unlock();
			}

			//Déplace les invaders
			invaders_lock();
			invaders_move();
			invaders_unlock();
		}
	}
}

void invaders_task_init() {
	invaders_lock();
	//Initialise la vitesse pour le niveau 1
	wave.invader_speed = 1;
	wave.level = 0;
	//Nombre d'invader au départ du niveau 1
	wave.invaders_count = 6;
	invaders_update();
	invaders_unlock();
}

void invaders_update() {
	int nb_invaders_per_line[NB_INVADERS_MAX] = { 0 };
	int line = 0;
	int invader_id = 0;
	int i, j;

	//Détermine le nombre d'ennemi par ligne
	for (i = 0; i < wave.invaders_count; i++) {

		//Initialise les valeurs chaque ennemi
		wave.invaders[i].hp = 10;
		wave.invaders[i].hitbox.height = HEIGT_INVADER;
		wave.invaders[i].hitbox.width = WIDTH_INVADER;
		wave.invaders[i].hitbox.type = G_INVADER;

		//Incrément le nombre d'invader de la ligne
		nb_invaders_per_line[line]++;

		//Détermine si la ligne est complète
		if ((GAME_ZONE_X_MAX - GAME_ZONE_X_MIN - 2*MARGE - WIDTH_INVADER
				- (nb_invaders_per_line[line]
						* (SPACE_BETWEEN_INVADER + WIDTH_INVADER))) < 0)
			//Passe à la ligne suivante
			line++;
	}

	//Placement des invaders
	for (i = 0; i <= line; i++) {

		for (j = 0; j < nb_invaders_per_line[i]; j++) {

			//Si dernière ligne invaders -> décallage pour aligner les invaders
			//dans la zone de jeu
			if (line != 0 && i == line) {
				wave.invaders[invader_id].hitbox.x = (GAME_ZONE_X_MIN + 2*MARGE
						+ ((WIDTH_INVADER+SPACE_BETWEEN_INVADER)
								* ((nb_invaders_per_line[0]
										- nb_invaders_per_line[i])/2)))
						+ (j * (SPACE_BETWEEN_INVADER + WIDTH_INVADER));
				wave.invaders[invader_id].hitbox.y = (GAME_ZONE_Y_MIN
						+ SPACE_BETWEEN_INVADER)
						+ (i * (SPACE_BETWEEN_INVADER + HEIGT_INVADER));
			//Sinon alignement normal des invaders
			} else {
				wave.invaders[invader_id].hitbox.x = (GAME_ZONE_X_MIN + MARGE)
						+ (j * (SPACE_BETWEEN_INVADER + WIDTH_INVADER));
				wave.invaders[invader_id].hitbox.y = (GAME_ZONE_Y_MIN
						+ SPACE_BETWEEN_INVADER)
						+ (i * (SPACE_BETWEEN_INVADER + HEIGT_INVADER));
			}
			//Passe à l'invader suivant
			invader_id++;
		}
	}
}

void invaders_move() {
	int i;
	int x = 0;

	//Variable qui détermine si les invaders doivent se déplacer horizontalement
	static int move_up = 0;
	//Variable pour déterminer si déplacement à gauche ou à droite des invaders
	static int moving_right = 1;
	hitbox_t dimension;
	int invader_dead = 0;

	//Détermine la dimmension de la vague
	invaders_get_wave_box(&dimension);

	//Déplacement à droite
	if (moving_right) {
		if ((dimension.x + dimension.width + wave.invader_speed)
				< GAME_ZONE_X_MAX - MARGE) {
			x = wave.invader_speed;
		} else {
			x = LCD_MAX_X - MARGE - (dimension.x + dimension.width);
			moving_right = 0;
			move_up = 1;
		}
	}
	//Déplacement à gauche
	else {
		if (((int32_t) dimension.x - (int32_t) wave.invader_speed)
				> GAME_ZONE_X_MIN + MARGE) {
			x = -wave.invader_speed;
		} else {
			x = -dimension.x + MARGE;
			moving_right = 1;
			move_up = 1;
		}
	}

	for (i = 0; i < wave.invaders_count; i++) {
		wave.invaders[i].hitbox.x += x;

		//Avance les invaders verticalement
		if (move_up)
			wave.invaders[i].hitbox.y += wave.level + 1;

		//vérifie si un invader à atteint la terre
		if (wave.invaders[i].hitbox.y + wave.invaders[i].hitbox.height
				== GAME_ZONE_Y_MAX
			)
			game_over = 1;

		//Compte le nombre d'invaders morts
		if (wave.invaders[i].hp <= 0)
			invader_dead++;
	}

	//Stope le déplacement verticale des invaders
	if (move_up)
		move_up = 0;

	// Test si le niveau est terminé
	if (invader_dead == wave.invaders_count)
		level_finish = 1;

	// Si la fonction random retourne 1, les invaders peuvent tirer des bombes
	if (invaders_random(40 - wave.level) == 1) {
		for (i = 0; i < 1 + wave.level; i++) {
			while (1) {

				//Détermine de manière alétoire quel invader va pouvoir tirer
				uint8_t rand = invaders_random(wave.invaders_count);
				if (wave.invaders[rand].hp > 0) {
					//Tire une bombe
					fire_weapon(wave.invaders[rand].hitbox, BOMB);
					break;
				}
			}
		}
	}
}


void invaders_get_wave_box(hitbox_t *wave_hitbox) {

	int i;

	//Initialise les valeur des dimmension de la vague avec
	//Les extrémes opposés
	wave_hitbox->x = LCD_MAX_X;
	wave_hitbox->y = LCD_MAX_Y;
	wave_hitbox->width = 0;
	wave_hitbox->height = 0;


	for (i = 0; i < wave.invaders_count; i++) {
		if (wave.invaders[i].hp > 0) {
			//Détermine la coordonnée x de la vague
			if (wave.invaders[i].hitbox.x < wave_hitbox->x) {
				wave_hitbox->x = wave.invaders[i].hitbox.x;
			}
			//Détermine la coordonnée y de la vague
			if (wave.invaders[i].hitbox.y < wave_hitbox->y) {
				wave_hitbox->y = wave.invaders[i].hitbox.y + 1;
			}

			//Détermine la largeur de la vague
			if (wave.invaders[i].hitbox.x + wave.invaders[i].hitbox.width
					> wave_hitbox->x + wave_hitbox->width) {
				wave_hitbox->width = (wave.invaders[i].hitbox.x
						+ wave.invaders[i].hitbox.width) - wave_hitbox->x - 1;
			}
			//Détermine la hauteur de la vague
			if (wave.invaders[i].hitbox.y + wave.invaders[i].hitbox.height
					> wave_hitbox->y + wave_hitbox->height) {
				wave_hitbox->height = (wave.invaders[i].hitbox.y
						+ wave.invaders[i].hitbox.height) - wave_hitbox->y;
			}
		}
	}
}

int invaders_lock() {
	//Bloque le mutex si l'accès au tableau d'invader est critique
	if (invaders_task_mutex_created) {
		return rt_mutex_lock(&invaders_task_mutex, TM_INFINITE);
	}
	return -1;
}

int invaders_unlock() {
	//Débloque le mutex
	if (invaders_task_mutex_created) {
		return rt_mutex_unlock(&invaders_task_mutex);
	}
	return -1;
}


void level_up() {

	//Passe au niveau suivant
	wave.level++;

	//Augment de 2 le nombre d'invaders tant que la valeur maximum d'invaders
	//n'est pas atteinte
	if (wave.invaders_count < NB_INVADERS_MAX
		)
		wave.invaders_count += 2;

	//Détermine le niveau de difficulté du jeu
	wave.invader_speed = 1 + difficulty;
}

int invaders_random(int range) {
	//Retourne un nombre aléatoire entre 0 et range
	return get_random() % range;
}

