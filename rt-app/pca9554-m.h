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

/*! \brief Fonction pour activer localement une led
 *  \param led_num Numéro de 0 à 3 représentant la led à activer localement.
 *  \return Résultat
 *  	\retval 0 Numéro de led correct
 *  	\retval -1 Numéro de led incorrect
 *
 *  Les leds débutent de bas en haut (le numéros sur la board ne correspondent pas)
 */
extern ssize_t pca9554_en_led(uint8_t led_num);

/*! \brief Fonction pour désactiver localement une led
 *  \param led_num Numéro de 0 à 3 représentant la led à désactiver localement.
 *  \return Résultat
 *  	\retval 0 Numéro de led correct
 *  	\retval -1 Numéro de led incorrect
 *
 *  Les leds débutent de bas en haut (le numéros sur la board ne correspondent pas)
 */
extern ssize_t pca9954_dis_led(uint8_t led_num);

/*! \brief Fonction pour récupérer la valeur locale des switchs
 *  \param switch_num Numéro de 0 à 3 représentant le switch dont on veut récupérer la valeur
 *  \param switch_val 0 ou 1 pour représenter la valeur du switch
 *  \return Résultat
 *  	\retval 0 Numéro de switch correct
 *  	\retval -1 Numéro de switch incorrect
 *
 *  Les switches débutent de bas en haut (le numéros sur la board ne correspondent pas)
 */
extern ssize_t pca9554_get_switch(uint8_t switch_num, uint8_t *switch_val);

/*! \brief Fonction pour envoyer la valeur locale des leds vers le pca9554
 *  \return Résultat
 *  	\retval 0 Succès lors de l'écriture
 *  	\retval -1 Erreur lors de l'écriture
 */
extern ssize_t pca9554_send(void);

/*! \brief Fonction pour recevoir la valeur des switches depuis le pca9554 en local
 *  \return Résultat
 *  	\retval 0 Succès lors de la lecture
 *  	\retval -1 Erreur lors de la lecture
 */
extern ssize_t pca9554_receive(void);
