/*!
 * \file lcdlib.h
 * \brief Fichier (header) pour la gestion du lcd
 * \author Michel Starkier, Lionel Sambuc, Yannick Lanz
 * \version 1.2 (fork)
 * \date 17 janvier 2012
 *
 * Fichier (header) pour la gestion du lcd.
 * Comprend les entêtes de fonction pour la manipulation
 * du lcd comme le tracage de ligne, de rectangle, de cercle, etc.
 */
#ifndef LCDLIB_H_
#define LCDLIB_H_

#include <linux/types.h>

#define LCD_MAX_X  240
#define LCD_MAX_Y  320

#define RED(a_red)		(((a_red)   & 0x1f)<<11)
#define GREEN(a_green)	(((a_green) & 0x3f)<< 5)
#define BLUE(a_blue)	(((a_blue)  & 0x1f)<< 0)

// Variable pour représentant la 1ere adresse du tableau pour le double buffering
extern void *fb_mem_rt;

typedef enum {
	false = 0, true = 1
} bool;

/*!
 * \struct progress_bar_t
 * \brief Objet représentant une progress bar.
 *
 * progress_bar_t représente une progress bar. Il contient sa
 * position (x, y), sa taille (width, height), sa couleur ainsi
 * que les valeur courante et max de la progress bar.
 */
typedef struct{
	uint16_t x;				/*!< Position en y de la progress bar */
	uint16_t y;				/*!< Position en x de la progress bar */
	uint16_t width;			/*!< Largeur de la progress bar */
	uint16_t height;		/*!< Hauteur de la progress bar */
	int couleur;			/*!< Couleur de la progress bar */
	uint16_t current_value;	/*!< Valeur courante de la progress bar */
	uint16_t max_value;		/*!< Valeur maximum de la progress bar */
} progress_bar_t;

/*!
 * \struct button_t
 * \brief Objet représentant un bouton.
 *
 * button_t représente un bouton. Il contient sa
 * position (x, y), sa taille (width, height) ainsi que son label.
 */
typedef struct{
	uint16_t x;				/*!< Position en x du bouton */
	uint16_t y;				/*!< Position en y du bouton */
	uint16_t width;			/*!< Largeur du bouton */
	uint16_t height;		/*!< Hauteur du bouton */
	char *label;			/*!< label du bouton */
} button_t;

/**
 * Get a 32-bit (unsigned) random number
 * @return a 32 bits random value (range 0x00000000-0xffffffff)
 */
unsigned int get_random(void);

/**
 * Initialize the i.MX21 framebuffer.
 */
void fb_init(void);

/**
 * Draw the double buffering buffer to the lcd
 */
void fb_display(void);

/**
 * Draw a progress bar
 */
void fb_progress_bar(progress_bar_t pb);

/**
 * Draw a button
 */
void fb_button(button_t button);

/**
 * Draw the line delimited of the point (x_min:y_min) and (x_max:y_max) with
 * the given color.
 *
 * @param x0 the x coordinate of the first point
 * @param y0 the y coordinate of the first point
 * @param x1 the x coordinate of the second point
 * @param y1 the y coordinate of the second point
 * @param color the color to use to fill the line
 */
void fb_line(int x0, int y0, int x1, int y1, int color);

/**
 * Draw the borders of the rectangle delimited by y_min, y_max, x_min and x_max with
 * the given color.
 *
 * @param y_min the y coordinate of the upper edge
 * @param y_max the y coordinate of the lower edge
 * @param x_min the x coordinate of the left edge
 * @param x_max the x coordinate of the right edge
 * @param color the color to use to draw the borders of the rectangle
 */
void fb_rect(int y_min, int y_max, int x_min, int x_max, int color);

/**
 * Fill the rectangle delimited by y_min, y_max, x_min and x_max with
 * the given color.
 *
 * @param y_min the y coordinate of the upper edge
 * @param y_max the y coordinate of the lower edge
 * @param x_min the x coordinate of the left edge
 * @param x_max the x coordinate of the right edge
 * @param color the color to use to fill the rectangle
 */
void fb_rect_fill(int y_min, int y_max, int x_min, int x_max, int color);

/**
 * Fill a circle with the color color, centered in (y,x) and of radius
 * radius.
 *
 * @param y the y coordinate of the center of the circle
 * @param x the x coordinate of the center of the circle
 * @param radius the radius of the circle
 * @param color the color of the circle
 */
int fb_circle_fill(int y, int x, int radius, int color);

/**
 * Draw a circle with the color color, centered in (y,x) and of radius
 * radius.
 *
 * @param y the y coordinate of the center of the circle
 * @param x the x coordinate of the center of the circle
 * @param radius the radius of the circle
 * @param color the color of the circle
 */
int fb_circle(int y, int x, int radius, int color);

/**
 * Set the pixel (y, x) to the color color.
 *
 * @param y the y coordinate of the pixel
 * @param x the x coordinate of the pixel
 * @param color the color of the pixel
 */
void fb_set_pixel(int y, int x, int color);

/**
 * Print the character c at the coordinates (x, y) with the given colors on the
 * LCD screen.
 *
 * @param fg_color the foreground color of the pixel
 * @param bg_color the background color of the pixel
 * @param c the character to print
 * @param y the y coordinate of the character
 * @param x the x coordinate of the character
 */
void fb_print_char(int fg_color, int bg_color, unsigned char c, int x, int y);

/**
 * Print the character c at the coordinates (x, y) with the given colors on the
 * LCD screen but without background color.
 *
 * @param fg_color the foreground color of the pixel
 * @param c the character to print
 * @param y the y coordinate of the character
 * @param x the x coordinate of the character
 */
void fb_print_char_transparent(int fg_color, unsigned char c, int x, int y);

/**
 * Print the string str at the coordinates (x, y) with the given colors on the
 * LCD screen.
 *
 * This function automatically wraps to the next line if the string is too wide
 * for the screen and respects embedded line feeds characters (\n).
 *
 * @param fg_color the foreground color of the pixel
 * @param bg_color the background color of the pixel
 * @param str the string to print
 * @param y the y coordinate of the character
 * @param x the x coordinate of the character
 */
void fb_print_string(int fg_color, int bg_color, unsigned char *str, int x, int y);

/**
 * Print the string str at the coordinates (x, y) with the given colors on the
 * LCD screen but without background color.
 *
 * This function automatically wraps to the next line if the string is too wide
 * for the screen and respects embedded line feeds characters (\n).
 *
 * @param fg_color the foreground color of the pixel
 * @param str the string to print
 * @param y the y coordinate of the character
 * @param x the x coordinate of the character
 */
void fb_print_string_transparent(int fg_color, unsigned char *str, int x, int y);

#endif /* LCDLIB_H_ */
