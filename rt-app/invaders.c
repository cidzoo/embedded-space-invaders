/*
 * enemies.c
 *
 *  Created on: Dec 22, 2011
 *      Author: michael
 */

 #include "global.h"
 #include "lcdlib.h"


 void init_invaders(Invader *listInvaders){

	 int i;
	 uint8_t width_invader = 35;
	 uint8_t height_invader = 35;
	 uint8_t hp_invader = 10;

	 uint8_t space_between_invader=20;
	 int nbMaxInvadersPerLine=0;
	 int nbInvadersPerLine;
	 int nbInvaders=7;

	 int wave_row=1;

	 //calculate the max number of invaders per lines

	 do
	 {

		 nbMaxInvadersPerLine++;

	 }while ((LCD_MAX_X-(nbMaxInvadersPerLine*width_invader)-(nbMaxInvadersPerLine*space_between_invader))>0);




	 for (i=0;i<nbInvaders;i++){
		 listInvaders[i].hp=hp_invader;
		 listInvaders[i].height=height_invader;
		 listInvaders[i].width=width_invader;

		 if((nbInvaders-nbMaxInvadersPerLine)<nbMaxInvadersPerLine)
			 wave_row++;

		 listInvaders[i].x= space_between_invader+(i*space_between_invader)+(i*width_invader);
		 listInvaders[i].y= 50 + ((wave_row-1)*(height_invader+50));
	 }



 }

 void move_invaders(Invader *listInvaders){




 }


 //retourne les 4 coins de l'espace de la wave
 void get_invaders_hitbox(*dimension){


 }


