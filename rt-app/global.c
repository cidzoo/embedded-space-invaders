/*
 * global.c
 *
 *  Created on: Dec 21, 2011
 *      Author: romain
 */

/* Functions to manipulate the list of bullet */
void addBullet(Bullet *b){
	int i=0;
	//find the first empty slot and place the bullet there
	for(i=0;i<NB_MAX_BULLETS;i++){
		if(bullets[i] == NULL){
			bullets[i] = b;
			break;
		}
	}
}

void removeBullet(Bullet *b){
	int i=0;
	//find the bullet to delete
	for(i=0;i<NB_MAX_BULLETS;i++){
		if(bullets[i] == b){
			bullets[i] = NULL;
			break;
		}
	}
}

//TODO : ask to the professor what's the best between thoses two versions
//void addBullet(Bullet *b){
//	ListBullets *new = (ListBullets*)malloc(sizeof(ListBullets));
//	new->bullet = b;
//	new->next = headListBullets;
//	headListBullets = new;
//}
//
//void removeBullet(Bullet *b){
//	ListBullets *current = headListBullets;
//	while(current != NULL){
//		if(current->next==b){
//			ListBullets *next = current->next;
//			current->next = next->next;
//			free(next);
//		}
//		current = current->next;
//	}
//}
