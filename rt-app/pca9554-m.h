// Type enuméré pour connaitre le type de l'IO
typedef enum{
	io_led = 0,
	io_switch = 1
} pca_io_type_t;

// Type structure pour passer les données de l'IO
typedef struct{
	pca_io_type_t 	io_type;	/*!< Type de l'io pour la demande */
	uint8_t			io_num;		/*!< Numéro de l'io pour la demande */
	int				value;		/*!< Valeur d'entrée/retour en fonction de io_type */
} pca_io_data_t;

// Prototype des différentes fonctions qui gère la communication avec le driver.
//int pca9554_open (struct inode *, struct file *);
//int pca9554_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
//int pca9554_close(struct inode *, struct file *);

// Fonctions prototype pour les accès KERNEL
ssize_t pca9554_get(pca_io_data_t *io_data);
ssize_t pca9554_set(pca_io_data_t *io_data);
