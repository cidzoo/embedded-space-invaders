#ifndef __RT_APP_M_H__
#define __RT_APP_M_H__
/******************************************************************************
 *                           Xenomai Definitions
 *****************************************************************************/
#define TIMER_PERIODIC	0           /**< 1:Periodic timer, 0:Aperiodic timer */

#define STACK_SIZE		8192        /**< Default stack size */
#define MS				1000000		/**< 1 ms in ns */

void __exit cleanup_module(void);

#endif /* __RT_APP_M_H__ */
