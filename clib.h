#ifndef __CLIB_H__
#define __CLIB_H__

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define ENONE   0
#define EAGAIN  1
#define EINVAL  2
#define ENOMEM  3
#define ENODEV  4
#define ERANGE  5

#define LONG_MIN (-2147483648LL)
#define LONG_MAX 2147483647LL
#define STR_MAX 1024

extern int errno;

void putc(char c);
void puts(char *s);
char getc(void);
char *gets(char *s);

void printf(char *fmt, ...);
int scanf(char *fmt, ...);
void sprintf(char *str, char *fmt, ...);
int sscanf(char *str, char *fmt, ...);

int isalpha(const char c);
int isdigit(const char c);
int isupper(const char c);
int islower(const char c);
char toupper(const char c);
char tolower(const char c);

int strlen(char *s);
int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, int n);
char *strcpy(char *dest, char *src);
char *strncpy(char *dest, char *src, int n);
char *strtok(char *s, const char *delim);
long strtol(char *s, char **endp, int base);
int ltostr(char *s, char **endp, long val, int base);

int str_contains(const char *s, char c);

#endif /*__CLIB_H__*/
