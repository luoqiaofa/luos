
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stdint.h>
#include "types.h"

typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef unsigned int uint;

#ifdef CONFIG_PHYS_64BIT
typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;
#else
/*  DMA addresses are 32-bits wide */
typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;
#endif
#define USHRT_MAX   ((u16)(~0U))
#define SHRT_MAX    ((s16)(USHRT_MAX>>1))
#define SHRT_MIN    ((s16)(-SHRT_MAX - 1))
#define INT_MAX     ((int)(~0U>>1))
#define INT_MIN     (-INT_MAX - 1)
#define UINT_MAX    (~0U)
#define LONG_MAX    ((long)(~0UL>>1))
#define LONG_MIN    (-LONG_MAX - 1)
#define ULONG_MAX   (~0UL)
#define LLONG_MAX   ((long long)(~0ULL>>1))
#define LLONG_MIN   (-LLONG_MAX - 1)
#define ULLONG_MAX  (~0ULL)


#if 0
#ifndef _VALIST
#define _VALIST
typedef char *va_list;
#endif /* _VALIST */
#endif

typedef struct _file {
    int eof;
    int seek;
    void *buf;
} FILE;

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern int snprintf(char * buf, size_t size, const char *fmt, ...);
extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int sprintf(char * buf, const char *fmt, ...);
extern int vsscanf(const char * buf, const char * fmt, va_list args);
extern int sscanf(const char * buf, const char * fmt, ...);

extern void putc(int c, FILE *stream);
extern int puts(const char *s);
extern int getc(FILE *stream);

int fflush(FILE *stream);
int printf(const char *fmt, ...);
int fprintf(FILE *stream, const char *format, ...);
int scanf(const char * fmt, ...);


extern FILE *stdin; 
extern FILE *stdout; 
extern FILE *stderr; 

#endif /* _STDIO_H */
