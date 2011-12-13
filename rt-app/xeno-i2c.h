
/*
 * xeno-i2c.h - Definition of prototypes for the i2c driver.
 * 		To be included in every file where these functions are called.
 */

extern int xeno_i2c_init(void);
extern int xeno_i2c_exit(void);
extern ssize_t xeno_i2c_read(char *buf, size_t count);
extern ssize_t xeno_i2c_write (const char *buf, size_t count);
extern int xeno_i2c_ioctl(unsigned int cmd, unsigned long arg);


/* imx-registers for i2c */
struct i2c_imx_i2creg {
  	volatile u32 iadr;
	volatile u32 ifdr;
	volatile u32 i2cr;
	volatile u32 i2sr;
	volatile u32 i2dr;
};

extern void set_i2c_imx_irq_ok(int value);
extern int get_i2c_imx_irq_ok(void);
extern void set_i2c_imx_i2sr(int value);
extern int get_i2c_imx_i2sr(void);
extern struct i2c_imx_i2creg *get_i2c_imx_reg(void);
