/*
    xeno_ts.c - Xenomai wrapper functions for using touchscreen driver.

    (c) 2009 D. Rossier/C. Bardet, REDS Institute, HEIG-VD

*/


#define EVDEV_MINOR_BASE	64
#define EVDEV_MINORS		32
#define EVDEV_BUFFER_SIZE	64

#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/major.h>
#include <linux/smp_lock.h>
#include <linux/device.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/compat.h>

#include "xeno-ts.h"

static	struct evdev_list *xeno_evdev_list;

struct evdev {
	int exist;
	int open;
	int minor;
	char name[16];
	struct input_handle handle;
	wait_queue_head_t wait;
	struct evdev_list *grab;
	struct list_head list;
};

struct evdev_list {
	struct input_event buffer[EVDEV_BUFFER_SIZE];
	int head;
	int tail;
	struct fasync_struct *fasync;
	struct evdev *evdev;
	struct list_head node;
};

static struct evdev *xeno_evdev_table[EVDEV_MINORS];

static void xeno_evdev_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	struct evdev *evdev = handle->private;
	struct evdev_list *list;

	if (evdev->grab) {
		list = evdev->grab;

		do_gettimeofday(&list->buffer[list->head].time);
		list->buffer[list->head].type = type;
		list->buffer[list->head].code = code;
		list->buffer[list->head].value = value;
		list->head = (list->head + 1) & (EVDEV_BUFFER_SIZE - 1);

		kill_fasync(&list->fasync, SIGIO, POLL_IN);
	} else
		list_for_each_entry(list, &evdev->list, node) {

			do_gettimeofday(&list->buffer[list->head].time);
			list->buffer[list->head].type = type;
			list->buffer[list->head].code = code;
			list->buffer[list->head].value = value;
			list->head = (list->head + 1) & (EVDEV_BUFFER_SIZE - 1);

			kill_fasync(&list->fasync, SIGIO, POLL_IN);
		}

	wake_up_interruptible(&evdev->wait);
}


static void xeno_evdev_free(struct evdev *evdev)
{
	xeno_evdev_table[evdev->minor] = NULL;
	kfree(evdev);
}

static int xeno_evdev_release(void)
{
	struct evdev_list *list = xeno_evdev_list;

	if (list->evdev->grab == list) {
		input_release_device(&list->evdev->handle);
		list->evdev->grab = NULL;
	}

	//xeno_evdev_fasync(-1, file, 0);
	list_del(&list->node);

	if (!--list->evdev->open) {
		if (list->evdev->exist)
			input_close_device(&list->evdev->handle);
		else
			xeno_evdev_free(list->evdev);
	}

	kfree(list);
	return 0;
}

static int xeno_evdev_open(void)
{
	//struct evdev_list *list;
	//int i = iminor(inode) - EVDEV_MINOR_BASE;
	int i = 0;
	//int accept_err;

	if (i >= EVDEV_MINORS || !xeno_evdev_table[i] || !xeno_evdev_table[i]->exist)
		return -ENODEV;

	//if ((accept_err = input_accept_process(&(xeno_evdev_table[i]->handle), file)))
	//	return accept_err;

	if (!(xeno_evdev_list = kmalloc(sizeof(struct evdev_list), GFP_KERNEL)))
		return -ENOMEM;
	memset(xeno_evdev_list, 0, sizeof(struct evdev_list));

	xeno_evdev_list->evdev = xeno_evdev_table[i];
	list_add_tail(&xeno_evdev_list->node, &xeno_evdev_table[i]->list);
	//file->private_data = list;

	if (!xeno_evdev_list->evdev->open++)
		if (xeno_evdev_list->evdev->exist)
			input_open_device(&xeno_evdev_list->evdev->handle);

	return 0;
}

static ssize_t xeno_evdev_read(char *buffer, size_t count, int f_flags)
{
	//struct evdev_list *list = file->private_data;
	int retval;
	f_flags |= O_RDONLY;

	if (count < sizeof(struct input_event))
		return -EINVAL;

	if (xeno_evdev_list->head == xeno_evdev_list->tail && xeno_evdev_list->evdev->exist && (f_flags & O_NONBLOCK))
		return -EAGAIN;

	retval = wait_event_interruptible(xeno_evdev_list->evdev->wait,
			xeno_evdev_list->head != xeno_evdev_list->tail || (!xeno_evdev_list->evdev->exist));

	if (retval)
		return retval;

	if (!xeno_evdev_list->evdev->exist)
		return -ENODEV;

	while (xeno_evdev_list->head != xeno_evdev_list->tail && retval + sizeof(struct input_event) <= count) {
		memcpy(buffer + retval, xeno_evdev_list->buffer + xeno_evdev_list->tail, sizeof(struct input_event));
		xeno_evdev_list->tail = (xeno_evdev_list->tail + 1) & (EVDEV_BUFFER_SIZE - 1);
		retval += sizeof(struct input_event);
	}

	return retval;
}

/*****************/

struct tslib_input {
	//struct tslib_module_info module;

	int	current_x;
	int	current_y;
	int	current_p;

	int	sane_fd;
	int	using_syn;
} inf;

static long xeno_evdev_ioctl(unsigned int cmd, unsigned long arg);

static int check_fd(struct tslib_input *i)
{
	//struct tsdev *ts = i->module.dev;
	unsigned int version;
	u_int32_t bit;
	u_int64_t absbit;

	xeno_evdev_ioctl(EVIOCGVERSION, (unsigned long)&version);
	xeno_evdev_ioctl(EVIOCGBIT(EV_ABS, sizeof(bit) * 8), (unsigned long)&bit);


	if (! ((xeno_evdev_ioctl(EVIOCGVERSION, (unsigned long)&version) >= 0) &&
		(version == EV_VERSION) &&
		(xeno_evdev_ioctl(EVIOCGBIT(0, sizeof(bit) * 8), (unsigned long)&bit) >= 0) &&
		(bit & (1 << EV_ABS)) &&
		(xeno_evdev_ioctl(EVIOCGBIT(EV_ABS, sizeof(absbit) * 8), (unsigned long)&absbit) >= 0) &&
		(absbit & (1 << ABS_X)) &&
		(absbit & (1 << ABS_Y)) && (absbit & (1 << ABS_PRESSURE)))) {
		printk("selected device is not a touchscreen I understand\n");
		return -1;
	}

	if (bit & (1 << EV_SYN))
		i->using_syn = 1;

	return 0;
}

int xeno_input_read(struct ts_sample *samp, int nr, int f_flags) {
  struct tslib_input *i = (struct tslib_input *)&inf;
  //struct tsdev *ts = inf->dev;
  struct input_event ev;
  int ret = nr;
  int total = 0;

	if (i->sane_fd == 0)
		i->sane_fd = check_fd(i);

	if (i->sane_fd == -1)
 		return 0;

	if (i->using_syn) {
		while (total < nr) {
			ret = xeno_evdev_read((char *)&ev, sizeof(struct input_event), f_flags);
			if (ret < (int)sizeof(struct input_event)) {
				total = -1;
				break;
			}

			switch (ev.type) {
			case EV_KEY:
				switch (ev.code) {
				case BTN_TOUCH:
					if (ev.value == 0) {
						/* pen up */
						samp->x = 0;
						samp->y = 0;
						samp->pressure = 0;
						samp->tv = ev.time;
						samp++;
						total++;
					}
					break;
				}
				break;
			case EV_SYN:
				/* Fill out a new complete event */
				samp->x = i->current_x;
				samp->y = i->current_y;
				samp->pressure = i->current_p;
				samp->tv = ev.time;
	#ifdef DEBUG
				fprintf(stderr, "RAW---------------------> %d %d %d %d.%d\n",
						samp->x, samp->y, samp->pressure, samp->tv.tv_sec,
						samp->tv.tv_usec);
	#endif		 /*DEBUG*/
					samp++;
				total++;
				break;
			case EV_ABS:
				switch (ev.code) {
				case ABS_X:
					i->current_x = ev.value;
					break;
				case ABS_Y:
					i->current_y = ev.value;
					break;
				case ABS_PRESSURE:
					i->current_p = ev.value;
					break;
				}
				break;
			}
		}
		ret = total;
	} else {
		unsigned char *p = (unsigned char *) &ev;
		int len = sizeof(struct input_event);

		while (total < nr) {
			ret = xeno_evdev_read(p, len, f_flags);
			if (ret == -1) {

				break;
			}

			if (ret < (int)sizeof(struct input_event)) {
				/* short read
				 * restart read to get the rest of the event
					 */
					p += ret;
					len -= ret;
					continue;
				}
				/* successful read of a whole event */

				if (ev.type == EV_ABS) {
					switch (ev.code) {
					case ABS_X:
						if (ev.value != 0) {
							samp->x = i->current_x = ev.value;
							samp->y = i->current_y;
							samp->pressure = i->current_p;
						} else {
							printk("tslib: dropped x = 0\n");
							continue;
						}
						break;
					case ABS_Y:
						if (ev.value != 0) {
							samp->x = i->current_x;
							samp->y = i->current_y = ev.value;
							samp->pressure = i->current_p;
						} else {
							printk("tslib: dropped y = 0\n");
							continue;
						}
						break;
					case ABS_PRESSURE:
						samp->x = i->current_x;
						samp->y = i->current_y;
						samp->pressure = i->current_p = ev.value;
						break;
					}
					samp->tv = ev.time;
		#ifdef DEBUG
					fprintf(stderr, "RAW---------------------------> %d %d %d\n",
						samp->x, samp->y, samp->pressure);
		#endif	 /*DEBUG*/
					samp++;
					total++;
				} else if (ev.type == EV_KEY) {
					switch (ev.code) {
					case BTN_TOUCH:
						if (ev.value == 0) {
							/* pen up */
							samp->x = 0;
							samp->y = 0;
							samp->pressure = 0;
							samp->tv = ev.time;
							samp++;
							total++;
						}
						break;
					}
				} else {
					printk("tslib: Unknown event type %d\n", ev.type);
				}
				p = (unsigned char *) &ev;
			}
			ret = total;
		}

		return ret;
}

/**************************************/

/* No kernel lock - fine */
static unsigned int xeno_evdev_poll(struct file *file, poll_table *wait)
{
	struct evdev_list *list = xeno_evdev_list;
	poll_wait(file, &list->evdev->wait, wait);
	return ((list->head == list->tail) ? 0 : (POLLIN | POLLRDNORM)) |
		(list->evdev->exist ? 0 : (POLLHUP | POLLERR));
}

static long xeno_evdev_ioctl(unsigned int cmd, unsigned long arg)
{
	struct evdev_list *list = xeno_evdev_list;
	struct evdev *evdev = list->evdev;
	struct input_dev *dev = evdev->handle.dev;
	struct input_absinfo abs;
	unsigned char *p = (unsigned char *)arg;

	int i, t, u, v;

	if (!evdev->exist) return -ENODEV;

	switch (cmd) {

		case EVIOCGVERSION:
			i = EV_VERSION;
			memcpy(p, (unsigned char *)&i, sizeof(int));
			return 0;

		case EVIOCGID:
			memcpy(p, &dev->id, sizeof(struct input_id));

		case EVIOCGKEYCODE:
			memcpy(p, (unsigned char *) &t, sizeof(int));
			if (t < 0 || t >= dev->keycodemax || !dev->keycodesize) return -EINVAL;
			i = INPUT_KEYCODE(dev, t);
			memcpy(p+4, (unsigned char *)&i, sizeof(int));
			return 0;

		case EVIOCSKEYCODE:
			memcpy((unsigned char*) &t, p, sizeof(int));
			if (t < 0 || t >= dev->keycodemax || !dev->keycodesize) return -EINVAL;
			memcpy((unsigned char *)&v, p+4, sizeof(int));
			if (v < 0 || v > KEY_MAX) return -EINVAL;
			if (dev->keycodesize < sizeof(v) && (v >> (dev->keycodesize * 8))) return -EINVAL;
			u = SET_INPUT_KEYCODE(dev, t, v);
			clear_bit(u, dev->keybit);
			set_bit(v, dev->keybit);
			for (i = 0; i < dev->keycodemax; i++)
				if (INPUT_KEYCODE(dev,i) == u)
					set_bit(u, dev->keybit);
			return 0;

		case EVIOCSFF:
			if (dev->upload_effect) {
				struct ff_effect effect;
				int err;

				if (memcpy(&effect, p, sizeof(effect)))
					return -EFAULT;
				err = dev->upload_effect(dev, &effect);
				memcpy(&(((struct ff_effect __user *)arg)->id), (char *)&effect.id, sizeof(int));
					return -EFAULT;
				return err;
			}
			else return -ENOSYS;

		case EVIOCRMFF:
			if (dev->erase_effect) {
				return dev->erase_effect(dev, (int)arg);
			}
			else return -ENOSYS;

		case EVIOCGEFFECTS:
			memcpy(p, (char *)dev->ff_effects_max, sizeof(int));
			return 0;

		case EVIOCGRAB:
			if (arg) {
				if (evdev->grab)
					return -EBUSY;
				if (input_grab_device(&evdev->handle))
					return -EBUSY;
				evdev->grab = list;
				return 0;
			} else {
				if (evdev->grab != list)
					return -EINVAL;
				input_release_device(&evdev->handle);
				evdev->grab = NULL;
				return 0;
			}

		default:

			if (_IOC_TYPE(cmd) != 'E')
				return -EINVAL;

			if (_IOC_DIR(cmd) == _IOC_READ) {

				if ((_IOC_NR(cmd) & ~EV_MAX) == _IOC_NR(EVIOCGBIT(0,0))) {

					long *bits;
					int len;

					switch (_IOC_NR(cmd) & EV_MAX) {
						case      0: bits = dev->evbit;  len = EV_MAX;  break;
						case EV_KEY: bits = dev->keybit; len = KEY_MAX; break;
						case EV_REL: bits = dev->relbit; len = REL_MAX; break;
						case EV_ABS: bits = dev->absbit; len = ABS_MAX; break;
						case EV_MSC: bits = dev->mscbit; len = MSC_MAX; break;
						case EV_LED: bits = dev->ledbit; len = LED_MAX; break;
						case EV_SND: bits = dev->sndbit; len = SND_MAX; break;
						case EV_FF:  bits = dev->ffbit;  len = FF_MAX;  break;
						case EV_SW:  bits = dev->swbit;  len = SW_MAX;  break;
						default: return -EINVAL;
					}
					len = NBITS(len) * sizeof(long);
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, bits, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGKEY(0))) {
					int len;
					len = NBITS(KEY_MAX) * sizeof(long);
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->key, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGLED(0))) {
					int len;
					len = NBITS(LED_MAX) * sizeof(long);
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->led, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGSND(0))) {
					int len;
					len = NBITS(SND_MAX) * sizeof(long);
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->snd, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGSW(0))) {
					int len;
					len = NBITS(SW_MAX) * sizeof(long);
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->sw, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGNAME(0))) {
					int len;
					if (!dev->name) return -ENOENT;
					len = strlen(dev->name) + 1;
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->name, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGPHYS(0))) {
					int len;
					if (!dev->phys) return -ENOENT;
					len = strlen(dev->phys) + 1;
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->phys, len);
					return 0;
				}

				if (_IOC_NR(cmd) == _IOC_NR(EVIOCGUNIQ(0))) {
					int len;
					if (!dev->uniq) return -ENOENT;
					len = strlen(dev->uniq) + 1;
					if (len > _IOC_SIZE(cmd)) len = _IOC_SIZE(cmd);
					memcpy(p, dev->uniq, len);
					return 0;
				}

				if ((_IOC_NR(cmd) & ~ABS_MAX) == _IOC_NR(EVIOCGABS(0))) {

					int t = _IOC_NR(cmd) & ABS_MAX;

					abs.value = dev->abs[t];
					abs.minimum = dev->absmin[t];
					abs.maximum = dev->absmax[t];
					abs.fuzz = dev->absfuzz[t];
					abs.flat = dev->absflat[t];
					memcpy(p, &abs, sizeof(struct input_absinfo));

					return 0;
				}

			}

			if (_IOC_DIR(cmd) == _IOC_WRITE) {

				if ((_IOC_NR(cmd) & ~ABS_MAX) == _IOC_NR(EVIOCSABS(0))) {

					int t = _IOC_NR(cmd) & ABS_MAX;

					if (memcpy(&abs, p, sizeof(struct input_absinfo)))
						return -EFAULT;

					dev->abs[t] = abs.value;
					dev->absmin[t] = abs.minimum;
					dev->absmax[t] = abs.maximum;
					dev->absfuzz[t] = abs.fuzz;
					dev->absflat[t] = abs.flat;

					return 0;
				}
			}
	}
	return -EINVAL;
}

static struct file_operations xeno_evdev_fops = {
	.owner =	THIS_MODULE,
	//.read =		xeno_evdev_read,
	//.write =	xeno_evdev_write,
	.poll =		xeno_evdev_poll,
	//.open =		xeno_evdev_open,
	//.release =	xeno_evdev_release,
	//.unlocked_ioctl = xeno_evdev_ioctl,
	//.fasync =	xeno_evdev_fasync,
	//.flush =	xeno_evdev_flush
};

static struct input_handle *xeno_evdev_connect(struct input_handler *handler, struct input_dev *dev, struct input_device_id *id)
{
	struct evdev *evdev;
	int minor;

	for (minor = 0; minor < EVDEV_MINORS && xeno_evdev_table[minor]; minor++);
	if (minor == EVDEV_MINORS) {
		printk(KERN_ERR "evdev: no more free evdev devices\n");
		return NULL;
	}

	if (!(evdev = kmalloc(sizeof(struct evdev), GFP_KERNEL)))
		return NULL;
	memset(evdev, 0, sizeof(struct evdev));

	INIT_LIST_HEAD(&evdev->list);
	init_waitqueue_head(&evdev->wait);

	evdev->exist = 1;
	evdev->minor = minor;
	evdev->handle.dev = dev;
	evdev->handle.name = evdev->name;
	evdev->handle.handler = handler;
	evdev->handle.private = evdev;
	sprintf(evdev->name, "event%d", minor);

	xeno_evdev_table[minor] = evdev;

	devfs_mk_cdev(MKDEV(INPUT_MAJOR, EVDEV_MINOR_BASE + minor),
			S_IFCHR|S_IRUGO|S_IWUSR, "input/event%d", minor);
	class_device_create(input_class,
			MKDEV(INPUT_MAJOR, EVDEV_MINOR_BASE + minor),
			dev->dev, "event%d", minor);

	return &evdev->handle;
}

static void xeno_evdev_disconnect(struct input_handle *handle)
{
	struct evdev *evdev = handle->private;
	struct evdev_list *list;

	class_device_destroy(input_class,
			MKDEV(INPUT_MAJOR, EVDEV_MINOR_BASE + evdev->minor));
	devfs_remove("input/event%d", evdev->minor);
	evdev->exist = 0;

	if (evdev->open) {
		input_close_device(handle);
		wake_up_interruptible(&evdev->wait);
		list_for_each_entry(list, &evdev->list, node)
			kill_fasync(&list->fasync, SIGIO, POLL_HUP);
	} else
		xeno_evdev_free(evdev);
}

static struct input_device_id xeno_evdev_ids[] = {
	{ .driver_info = 1 },	/* Matches all devices */
	{ },			/* Terminating zero entry */
};

MODULE_DEVICE_TABLE(input, xeno_evdev_ids);

static struct input_handler xeno_evdev_handler = {
	.event =	xeno_evdev_event,
	.connect =	xeno_evdev_connect,
	.disconnect =	xeno_evdev_disconnect,
	.fops =		&xeno_evdev_fops,
	.minor =	EVDEV_MINOR_BASE,
	.name =		"evdev",
	.id_table =	xeno_evdev_ids,
};

extern void linear_init(void);

int xeno_ts_init(void)
{
	struct tslib_input *i = &inf;
	input_register_handler(&xeno_evdev_handler);
	xeno_evdev_open();

	//i->module.ops = &__ts_input_ops;
	i->current_x = 0;
	i->current_y = 0;
	i->current_p = 0;
	i->sane_fd = 0;
	i->using_syn = 0;

	linear_init();

	return 0;
}

void xeno_ts_exit(void)
{
	input_unregister_handler(&xeno_evdev_handler);
	xeno_evdev_release();
}

EXPORT_SYMBOL(xeno_ts_init);
EXPORT_SYMBOL(xeno_ts_exit);
EXPORT_SYMBOL(xeno_input_read);
