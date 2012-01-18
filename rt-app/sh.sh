rmmod rt-app.ko
rmmod pca9554.ko

tftp -g -r pca9554.ko 10.0.0.1
tftp -g -r rt-app.ko 10.0.0.1

insmod pca9554.ko
insmod rt-app.ko
