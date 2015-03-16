#include <stdarg.h>

char* intobase(char *buf, int n, int val, int width, int base);
char* intodec(char *buf, int n, int val, int width);
char* intobin(char *buf, int n, int val, int width);
char* intohex(char *buf, int n, int val, int width);
int snprintf(char *s, int n, const char *format, ... );
int snprintf_ap(char *s, int n, const char *format, va_list ap);
char* console_printf( const char * format, ...);
int printf ( const char * format, ... );
