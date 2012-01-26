/*!
 * \file fb_task.c
 * \brief Fichier (body) pour la gestion de la tâche de gestion du frame buffer
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (body) pour la tâche de gestion du frame buffer.
 * A chaque refraichissement de l'écran (définit par la période de cette tâche),
 * récupère les données des invaders, du vaisseau, des bombes et des bullets
 * puis les affiche à l'écran.
 * Affiche aussi le header contenant les statistiques (vie, précision).
 * Cette tâche gère aussi les menus (début, pause, game over) et récupère
 * les informations du touchscreen (venant de la tâche ship) quand elle en a
 * besoin (pour les boutons des menus lorsque le jeu est arrêté).
 */
#include <linux/slab.h>
#include "rt-app-m.h"
/* Inclusions pour les tâche (définitions de type aussi) */
#include "fb_task.h"
#include "hit_task.h"
#include "invaders_task.h"
#include "ship_task.h"
/* Inclusions pour les outils */
#include "lcdlib.h"
#include "vga_lookup.h"

/*!
 * \struct menu_t
 * \brief Objet représentant un menu (object graphique)
 *
 * menu_t représente un menu et contient une position absolue (x, y),
 * une taille (largeur, hauteur) ainsi qu'un titre.
 * Il est utilisé lorsqu'on fait appel à la fonction draw_menu.
 * Ce type n'est utilisé que dans la fb_task est n'est donc pas définit
 * dans le header.
 */
typedef struct{
	uint16_t x;				/*!< Position en x */
	uint16_t y;				/*!< Position en y */
	uint16_t width;			/*!< Largeur */
	uint16_t height;		/*!< Hauteur */
	char *title;			/*!< Titre */
} menu_t;

/**
 * Variables privées
 */

//! Var de type button_t pour le bouton de début de partie
static button_t start_button = {
	.x = 40,
	.y = 170,
	.width = 160,
	.height = 50,
	.title = "START"
};

//! Var de type button_t pour le bouton de recommencement dans le menu pause
static button_t restart_break_button = {
	.x = 40,
	.y = 170,
	.width = 75,
	.height = 50,
	.title = "RESTART"
};

//! Var de type button_t pour le bouton pour continuer dans le menu pause
static button_t continue_button = {
	.x = 125,
	.y = 170,
	.width = 75,
	.height = 50,
	.title = "CONTINUE"
};

//! Var de type button_t pour le bouton de recommencement après un game over
static button_t restart_game_over_button = {
	.x = 40,
	.y = 210,
	.width = 160,
	.height = 50,
	.title = "RESTART"
};

//! Var de type menu_t pour le menu de commencement
static menu_t begin_menu = {
	.x = 30,
	.y = 40,
	.width = 180,
	.height = 190,
	.title = "SPACE INVADERS"
};

//! Var de type menu_t pour le menu de pause
static menu_t pause_menu = {
	.x = 30,
	.y = 40,
	.width = 180,
	.height = 190,
	.title = "PAUSE"
};

//! Var de type menu_t pour le menu de game over
static menu_t game_over_menu = {
	.x = 30,
	.y = 40,
	.width = 180,
	.height = 230,
	.title = "GAME OVER"
};

//! Descripteur de la tâche frame buffer
RT_TASK fb_task_handle;
//! Flag pour savoir si la tâche fb à été crée avec succès
static uint8_t fb_task_created = 0;

//! Var pour contenir localement la wave d'invaders (snapshot)
static wave_t wave_loc;
//! Var pour contenir localement les bullets (snapshot)
static bullet_t bullets_loc[NB_MAX_BULLETS];
//! Var pour contenir localement le vaisseau (snapshot)
static spaceship_t ship_loc;
//! Var pour contenir localement les points (snapshot)
uint32_t game_points_loc;

/**
 * Fonctions privées
 */

/*! \brief Handler utilisé et représentant la tâche de gestion du frame buffer
 *  \param cookie Paramètre factultatif et non utilisé (besion de Xenomai)
 *
 *  Cette fonction représente le corps proprement dit de la tâche de gestion
 *  du frame buffer. C'est dans cette fonction que toute la gestion s'effectue
 *  et que les autre fonction sont appelées.
 *  Cette fonction ne doit pas être appelée directement mais seulement lors de
 *  l'init (fb_task_start()) à l'aide de rt_task_create().
 */
static void fb_task(void *cookie);

/*! \brief Fonction pour rafraichir graphiquement l'écran de façon basique.
 *
 *  Cette fonction est appelée périodiquement avant d'afficher autre chose sur l'écran.
 *  Elle met en place les bases graphique comme le fond, les invaders, le vaisseau,
 *  les bombes, les bullets ainsi que le header soit tout ce qui doit aparaitre dans
 *  tous les cas.
 */
static void fb_task_update(void);

/*! \brief Fonction afficher un bitmap ayant les paramètres définis par la hitbox.
 *  \param hb Var de type hitbox_t contenant les params.
 *
 *  Affiche un bitmap selon les paramètres de la hitbox passée en paramètre.
 *  La hitbox contient la position (x, y), une taille (largeur, hauteur) ainsi
 *  qu'un pointeur vers un tableau à 2 dimensions contenant le bitmap.
 */
static void draw_bitmap(hitbox_t hb);

/*! \brief Fonction afficher l'animation de l'invader en fond du menu.
 *  \param x Position en x de l'animation
 *  \param y Position en y de l'animation
 *
 *  Affiche l'animation du bitmap invader_menu en x, y.
 */
static void draw_invader_menu(uint16_t x, uint16_t y);

/*! \brief Fonction afficher un menu de type menu_t.
 *  \param menu Var de type menu_t contenant les données du menu à afficher.
 *
 *	Affiche un menu selon le paramètre menu passé. Le menu_t contient notament
 *	la position et le titre du menu.
 */
static void draw_menu(menu_t menu);

/*! \brief Fonction afficher le menu de démarrage
 */
static void draw_menu_begin(void);

/*! \brief Fonction afficher le menu de pause
 */
static void draw_menu_pause(void);

/*! \brief Fonction afficher le menu de fin de jeu
 */
static void draw_menu_game_over(void);

/*! \brief Fonction pour checker si une position (x, y) chevauche un bouton.
 *  \param button Var de type button_t représentant le bouton (et donc sa surface) à tester
 *  \param x Var pour la position en x
 *  \param y Var pour la position en y
 *  \return Resultat du chevauchement
 *  	\retval 0 La position et le bouton se chevauchent
 *  	\retval -1 La position et le bouton ne se chevauchent pas
 *
 *  Renvoit le résultat du chevauchement entre un bouton et une position (définit par x et y)
 */
static int check_button(button_t button, uint16_t x, uint16_t y);

/*! \brief Fonction pour afficher les statistiques de jeu à la position définie par x et y
 *  \param x Var pour la position en x
 *  \param y Var pour la position en y
 */
static void draw_stats(uint16_t x, uint16_t y);

/*! \brief Fonction pour afficher les crédits (nom des dev.) de jeu à la position définie par x et y
 *  \param x Var pour la position en x
 *  \param y Var pour la position en y
 */
static void draw_credits(uint16_t x, uint16_t y);

/*! \brief Fonction pour afficher les règles de jeu à la position définie par x et y
 *  \param x Var pour la position en x
 *  \param y Var pour la position en y
 */
static void draw_rules(uint16_t x, uint16_t y);

int fb_task_start() {
	int err;

	// On essai de créer la tâche pour le frame buffer
	err = rt_task_create(&fb_task_handle, "task_fb", TASK_STKSZ, TASK_FB_PRIO, 0);

	// On test si erreur
	if (err == 0) {
		// Si pas d'erreur, on la démarre
		err = rt_task_start(&fb_task_handle, &fb_task, NULL);
		// On met à jour le flag de création
		fb_task_created = 1;
		if (err != 0) {
			printk("rt-app: Task FB starting failed\n");
			goto fail;
		} else {
			printk("rt-app: Task FB starting succeed\n");
		}
	} else {
		printk("rt-app: Task FB creation failed\n");
		goto fail;
	}
	return 0;

	// En cas d'erreur lors de la création/initialisation
fail:
	fb_task_cleanup_task();
	fb_task_cleanup_objects();
	return -1;
}

void fb_task_cleanup_task() {
	// Si tache créée
	if (fb_task_created) {
		// On la nettoie
		printk("rt-app: Task FB cleanup task\n");
		rt_task_delete(&fb_task_handle);
		fb_task_created = 0;
	}
}

void fb_task_cleanup_objects() {
	// Nous n'avons pas d'obejt à nettoyer
	printk("rt-app: Task FB cleanup objects\n");
}

static void fb_task(void *cookie) {
	uint8_t update = 1;

	(void) cookie;

	// On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 50 * MS);

	for (;;) {
		// On attend la prochaine période
		rt_task_wait_period(NULL);

		// Si flag de level_up (depuis la tache invaders)
		if(game_level_up){
			game_level_up = 0;
			// On re-init la tache de collisions (suppression de tous les bullets, bombs)
			hit_task_init();
		}

		// On test si nous devons initialisé l'affichage (avant le menu de lancement)
		if(update){
			update = 0;
			// On rafraichit l'ecran avec tous les paramètres des autres taches
			fb_task_update();
			// On assombrit l'ecran
			int y, x;
			for (y = 0; y <= 319; y++) {
				for(x = 0; x <= 239; x++){
					*((unsigned short int*) (fb_mem_rt + 2 * x + y * 480)) &= (RED_SUBPIXEL(0x11) | GREEN_SUBPIXEL(0x11)| BLUE_SUBPIXEL(0x11));
				}
			}
		}

		// On test s'il y a game over
		if(!game_over){		// Il n'y pas game over
			// On test si le jeux est en pause
			if(!game_break){// Le jeu n'est pas arrêté
				// On rafraichit l'ecran avec tous les paramètres des autres taches
				fb_task_update();

				// On test si pression sur l'écran
				if(screen_pressed){
					screen_pressed = 0;	// On remet le flag à 0

					// On test si pression sur le haut de l'écran
					if(screen_y <= 100){
						// On met le jeu en pause
						game_break = 1;
						update = 1;
					}
				}
			}else{			// Le jeu est arrêté
				// On test si le jeu est commencé (faux au lancement)
				if(game_started){
					// Le jeu est commencer mais arrêté => nous sommes en pause
					// On affiche le menu de pause
					draw_menu_pause();
					// On test si pression sur l'écran
					if(screen_pressed){
						screen_pressed = 0;	// On remet le flag à 0

						// On test si pression sur le bouton continuer
						if(check_button(continue_button, screen_x, screen_y) == 0){
							// On veut sortir du menu de pause
							game_break = 0;
						}else if(check_button(restart_break_button, screen_x, screen_y) == 0){
							// On veut recommencer
							// On remet à 0 les paramètres de chaque tache
							hit_task_init();
							ship_task_init();
							invaders_task_init();

							// On init les stats
							game_bullet_kill = 0;
							game_bullet_used = 0;
							game_points = 0;

							// On débloque le jeu
							game_break = 0;
						}
					}
				}else{
					// Le jeu n'est pas commencé mais arrêté, il s'agit de la fenêtre d'accueil
					// On affiche le menu d'accueil
					draw_menu_begin();
					// On test si pression sur l'écran
					if(screen_pressed){
						screen_pressed = 0;	// On remet le flag à 0

						// On test si pression sur le bouton continuer
						if(check_button(start_button, screen_x, screen_y) == 0){
							// On veut commencer le jeu
							game_started = 1;
							// On veut sortir du menu de pause
							game_break = 0;
						}
					}
				}
			}
		}else{				// Il y a game over
			if(!game_break){
				// Le jeu doit s'arreter, on met game_break à 1
				game_break = 1;
				update = 1;
			}
			// On dessine le menu de game over
			draw_menu_game_over();
			// On test si pression sur l'écran
			if(screen_pressed){
				screen_pressed = 0;	// On remet le flag à 0

				// On test si pression sur le bouton recommencer
				if(check_button(restart_game_over_button, screen_x, screen_y) == 0){
					// On remet à 0 les paramètres de chaque tache
					hit_task_init();
					ship_task_init();
					invaders_task_init();

					// On init les stats
					game_bullet_kill = 0;
					game_bullet_used = 0;
					game_points = 0;

					// Le jeu n'est plus en pause
					game_break = 0;
					// Le jeu n'est plus en game over
					game_over = 0;
				}
			}
		}

		// On met à jour l'affichage
		rt_task_set_priority(NULL, 90);
		fb_display();
		rt_task_set_priority(NULL, 70);
	}
}

static void fb_task_update(void){
	int i;
	progress_bar_t pb;
	char buf[30];

	// On copie les invaders en local
	invaders_lock();
	memcpy(&wave_loc, &wave, sizeof(wave_loc));
	invaders_unlock();

	// On copie le vaisseau en local
	ship_lock();
	memcpy(&ship_loc, &ship, sizeof(ship_loc));
	ship_unlock();

	// On copie les bullets en local
	hit_lock();
	memcpy(bullets_loc, bullets, sizeof(bullets_loc));
	game_points_loc = game_points;
	hit_unlock();

	// On dessine le background
	fb_rect_fill(10, 319, 0, 239, LU_BLACK);

	// On dessine les bullets
	for (i = 0; i < NB_MAX_BULLETS; i++) {
		if (bullets_loc[i].weapon != NULL) {
			draw_bitmap(bullets_loc[i].hitbox);
		}
	}

	// On dessine les bombs
	for (i = 0; i < NB_MAX_BOMBS; i++) {
		if (bombs[i].weapon != NULL) {
			draw_bitmap(bombs[i].hitbox);
		}
	}

	// On dessine les invaders
	for (i = 0; i < wave_loc.invaders_count; i++) {
		if (wave_loc.invaders[i].hp > 0) {
			draw_bitmap(wave_loc.invaders[i].hitbox);
		}
	}

	// On dessine le vaisseau
	draw_bitmap(ship_loc.hitbox);

	// On dessine le header
	fb_rect_fill(0, GAME_ZONE_Y_MIN, 0, GAME_ZONE_X_MAX - 1, LU_GREY);
	fb_line(0, GAME_ZONE_Y_MIN, GAME_ZONE_X_MAX, GAME_ZONE_Y_MIN,
			LU_WHITE);

	// On print le texte pour la progress bar
	fb_print_string(LU_BLACK, LU_GREY, "    life:", 3, 3);
	// On print la progress bar pour la vie
	pb.x = 75;
	pb.y = 2;
	pb.width = 155;
	pb.height = 8;
	pb.current_value = ship_loc.hp;
	pb.max_value = LIFE_SHIP;
	// On determine la couleur
	if(100*ship_loc.hp/LIFE_SHIP > 70){
		pb.couleur = LU_PB_GREEN;
	}else if(100*ship_loc.hp/LIFE_SHIP > 40){
		pb.couleur = LU_PB_ORANGE;
	}else{
		pb.couleur = LU_PB_RED;
	}
	fb_progress_bar(pb);

	// On print le texte pour la progress bar
	fb_print_string(LU_BLACK, LU_GREY, "accuracy:", 3, 13);
	// On print la progress bar pour la precision
	pb.x = 75;
	pb.y = 12;
	pb.width = 155;
	pb.height = 8;
	if(game_bullet_used > 0){
		pb.current_value = game_bullet_kill;
		pb.max_value = game_bullet_used;
		// On determine la couleur
		if(game_bullet_used > 0){
			if(100*game_bullet_kill/game_bullet_used > 70){
				pb.couleur = LU_PB_GREEN;
			}else if(100*game_bullet_kill/game_bullet_used > 40){
				pb.couleur = LU_PB_ORANGE;
			}else{
				pb.couleur = LU_PB_RED;
			}
		}
	}else{
		pb.current_value = 1;
		pb.max_value = 1;
		pb.couleur = LU_DARK_GREY;
	}

	fb_progress_bar(pb);

	// On affiche les points
	sprintf(buf, "  points: %d", game_points);
	fb_print_string_transparent(LU_BLACK, buf, 3, 25);

	// On affiche le niveau de la vague
	sprintf(buf, "wave: %d", wave.level+1);
	fb_print_string_transparent(LU_BLACK, buf, 150, 25);
}



static void draw_bitmap(hitbox_t hb) {
	int i, j;
	for (i = 0; i < hb.height; i++) {
		for (j = 0; j < hb.width; j++) {
			if (*(hb.bitmap + i*hb.width + j) != LU_BLACK) {
				fb_set_pixel(hb.y + hb.height - i, hb.x + j, *(hb.bitmap + i*hb.width + j));
			}
		}
	}
}

static void draw_invader_menu(uint16_t x, uint16_t y){

	static uint8_t invader_bmp_select = 0;		// Invader du menu à 1 au debut
	static uint8_t invader_bmp_counter = 0;		// Timer pour le changement de l'invader

	static hitbox_t hitbox_invader_menu = {
		0, 		// x par défaut 52
		0, 		// y par défaut 70
		136, 	// width
		99 		// height
	};

	hitbox_invader_menu.x = x;		// On assigne le bon x
	hitbox_invader_menu.y = y;		// On assigne le bon y

	// On test quel invader on doit dessiner (1 ou 2 ?)
	if (invader_bmp_select == 0) {	// Image 1
		hitbox_invader_menu.bitmap = bmp_invader_menu1;
		draw_bitmap(hitbox_invader_menu);
	} else {						// Image 2
		hitbox_invader_menu.bitmap = bmp_invader_menu2;
		draw_bitmap(hitbox_invader_menu);
	}
	// On incrémente le timer
	if (invader_bmp_counter++ >= 10) {	// Si timer finit, on change d'image
		invader_bmp_counter = 0;
		// On change la valeur pour la prochaine image
		invader_bmp_select = !invader_bmp_select;
	}
}

static void draw_menu(menu_t menu){
	// On dessine le tour
	fb_rect(menu.y, menu.y + menu.height, menu.x, menu.x + menu.width, LU_WHITE);
	// On dessine le fond
	fb_rect_fill(menu.y + 1, menu.y + menu.height - 1, menu.x + 1, menu.x + menu.width - 1, LU_GREY_BACK);
	// On dessine le titre
	fb_print_string_transparent(LU_BLACK, menu.title, menu.x + ((menu.width-strlen(menu.title)*8)/2), menu.y + 5);
	//On dessine le fond (invader)
	draw_invader_menu(menu.x + 25, menu.y + 20);
	// On dessine les crédits
	//draw_credits(menu.x + 10, menu.y + 30);
}

static void draw_menu_begin(){
	// On dessine le menu de base
	draw_menu(begin_menu);
	// On affiche les règles du jeu
	draw_rules(begin_menu.x + 10, begin_menu.y + 25);
	// On dessine le bouton pour commencer
	fb_button(start_button);
}

static void draw_menu_pause(){
	// On dessine le menu de base
	draw_menu(pause_menu);
	// On affiche les règles du jeu
	draw_rules(begin_menu.x + 10, begin_menu.y + 25);
	// On dessine le bouton pour retourner au jeu
	fb_button(continue_button);
	// On dessine le bouton pour recommencer le jeu
	fb_button(restart_break_button);
}

static void draw_menu_game_over(){
	// On dessine le menu de base
	draw_menu(game_over_menu);
	// On dessine les stats
	draw_stats(game_over_menu.x + 10, game_over_menu.y + 30);
	// On dessine les crédits
	draw_credits(game_over_menu.x + 10, game_over_menu.y + 100);
	// On dessine le bouton pour recommencer le jeu
	fb_button(restart_game_over_button);
}

static void draw_stats(uint16_t x, uint16_t y){
	char buf[30];
	int16_t accuracy;
	// On déclare la progress bar
	progress_bar_t pb = {
		.x = x,
		.y = y + 45,
		.width = 160,
		.height = 8,
		.current_value = game_bullet_kill,
		.max_value = game_bullet_used
	};

	if(game_bullet_used > 0)
		accuracy = 100*game_bullet_kill/game_bullet_used;
	else
		accuracy = 0;
	// On affiche le nombre de points
	sprintf(buf, "POINTS:      %d", game_points);
	fb_print_string_transparent(LU_BLACK, buf, x, y);
	// On affiche le nombre de tir ayant touché un invader
	sprintf(buf, "BULLETS EFF: %d", game_bullet_kill);
	fb_print_string_transparent(LU_BLACK, buf, x, y + 10);
	// On affiche le nombre de tir totaux
	sprintf(buf, "BULLETS TOT: %d", game_bullet_used);
	fb_print_string_transparent(LU_BLACK, buf, x, y + 20);
	// On affiche la précision
	sprintf(buf, "ACCURACY:    %d%%", accuracy);
	fb_print_string_transparent(LU_BLACK, buf, x, y + 35);
	// On test pour la couleur de la progress bar
	if(accuracy > 70){
		pb.couleur = LU_PB_GREEN;
	}else if(accuracy > 40){
		pb.couleur = LU_PB_ORANGE;
	}else{
		pb.couleur = LU_PB_RED;
	}
	fb_progress_bar(pb);
}

static void draw_credits(uint16_t x, uint16_t y){
	// On affiche le titre
	fb_print_string_transparent(LU_BLACK, "CREDITS:", x, y);
	// On affiche les noms
	fb_print_string_transparent(LU_DARK_GREY, "Michael Favaretto", x+10, y+15);
	fb_print_string_transparent(LU_DARK_GREY, "Yannick Lanz", x+10, y+25);
	fb_print_string_transparent(LU_DARK_GREY, "Romain Maffina", x+10, y+35);
	fb_print_string_transparent(LU_DARK_GREY, "Mohamed Regaya", x+10, y+45);
}

static void draw_rules(uint16_t x, uint16_t y){
	fb_print_string_transparent(LU_DARK_GREY, "BUTTON1: WAVE", x, y);
	fb_print_string_transparent(LU_DARK_GREY, "BUTTON2: ROCKET", x, y+10);
	fb_print_string_transparent(LU_DARK_GREY, "BUTTON2: RAIL", x, y+20);
	fb_print_string_transparent(LU_DARK_GREY, "BUTTON2: GUN", x, y+30);
	fb_print_string_transparent(LU_DARK_GREY, "LEDS:", x, y+45);
	fb_print_string_transparent(LU_DARK_GREY, "  WEAPON'S CHARGE", x, y+55);
	fb_print_string_transparent(LU_DARK_GREY, "TOUCHSCREEN:", x, y+70);
	fb_print_string_transparent(LU_DARK_GREY, "	 MOVE SHIP", x, y+80);
}

static int check_button(button_t button, uint16_t x, uint16_t y){
	if(x >= button.x && x <= button.x + button.width &&
	   y >= button.y && y <= button.y + button.height){
		return 0;
	}
	return -1;
}
