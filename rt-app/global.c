/*!
 * \file global.c
 * \brief Fichier (body) pour les define, variables et types persos globaux
 * \author Romain Maffina
 * \version 0.1
 * \date decembre 2012
 */

#include "global.h"
#include "hit_task.h"
#include "invaders_task.h"

difficulty_t difficulty = NORMAL;

uint8_t game_over = 0;

uint32_t game_points = 0;
uint32_t game_bullet_kill = 0;
uint32_t game_bullet_used = 0;

uint8_t game_break = 1;
uint8_t game_started = 0;
uint8_t game_level_up = 0;

uint8_t screen_pressed = 0;
uint16_t screen_x = 0;
uint16_t screen_y = 0;

