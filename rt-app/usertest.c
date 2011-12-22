#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>


int main(int argc, char **argv) {

	int	fd_sw[4], fd_led[4];
	char old_val_sw[4], new_val_sw[4];

	char	buf[128];
	int i;

	// Ouverture les leds en ecriture
	printf("Opening leds:\n");
	for(i = 0; i < 4; i++){
		printf("  led[%d]: ", i);
		sprintf(buf, "/var/dev/io_expander/leds/%d", i);
		fd_led[i] = open(buf, O_WRONLY);
		if(fd_led[i] < 0){
			printf("error\n");
			return -1;
		}else{
			printf("done\n");
		}
	}

	// Ouverture des switches en lecture.
	printf("Opening switches:\n");
	for(i = 0; i < 4; i++){
		printf("  switches[%d]: ", i);
		sprintf(buf, "/var/dev/io_expander/switches/%d", i);
		fd_sw[i] = open(buf, O_RDONLY);
		if(fd_sw[i] < 0){
			printf("error\n");
			return -1;
		}else{
			printf("done\n");
		}

		old_val_sw[i] = 0;
		new_val_sw[i] = 0;
	}

	// Boucle principal
	printf("Boucle principale\n");
	char modif;
	while(1){
		modif = 0;
		for(i = 0; i < 4; i++){
			read(fd_sw[i], &new_val_sw[i], 1);
			if(old_val_sw[i] != new_val_sw[i]){
				if(new_val_sw[i] == 1){
					printf("sw[%d] pressed\n", i);
					// On s'occupe des leds
					write(fd_led[i], &new_val_sw[i], 1);
					printf("led[%d] turned on\n", i);
				}else{
					printf("sw[%d] released\n", i);
					// On s'occupe des leds
					write(fd_led[i], &new_val_sw[i], 1);
					printf("led[%d] turned off\n", i);
				}
				modif = 1;
			}
		}
		for(i = 0; i < 4; i++){
			if(old_val_sw[i] == new_val_sw[i] && modif && new_val_sw[i] == 1){
				printf("sw[%d] hold\n", i);
			}
			old_val_sw[i] = new_val_sw[i];
		}
		usleep(500);
	}

	/*printf("Opening switches:\n");



	if(argc >= 2){
		fd = open(argv[1], O_RDWR);
	}else{
		fd = open("/var/dev/io_expander/switches/0", O_RDWR);
	}

	if(fd < 0)
	{
		printf("Erreur à l'ouverture\n");
		return -1;
	}

	printf("reading...\n");
	while(1){
		read(fd, buffer, 128);
		if(read(fd, buffer, 128) == 1){
			printf("ok\n");
		}else{
			printf("nok\n");
		}
		int val = buffer[0];
		printf("Val: %d\n", buffer[0]);

		usleep(500);
	}


	printf("end.\n");*/

	return(0);
}
