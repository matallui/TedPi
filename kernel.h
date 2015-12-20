#ifndef __KERNEL__
#define __KERNEL__

#ifdef RPI2
#define REG_BASE 0x3f000000
#else
#define REG_BASE 0x20000000
#endif

#endif /* __KERNEL__ */
