#include "clib.h"
#include "uart.h"

typedef unsigned char *va_list;
#define va_start(list, param)   (list = (((va_list)&param) + sizeof(param)))
#define va_arg(list, type)      (*(type *)((list += sizeof(type)) - sizeof(type)))
#define va_end(list)            (list = (va_list)0)

int errno;

void putc(char c)
{
    uart_putc(c);
}

void puts(char *s)
{
    char *p;

    for (p = s; *p != '\0'; p++)
        uart_putc(*p);
}

char getc(void)
{
    return uart_getc();
}

char *gets(char *s)
{

    int end = 0;
    int i = 0, j;
    char *p;
    char buffer[STR_MAX];

    while (i < (STR_MAX-1) && !end) {
        buffer[i] = uart_getc(); 
        switch (buffer[i]) {
            case '\r':
            case '\n':
                uart_putc('\n');
                buffer[i++] = '\n';
                end = 1;
                break;
            case '\b':
            case '\x7f':
                if (i == 0) {
                    uart_putc('\a');
                } else {
                    uart_putc('\b');
                    uart_putc(' ');
                    uart_putc('\b');
                    i--;
                }
                break;
            case '\t':
                uart_putc('\a');
                break;
            default:
                if (i == 255) {
                    uart_putc('\a');
                } else {
                    uart_putc(buffer[i]);
                    i++;
                }
        }
    }
    
    p = s;
    for (j = 0; j < i; j++)
        *p++ = buffer[j];

    *p = '\0';

    return s;
}

void printf(char *fmt, ...)
{
    int d;
    char c;
    char *s;
    unsigned int x;
    va_list list;
    char *p;
    char buf[STR_MAX];

    va_start(list, fmt);
    for (p = fmt; *p != '\0'; p++) {
        if (*p == '%') {
            switch (*(++p)) {
                case 'd':
                    d = va_arg(list, int);
                    ltostr(buf, NULL, (long)d, 10);
                    puts(buf);
                    break;
                case 'u':
                    x = va_arg(list, unsigned int);
                    ltostr(buf, NULL, (long)x, 10);
                    puts(buf);
                    break;
                case 'c':
                    c = (char)va_arg(list, int);
                    putc(c);
                    break;
                case 's':
                    s = va_arg(list, char*);
                    puts(s);
                    break;
                case 'x':
                    x = va_arg(list, unsigned int);
                    ltostr(buf, NULL, (long)x, 16);
                    puts(buf);
                    break;
                case 'b':
                    x = va_arg(list, unsigned int);
                    ltostr(buf, NULL, (long)x, 2);
                    puts(buf);
                    break;
                default:
                    putc('%');
                    putc(*p);
                    break;
            }
        } else {
            putc(*p);
        }
    }
    va_end(list);
}

int scanf(char *fmt, ...)
{
    int i;
    int *d;
    char *c;
    char *s;
    unsigned int *x;
    va_list list;
    char *p;
    int count = 0;
    char buf[STR_MAX];
    char *str = buf;

    gets(buf);

    va_start(list, fmt);
    for (p = fmt; *p != '\0' && *str != '\0' 
            && *str != '\r' && *str != '\n'; p++) {

        while (*str == ' ' && *str != '\0' &&
                *str != '\n' && *str != '\r')
            str++;

        if (*str == '\0' || *str == '\r' || *str == '\n')
            break;

        if (*p == '%') {
            switch (*(++p)) {
                case 'd':
                    d = va_arg(list, int*);
                    *d = (int)strtol(str, &str, 10);
                    count++;
                    break;
                case 'u':
                    x = va_arg(list, unsigned int*);
                    *x = (unsigned int)strtol(str, &str, 10);
                    count++;
                    break;
                case 'c':
                    c = (char*)va_arg(list, int*);
                    *c = *str++;
                    count++;
                    break;
                case 's':
                    s = va_arg(list, char*);
                    i = 0;
                    while (*str != ' ' && *str != '\0' &&
                            *str != '\r' && *str != '\n')
                        s[i++] = *str++;
                    s[i] = '\0';
                    count++;
                    break;
                case 'x':
                    x = va_arg(list, unsigned int*);
                    *x = (unsigned int)strtol(str, &str, 16);
                    count++;
                    break;
                case 'b':
                    x = va_arg(list, unsigned int*);
                    *x = (unsigned int)strtol(str, &str, 2);
                    count++;
                    break;
                default:
                    break;
            }
        }
    }
    va_end(list);

    return count;
}

void sprintf(char *str, char *fmt, ...)
{
    char const digit[] = "0123456789abcdef";
    int i;
    int d;
    char c;
    char *s;
    unsigned int x = 0;
    va_list list;
    char *p, *ps;
    int base = 0;

    va_start(list, fmt);
    for (p = fmt; *p != '\0'; p++) {
        if (*p == '%') {
            switch (*(++p)) {
                case 'd':
                    d = va_arg(list, int);
                    if (d < 0) {
                        *str++ = '-';
                        x = (unsigned int)(-d);
                    }
                    base = 10;
                    break;
                case 'u':
                    x = va_arg(list, unsigned int);
                    base = 10;
                    break;
                case 'c':
                    c = (char)va_arg(list, int);
                    *str++ = c;
                    break;
                case 's':
                    s = va_arg(list, char*);
                    for (i = 0; s[i] != '\0'; i++)
                        *str++ = s[i];
                    break;
                case 'x':
                    x = va_arg(list, unsigned int);
                    base = 16;
                    break;
                case 'b':
                    x = va_arg(list, unsigned int);
                    base = 2;
                    break;
                default:
                    *str++ = '%';
                    *str++ = *p;
                    break;
            }            

            if (base) {
                ps = str;

                switch (base) {
                    case 2:
                        *ps++ = 'b';
                    case 8:
                        *ps++ = '0';
                        break;
                    case 10:
                        break;
                    case 16:
                        *ps++ = '0';
                        *ps++ = 'x';
                        break;
                    default:
                        return;
                }

                if (x == 0)
                    *ps++ = '0';

                for (i = x; i; i /= base) {
                    ps++;
                }
                str = ps;

                for (i = x; i; i /= base) {
                    *--ps = digit[i%base];
                }
                base = 0;
            }
        } else {
            *str++ = *p;
        }
    }
    
    *str = '\0';
    va_end(list);
    
}

int sscanf(char *str, char *fmt, ...)
{
    int i;
    int *d;
    char *c;
    char *s;
    unsigned int *x;
    va_list list;
    char *p;
    int count = 0;

    va_start(list, fmt);
    for (p = fmt; *p != '\0' && *str != '\0' 
            && *str != '\r' && *str != '\n'; p++) {

        while (*str == ' ' && *str != '\0' &&
                *str != '\n' && *str != '\r')
            str++;

        if (*str == '\0' || *str == '\r' || *str == '\n')
            break;

        if (*p == '%') {
            switch (*(++p)) {
                case 'd':
                    d = va_arg(list, int*);
                    *d = (int)strtol(str, &str, 10);
                    count++;
                    break;
                case 'u':
                    x = va_arg(list, unsigned int*);
                    *x = (unsigned int)strtol(str, &str, 10);
                    count++;
                    break;
                case 'c':
                    c = (char*)va_arg(list, int*);
                    *c = *str++;
                    count++;
                    break;
                case 's':
                    s = va_arg(list, char*);
                    i = 0;
                    while (*str != ' ' && *str != '\0' &&
                            *str != '\r' && *str != '\n')
                        s[i++] = *str++;
                    s[i] = '\0';
                    count++;
                    break;
                case 'x':
                    x = va_arg(list, unsigned int*);
                    *x = (unsigned int)strtol(str, &str, 16);
                    count++;
                    break;
                case 'b':
                    x = va_arg(list, unsigned int*);
                    *x = (unsigned int)strtol(str, &str, 2);
                    count++;
                    break;
                default:
                    break;
            }
        }
    }
    va_end(list);

    return count;
}

int strlen(char *s)
{
    int i;

    for (i = 0; s[i] != '\0'; i++);

    return i;
}

int strcmp(char *s1, char *s2)
{
    int i;
    
    for (i = 0; s1[i] != '\0' && s2[i] != '\0' && s1[i] == s2[i]; i++);

    if (s1[i] == '\0' && s2[i] == '\0')
        return 0;

    return (s1[i] - s2[i]);
}

int strncmp(char *s1, char *s2, int n)
{
    int i;
    
    for (i = 0; s1[i] != '\0' && s2[i] != '\0' && s1[i] == s2[i] && i < n; i++);

    if (i == n || (s1[i] == '\0' && s2[i] == '\0'))
        return 0;

    return (s1[i] - s2[i]);
}

char *strcpy(char *dest, char *src)
{
    int i;

    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];

    dest[i] = '\0';

    return dest;
}

char *strncpy(char *dest, char *src, int n)
{
    int i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';

    return dest;
}

int str_contains(const char *s, char c)
{
    int i;

    for (i = 0; s[i] != '\0' && i < STR_MAX; i++) {
        if (s[i] == c)
            return i+1;
    }

    return 0;
}

char *strtok(char *s, const char *delim)
{
    static char str[STR_MAX+1];
    static char *p = NULL;
    static char tok[STR_MAX+1];
    int i;

    if (!s && !p) {
        errno = EINVAL;
        return NULL;
    }

    if (s) {
        strncpy(str, s, STR_MAX);
        str[STR_MAX] = '\0';
        p = str;
    }

    for (i = 0; *p != '\0' && i < STR_MAX; p++) {
        if (str_contains(delim, *p)) {
            if (i) 
                break;
        } else {
            tok[i++] = *p;
        }
    }

    if (i == 0) {
        p = NULL;
        return NULL;
    }

    tok[i] = '\0';
    return tok;
}

int isalpha(const char c) 
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z'))
        return 1;
    return 0;
}

int isdigit(const char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

int isupper(const char c)
{
    return ((c & 0x20) == 0) ? 1 : 0;
}

int islower(const char c)
{
    return (isupper(c) == 0) ? 1 : 0;
}

char toupper(const char c)
{
    return (c & 0xDF);
}

char tolower(const char c)
{
    return (c | 0x20);
}

long strtol(char *s, char **endp, int base)
{
    char *p;
    int neg;
    int ovf;
    long tmp;
    long prev;
    long res;
    int i;

    ovf = 0;
    neg = 0;
    p = s;

    while (*p == ' ' && *p != '\0')
        p++;

    if (*p == '-') {
        neg = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if ((base == 0 || base == 16) &&
        (*p == '0') && (p[1] == 'x' || p[1] == 'X')) {
        base = 16;
        p += 2;
    } else if ((base == 0 || base == 2) &&
        (*p == '0') && (p[1] == 'b' || p[1] == 'B')) {
        base = 2;
        p += 2;
    } else if ((base == 0 || base == 8 ) && (*p == '0')) {
        base = 8;
    } else {
        base = 10;
    }

    for (res = 0, i = 0; *p != '\0'; p++, i++) {
        if (isdigit(*p))
            tmp = (long)(*p - '0');
        else if (isalpha(*p))
            tmp = islower(*p) ? (long)(*p - 'a' + 10) : (long)(*p - 'A' + 10);
        else
            break; 

        if (tmp >= base)
            break;

        prev = res;
        res *= base;
        res += tmp;

        if (res < prev || res >= LONG_MAX) {
            ovf = 1;
            break;
        }
    }

    if (i == 0 || ovf)
        res = neg ? LONG_MIN : LONG_MAX;
    else if (neg)
        res = -res;
    
    if (endp) {
        *endp = (i > 0) ? p : s;
    }
    
    return res;
    
}

int ltostr(char *s, char **endp, long val, int base)
{
    char const digit[] = "0123456789abcdef";
    char *ps;
    unsigned long x;
    int i;
    
    ps = s;

    switch (base) {
        case 2:
            *ps++ = 'b';
        case 8:
            *ps++ = '0';
            break;
        case 10:
            break;
        case 16:
            *ps++ = '0';
            *ps++ = 'x';
            break;
        default:
            base = 10;
            break;
    }

    if (val < 0 && base == 10) {
        x = (unsigned long)(val * (-1));
        *ps++ = '-';
    } else {
        x = (unsigned long)val;
    }

    if (x == 0)
        *ps++ = '0';

    for (i = x; i; i /= base) {
        ps++;
    }

    *ps = '\0';
    s = ps;

    for (i = x; i; i /= base) {
        *--ps = digit[i%base];
    }

    if (endp)
        *endp = s;

    return 0;
}
