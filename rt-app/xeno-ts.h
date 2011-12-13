
/*
 * xeno-ts.h - Definition of prototypes for the i2c driver.
 * 		To be included in every file where these functions are called.
 */

#include <linux/time.h>

struct ts_sample {
	int				x;
	int				y;
	unsigned int	pressure;
	struct timeval	tv;
};

int xeno_ts_init(void);
void xeno_ts_exit(void);
int xeno_ts_read(struct ts_sample *samp, int nr, int f_flags);


