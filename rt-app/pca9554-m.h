// Prototype des différentes fonctions qui gère la communication avec le driver.
int pca9554_open (struct inode *, struct file *);
int pca9554_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
int pca9554_close(struct inode *, struct file *);
ssize_t pca9554_write(struct file *, const char __user *, size_t, loff_t *);
ssize_t pca9554_read(struct file *, char __user *, size_t, loff_t *);
