
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include "types.h"

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
