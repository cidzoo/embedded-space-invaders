/*------------------------------------------------------------------------
 * LCD control routines
 *
 * Author            : Michel Starkier, Lionel Sambuc
 * Date              : 16.10.2007
 * Version           : 1.1
 * Updates			 : 01.12.2011
 * 						Moved font definition to lcdfont.h, deleted pong-related
 * 						functions and definitions, completed code documentation.
 *
 *-----------------------------------------------------------------------------*/

#define LCD_MAX_X  240
#define LCD_MAX_Y  320

#define RED(a_red)		(((a_red)   & 0x1f)<<11)
#define GREEN(a_green)	(((a_green) & 0x3f)<< 5)
#define BLUE(a_blue)	(((a_blue)  & 0x1f)<< 0)

typedef enum {
	false = 0, true = 1
} bool;

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
 * LCD screen
 *
 * @param fg_color the foreground color of the pixel
 * @param bg_color the background color of the pixel
 * @param c the character to print
 * @param y the y coordinate of the character
 * @param x the x coordinate of the character
 */
void fb_print_char(int fg_color, int bg_color, unsigned char c, int x, int y);

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
