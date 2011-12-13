/*
    xeno_ts-lib.c - Touchscreen lib.

    (c) 2009 D. Rossier/C. Bardet, REDS Institute, HEIG-VD

*/

#include <linux/module.h>

#include "xeno-ts.h"

/** pthres **/
struct tslib_pthres {
	//struct tslib_module_info module;
	unsigned int	pmin;
	unsigned int	pmax;
} pthres;
struct tslib_pthres *p = &pthres; /* Compatibility workaround (DRE) */

/** variance **/
struct tslib_variance {
	//struct tslib_module_info module;
	int delta;
        struct ts_sample last;
        struct ts_sample noise;
	unsigned int flags;
#define VAR_PENDOWN		0x00000001
#define VAR_LASTVALID		0x00000002
#define VAR_NOISEVALID		0x00000004
#define VAR_SUBMITNOISE		0x00000008
} variance;
struct tslib_variance *var = &variance; /* Workaround (DRE) */

/** dejitter **/

/**
 * This filter works as follows: we keep track of latest N samples,
 * and average them with certain weights. The oldest samples have the
 * least weight and the most recent samples have the most weight.
 * This helps remove the jitter and at the same time doesn't influence
 * responsivity because for each input sample we generate one output
 * sample; pen movement becomes just somehow more smooth.
 */

#define NR_SAMPHISTLEN	4

/* To keep things simple (avoiding division) we ensure that
 * SUM(weight) = power-of-two. Also we must know how to approximate
 * measurements when we have less than NR_SAMPHISTLEN samples.
 */
static const unsigned char weight [NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
{
	/* The last element is pow2(SUM(0..3)) */
	{ 5, 3, 0, 0, 3 },	/* When we have 2 samples ... */
	{ 8, 5, 3, 0, 4 },	/* When we have 3 samples ... */
	{ 6, 4, 3, 3, 4 },	/* When we have 4 samples ... */
};

struct ts_hist {
	int x;
	int y;
	unsigned int p;
};

struct tslib_dejitter {
//	struct tslib_module_info module;
	int delta;
	int x;
	int y;
	int down;
	int nr;
	int head;
	struct ts_hist hist[NR_SAMPHISTLEN];
} dejitter;

struct tslib_dejitter *djt = &dejitter; /* Workaround */

/** linear **/
struct tslib_linear {
	//struct tslib_module_info module;
	int	swap_xy;

// Linear scaling and offset parameters for pressure
	int	p_offset;
	int	p_mult;
	int	p_div;

// Linear scaling and offset parameters for x,y (can include rotation)
	int	a[7];
} linear;
struct tslib_linear *lin = &linear; /* Workaround */

 /**************************************/

/** pthres **/
extern int xeno_input_read(struct ts_sample *samp, int nr, int f_flags);
static int pthres_read(struct ts_sample *samp, int nr, int f_flags)
{
	//struct tslib_pthres *p = (struct tslib_pthres *)info;
	int ret;
	static int xsave = 0, ysave = 0;
	static int press = 0;

	//ret = info->next->ops->read(info->next, samp, nr);
	ret = xeno_input_read(samp, nr, f_flags);
	if (ret >= 0) {
		int nr = 0, i;
		struct ts_sample *s;

		for (s = samp, i = 0; i < ret; i++, s++) {
			if (s->pressure < p->pmin) {
				if (press != 0) {
					/* release */
					press = 0;
					s->pressure = 0;
					s->x = xsave;
					s->y = ysave;
				} else {
					/* release with no press, outside bounds, dropping */
					int left = ret - nr - 1;
					if (left > 0) {
						memmove(s, s + 1, left * sizeof(struct ts_sample));
						s--;
						continue;
					}
					break;
				}
			} else {
				if (s->pressure > p->pmax) {
					/* pressure outside bounds, dropping */
					int left = ret - nr - 1;
					if (left > 0) {
						memmove(s, s + 1, left * sizeof(struct ts_sample));
						s--;
						continue;
					}
					break;
				}
				/* press */
				press = 1;
				xsave = s->x;
				ysave = s->y;
			}
			nr++;
		}
		return nr;
	}
	return ret;
}

void pthres_init(void) {

	printk("Initializing pthres function for touchscreen.\n");

	p->pmin = 1;
	p->pmax = INT_MAX;
}
EXPORT_SYMBOL(pthres_init);

/** variance **/
static int sqr (int x)
{
	return x * x;
}

int variance_read(struct ts_sample *samp, int nr, int f_flags)
{
	//struct tslib_variance *var = (struct tslib_variance *)info;
	struct ts_sample cur;
	int count = 0, dist;

	while (count < nr) {
		if (var->flags & VAR_SUBMITNOISE) {
			cur = var->noise;
			var->flags &= ~VAR_SUBMITNOISE;
		} else {
			if (pthres_read(&cur, 1, f_flags) < 1)
				return count;
		}

		if (cur.pressure == 0) {
			/* Flush the queue immediately when the pen is just
			 * released, otherwise the previous layer will
			 * get the pen up notification too late. This
			 * will happen if info->next->ops->read() blocks.
			 */
			if (var->flags & VAR_PENDOWN) {
				var->flags |= VAR_SUBMITNOISE;
				var->noise = cur;
			}
			/* Reset the state machine on pen up events. */
			var->flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);
			goto acceptsample;
		} else
			var->flags |= VAR_PENDOWN;

		if (!(var->flags & VAR_LASTVALID)) {
			var->last = cur;
			var->flags |= VAR_LASTVALID;
			continue;
		}

		if (var->flags & VAR_PENDOWN) {
			/* Compute the distance between last sample and current */
			dist = sqr (cur.x - var->last.x) +
			       sqr (cur.y - var->last.y);

			if (dist > var->delta) {
				/* Do we suspect the previous sample was a noise? */
				if (var->flags & VAR_NOISEVALID) {
					/* Two "noises": it's just a quick pen movement */
					samp [count++] = var->last = var->noise;
					var->flags = (var->flags & ~VAR_NOISEVALID) |
						VAR_SUBMITNOISE;
				} else
					var->flags |= VAR_NOISEVALID;

				/* The pen jumped too far, maybe it's a noise ... */
				var->noise = cur;
				continue;
			} else
				var->flags &= ~VAR_NOISEVALID;
		}

acceptsample:
#ifdef DEBUG
		fprintf(stderr,"VARIANCE----------------> %d %d %d\n",
			var->last.x, var->last.y, var->last.pressure);
#endif
		samp [count++] = var->last;
		var->last = cur;
	}

	return count;
}

void variance_init(void) {

	printk("Initializing variance function for touchscreen.\n");

	var->delta = 30;
	var->flags = 0;
    var->delta = sqr (var->delta);

	pthres_init();
}

/** dejitter **/
static void average (struct tslib_dejitter *djt, struct ts_sample *samp)
{
	const unsigned char *w;
	int sn = djt->head;
	int i, x = 0, y = 0;
	unsigned int p = 0;

        w = weight [djt->nr - 2];

	for (i = 0; i < djt->nr; i++) {
		x += djt->hist [sn].x * w [i];
		y += djt->hist [sn].y * w [i];
		p += djt->hist [sn].p * w [i];
		sn = (sn - 1) & (NR_SAMPHISTLEN - 1);
	}

	samp->x = x >> w [NR_SAMPHISTLEN];
	samp->y = y >> w [NR_SAMPHISTLEN];
	samp->pressure = p >> w [NR_SAMPHISTLEN];
#ifdef DEBUG
	fprintf(stderr,"DEJITTER----------------> %d %d %d\n",
		samp->x, samp->y, samp->pressure);
#endif
}

static int dejitter_read(struct ts_sample *samp, int nr, int f_flags)
{
    //struct tslib_dejitter *djt = (struct tslib_dejitter *)info;
	struct ts_sample *s;
	int count = 0, ret;

	//ret = info->next->ops->read(info->next, samp, nr);
	ret = variance_read(samp, nr, f_flags);
	for (s = samp; ret > 0; s++, ret--) {
		if (s->pressure == 0) {
			/*
			 * Pen was released. Reset the state and
			 * forget all history events.
			 */
			djt->nr = 0;
			samp [count++] = *s;
                        continue;
		}

                /* If the pen moves too fast, reset the backlog. */
		if (djt->nr) {
			int prev = (djt->head - 1) & (NR_SAMPHISTLEN - 1);
			if (sqr (s->x - djt->hist [prev].x) +
			    sqr (s->y - djt->hist [prev].y) > djt->delta) {
#ifdef DEBUG
				fprintf (stderr, "DEJITTER: pen movement exceeds threshold\n");
#endif
                                djt->nr = 0;
			}
		}

		djt->hist[djt->head].x = s->x;
		djt->hist[djt->head].y = s->y;
		djt->hist[djt->head].p = s->pressure;
		if (djt->nr < NR_SAMPHISTLEN)
			djt->nr++;

		/* We'll pass through the very first sample since
		 * we can't average it (no history yet).
		 */
		if (djt->nr == 1)
			samp [count] = *s;
		else {
			average (djt, samp + count);
			samp [count].tv = s->tv;
		}
		count++;

		djt->head = (djt->head + 1) & (NR_SAMPHISTLEN - 1);
	}

	return count;
}

void dejitter_init(void) {
	printk("Initializing dejitter function for touchscreen.\n");

	djt->delta = 100;
	djt->head = 0;

	djt->delta = sqr (djt->delta);
	variance_init();
}

/** linear **/
int xeno_ts_read(struct ts_sample *samp, int nr, int f_flags)
{
	//struct tslib_linear *lin = (struct tslib_linear *)info;
	int ret;
	int xtemp,ytemp;

	//ret = info->next->ops->read(info->next, samp, nr);
	ret = dejitter_read(samp, nr, f_flags);
	if (ret >= 0) {
		int nr;

		for (nr = 0; nr < ret; nr++, samp++) {
#ifdef DEBUG
			fprintf(stderr,"BEFORE CALIB--------------------> %d %d %d\n",samp->x, samp->y, samp->pressure);
#endif /*DEBUG*/
			xtemp = samp->x; ytemp = samp->y;
			samp->x = 	( lin->a[2] +
					lin->a[0]*xtemp +
					lin->a[1]*ytemp ) / lin->a[6];
			samp->y =	( lin->a[5] +
					lin->a[3]*xtemp +
					lin->a[4]*ytemp ) / lin->a[6];

			samp->pressure = ((samp->pressure + lin->p_offset)
						 * lin->p_mult) / lin->p_div;
			if (lin->swap_xy) {
				int tmp = samp->x;
				samp->x = samp->y;
				samp->y = tmp;
			}
		}
	}

	return ret;
}

void linear_init(void) {

	printk("Initializing linear function for touchscreen.\n");
	/* The following values have been retrieved from /var/pointercal on EMB touchscreen (DRE) */
	lin->a[0] = 8683;
	lin->a[1] = -3;
	lin->a[2] = -820774;
	lin->a[3] = -37;
	lin->a[4] = -11410;
	lin->a[5] = 22000426;
	lin->a[6] = 65536;
	lin->p_offset = 0;
	lin->p_mult   = 1;
	lin->p_div    = 1;
	lin->swap_xy  = 0;

	dejitter_init();
}
