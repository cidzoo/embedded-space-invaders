/*!
 * \file pca9554.h
 * \brief Fichier (header) pour le module de gestion du PCA9554
 * \author Yannick Lanz
 * \version 0.1
 * \date 17 janvier 2012
 *
 * Fichier (header) principal pour le module du PCA9554.
 * Le PCA9554 est un io expander fonctionnant sur le bus I2C.
 * Ce module utilise directement le driver I2C de linux et
 * permet de lire les switchs ainsi que de définir les leds de la board
 * utilisée.
 */
/*! Fonctions prototype pour les accès KERNEL */

extern ssize_t pca9554_en_led(uint8_t led_num);
extern ssize_t pca9954_dis_led(uint8_t led_num);
extern ssize_t pca9554_get_switch(uint8_t switch_num, uint8_t *switch_val);
extern ssize_t pca9554_send(void);
extern ssize_t pca9554_receive(void);
