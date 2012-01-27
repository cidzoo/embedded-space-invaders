/*!
 * \file bitmap.h
 * \brief Fichier (header) pour le stockage des tableaux de bitmap graphiques
 * \author Romain Maffina
 * \version 0.1
 * \date 21 janvier 2012
 *
 * Fichier (header) pour le stockage des tableaux de bitmap graphiques.
 * Ces tableaux de pixels ont étés créé de la façon suivante :
 * 	- Choix d'une image, peu importe le format
 * 	- On l'ouvre dans Gimp, on la modifie comme voulu puis on l'enregistre
 * 	au format bitmap en RGB565.
 * 	- Puis grâce à hexdump le contenu de la matrice de pixel est récupérée,
 * 	par exemple la ligne suivante a été utilisée pour l'invader de 16x16:
 * 		hexdump -s 0x46 -v -e '"{" 16/2 "0x%04X " "},\n"' invader.bmp
* 		-s permet d'indiquer l'offset depuis le début du fichier (dans notre
* 	cas l'endroit ou commence la matrice de pixel)
* 		-e permet de mettre en forme selon la structure précisée juste derrière
* 	- les lignes obtenues sont du type : "{0x0000 0xEF5F 0xEF5F 0x0000},"
* 	il ne reste plus qu'à séparer chaque groupe par une virgule (find&remplace)
*
 */
#ifndef BITMAPS_H_
#define BITMAPS_H_

#include <linux/types.h>
#include "lcdlib.h"

//! Bitmap sizes

#define BOMB_WIDTH 8 	/**< Bomb bitmap width */
#define BOMB_HEIGHT 8	/**< Bomb bitmap height */

#define GUN_WIDTH 4		/**< Gun bitmap width */
#define GUN_HEIGHT 5	/**< Gun bitmap height */

#define RAIL_WIDTH 6	/**< Rail bitmap width */
#define RAIL_HEIGHT LCD_MAX_Y /**< Rail bitmap height */

#define ROCKET_WIDTH 10	/**< Rocket bitmap width */
#define ROCKET_HEIGHT 14 /**< Rocket bitmap height */

#define WAVE_WIDTH LCD_MAX_X /**< Wave bitmap width */
#define WAVE_HEIGHT 4	/**< Wave bitmap height */

#define INVADER_WIDTH 16 /**< Invader bitmap width */
#define INVADER_HEIGHT 16	/**< Invader bitmap height */

#define SHIP_WIDTH 32	/**< Ship bitmap width */
#define SHIP_HEIGHT 32	/**< Ship bitmap height */

#define INVADER_MENU_WIDTH	136	/**< Menu bitmap width */
#define INVADER_MENU_HEIGHT	99 	/**< Menu bitmap height */

extern uint16_t bmp_bomb[BOMB_HEIGHT][BOMB_WIDTH];
extern uint16_t bmp_gun[GUN_HEIGHT][GUN_WIDTH];
extern uint16_t bmp_rail[RAIL_HEIGHT][RAIL_WIDTH];
extern uint16_t bmp_rocket[ROCKET_HEIGHT][ROCKET_WIDTH];
extern uint16_t bmp_wave[WAVE_HEIGHT][WAVE_WIDTH];
extern uint16_t bmp_invader[INVADER_HEIGHT][INVADER_WIDTH];
extern uint16_t bmp_ship[SHIP_HEIGHT][SHIP_WIDTH];
extern uint16_t bmp_invader_menu1[INVADER_MENU_HEIGHT][INVADER_MENU_WIDTH];
extern uint16_t bmp_invader_menu2[INVADER_MENU_HEIGHT][INVADER_MENU_WIDTH];

#endif /* BITMAPS_H_ */
