/*
 * collisions-and-shots.c
 *
 *  Created on: Dec 21, 2011
 *      Author: romain
 */

/* Functions for collisions and shots */
void fire_weapon(Weapon w){
	uint16_t startX, startY;
	//Control that the weapon selected can be used
	if(w->ready){

		//Grap the position of the spaceship's gun's top
		startX = ss->x + ss->width/2;
		startY = ss->y-1;

		//Fire the weapon (create the bullet)
		Bullet b;

		switch(w->type){
		case GUN:
			b->x = startX-2/2; // width/2
			b->y = startY-4; //-height
			b->width = 2;
			b->height = 4;
			break;
		case RAIL:
			b->x = startX-2/2; // width/2
			b->y = 0;
			b->width = 2;
			b->height = LCD_HEIGHT-startY;
			break;
		case ROCKET:
			b->x = startX-4/2; // width/2
			b->y = startY-8; //-height
			b->width = 4;
			b->height = 8;
			break;
		case WAVE:
			b->x = 0; // width/2
			b->y = startY-2;
			b->width = LCD_WIDTH;
			b->height = 2;
			break;
		}

		//add it to the list of current bullets
		addNewBullet(b);

	}//end ready
}

void checkForCollisions(){

	//for each remaining invader : control that no bullet is touching it
	for(i=0;i<nbBullets;i++){
		//for each corner of the bullet's hitbox :
			//Control if this corner is touching invader's hitbox like :
			//bulletCornerX > invaderLeftCorner && bulletCornerX < invaderRightCorner
			//...
				//if so : damage the invader and delete the bullet
	}

	//control that the spaceship is not been touched by a invader's bomb
}


