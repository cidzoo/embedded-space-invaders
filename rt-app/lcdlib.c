/*------------------------------------------------------------------------
* But               : fonctions d'affichage pour le LCD i.MX21
*
* Auteurs           : Michel Starkier, Cédric Bardet, Daniel Rossier
* Date              : 16.10.2007
* Version           : 1.0
*
* Fichier           : lcdlib.c
*-----------------------------------------------------------------------------*/

#include "lcdlib.h"
#include "lcdfont.h"
#include <linux/fb.h>

unsigned char *fb_mem = NULL;
static unsigned int Xn = 7391; /* Initial random seed */

void fb_set_pixel(int y_min,int x_min,int couleur);
int circleYpos(int cXpos, int radius);

/*
  Initialize the framebuffer, in other words get a virtual address at the first pixel of the LCD
*/
void fb_init(void) {

  int fbidx = 0; /*iminor(file->f_dentry->d_inode);*/
  struct fb_info *info = registered_fb[fbidx];  /* This is required to get the physical address and length */

  fb_mem = ioremap(info->fix.smem_start, PAGE_ALIGN((info->fix.smem_start & ~PAGE_MASK) + info->fix.smem_len));

  printk("framebuffer configured OK.\n");
}

/*
  Generate a random 32-bit numbers
*/
unsigned int get_random(void) {
  static unsigned int Xnp1, a, c, m;

  /* Initializing the random generator */
  a = 13;
  c = 7;
  m = 0xFFFFFFFF;

  Xnp1 = (a * Xn + c) % m;

  Xn = Xnp1;

  return Xn;
}

void fb_set_pixel(int y, int x, int couleur) {
  *((unsigned short int*)(fb_mem + 2*x + y*480)) = couleur;
}

void fb_rect_fill(int y_min, int y_max, int x_min, int x_max, int couleur) {

  int x = 0, y = 0;

  for (y = y_min; y <= y_max; y++)
    for (x = x_min; x <= x_max; x++)
      fb_set_pixel(y, x, couleur);
}

/******************************************/

/* calcule la coordonnee y d'un quart de cercle en fonction de x
   et du rayon => x et y positifs				*/

int mySqrt(int sq){
   int an,i,an1;
   an=sq;

   if (sq == 0)
     return 0;
   for (i=0;i<6;i++) {
	 an1=(an+sq/an)/2;
	 an=an1;
   }
   return (an);
}

int circleYpos(int cXpos, int radius) {
   return mySqrt(radius*radius - cXpos*cXpos);
}

/* remplit un cercle en fonction des coordonn�es du centre, du rayon,
   et de la couleur 							*/
int fb_circle_fill(int yPos,int xPos,int radius,int color){
   int i,j,y;
   if (radius >= LCD_MAX_X/2) radius = LCD_MAX_X/2;
   if (xPos+radius >= LCD_MAX_X) xPos = LCD_MAX_X-1-radius;
   if (yPos+radius >= LCD_MAX_Y) yPos = LCD_MAX_Y-1-radius;
   if (xPos-radius < 0 ) xPos = radius;
   if (yPos-radius < 0 ) yPos = radius;
   for (i=0;i<=radius;i++){
      y = circleYpos( i, radius);             /* calcul pixel p�riph�rie */
      for (j=0;j<=y;j++) {                     /* remplissage */
         fb_set_pixel(yPos+j,xPos+i,color);
         fb_set_pixel(yPos-j,xPos+i,color);
         fb_set_pixel(yPos+j,xPos-i,color);
         fb_set_pixel(yPos-j,xPos-i,color);
      }
   }
 return 0;
}

/* Dessine un cercle en fonction des coordonn�es du centre, du rayon,
   et de la couleur 							*/
int fb_circle(int yPos, int xPos, int radius, int color) {
	int i, y;
	int a, b, c, d;
	if (radius >= LCD_MAX_X / 2)
		radius = LCD_MAX_X / 2;

	for (i = 0; i <= radius; i++) {
		y = circleYpos(i, radius); /* calcul pixel p�riph�rie */

		a = (yPos > LCD_MAX_Y) ? LCD_MAX_Y : yPos;
		b = (yPos < 0) ? 0 : yPos;
		c = (xPos + i > LCD_MAX_X) ? LCD_MAX_X : xPos + i;
		d = (xPos - i < 0) ? 0 : xPos - i;
		fb_set_pixel(a, c, color);
		fb_set_pixel(b, c, color);
		fb_set_pixel(a, d, color);
		fb_set_pixel(b, d, color);

		a = (yPos + y > LCD_MAX_Y) ? LCD_MAX_Y : yPos + y;
		b = (yPos - y < 0) ? 0 : yPos - y;
		c = (xPos + i > LCD_MAX_X) ? LCD_MAX_X : xPos + i;
		d = (xPos - i < 0) ? 0 : xPos - i;
		fb_set_pixel(a, c, color);
		fb_set_pixel(b, c, color);
		fb_set_pixel(a, d, color);
		fb_set_pixel(b, d, color);
	}
	return 0;
}

/*---------------------------------------------------------------------------*-
   fb_print_char
  -----------------------------------------------------------------------------
   Descriptif: Cette fonction permet d'afficher un caractère en fonction
   	   	   	   de la couleur et de la position choisie par l'utilisateur.
   Entrée    : int color 			: couleur choisie
   	   	   	   int color_fond 		: couleur de fond
   	   	   	   unsigned char car 	: caractère choisi
   	   	   	   int x 				: position en x sur l'afficheur
   	   	   	   int y 				: position en y sur l'afficheur
   Sortie    : Aucune
-*---------------------------------------------------------------------------*/
void fb_print_char(int color, int color_fond, unsigned char car, int x, int y)
{
	unsigned char ligne;
	unsigned int j;
	unsigned int i;
	unsigned int tab[] = { 1, 2, 4, 8, 16, 32, 64, 128};

	// Permet de parcourir les 8 lignes
	for (i = 0; i < 8 ; i++)
	{
		// Permet de récuperer la valeur codé sur 8 bits
		// pour le caractère.
		ligne = fb_font_data[car-FIRST_CHAR][i];

		// Permet de parcourir les 8 colonnes
		for (j = 0 ; j < 8; j++)
		{
			// Permet de déterminer si on doit allumer le pixel cocnerné
			if (ligne & tab[7-j])
				fb_set_pixel(y+i,x+j,color);
			else
				fb_set_pixel(y+i,x+j,color_fond);
		}
	}
}

/*---------------------------------------------------------------------------*-
   fb_print_string
  -----------------------------------------------------------------------------
   Descriptif: Cette fonction permet d'afficher un texte (une chaîne de caractère)
   	   	   	   sur l'afficheur LCD. La fonction gère automatiquement le retour
   	   	   	   à la ligne si le texte est trop long ou si le caractère '\n' est
   	   	   	   présent.

   Remarque  : Il faut faire attention car aucun contrôle de dépassement du
    		   texte en bas de l'écran est effectué

   Entrée    : int color 					: couleur choisie.
   	   	   	   int color_fond 				: couleur de fond
      	   	   unsigned char *ptr_texte 	: texte à afficher
   	   	   	   int x 						: position de départ en x sur l'afficheur
   	   	   	   int y 						: position de déaprt en y sur l'afficheur
   Sortie    : Aucune
-*---------------------------------------------------------------------------*/
void fb_print_string(int color, int color_fond, unsigned char *ptr_texte, int x, int y)
{
	unsigned char pos_car = 0;			// Position du caractère sur l'afficheur.
	unsigned char pos_car_ligne = 0;	// Position d'un caractère au sein d'une ligne de l'afficheur.

	// Tant que la chaîne de caractère n'a pas été complétement parcourue
	while(*(pos_car + ptr_texte) != '\0')
	{
		// Si le caractère de retour à la ligne est présent
		if(*(pos_car + ptr_texte) == '\n')
		{
			pos_car_ligne = 0;
			x = 0;
			y += 10;
			pos_car++;
			continue;
		}
		// Si le nombre maximum de caractère est présent sur la ligne (30 caractères)?
		else if(((pos_car_ligne % 30) == 0) && pos_car_ligne)
		{
			pos_car_ligne = 0;
			x = 0;
			y += 10;
		}

		// Affichage du caractère sur l'afficheur LCD
		fb_print_char(color,color_fond, *(ptr_texte + pos_car), x + (8*pos_car_ligne), y);
		pos_car++;
		pos_car_ligne++;
	}
}
