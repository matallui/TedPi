#include <stdio.h>
#include <errno.h>
#include "uart.h"

#undef errno
extern int errno;

/* Required include for fstat() */
#include <sys/stat.h>

/* Required include for times() */
#include <sys/times.h>

/* Prototype for the UART write function */
#include "uart.h"

/* A pointer to a list of environment variables and their values. For a minimal
   environment, this empty list is adequate: */
char *__env[1] = {0};
char **environ = __env;

/* A helper function written in assembler to aid us in allocating memory */
extern caddr_t _get_stack_pointer(void);

void _exit(int status)
{
    (void)status;
    while(1);
}

int _close(int file)
{
    errno = EBADF;
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _getpid(void)
{
    return 1;
}

int _isatty(int file)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char *name, int flags, int mode)
{
    errno = ENOSYS;
    return -1;
}

int _read(int file, char *ptr, int len)
{
    int end = 0;
    int i = 0, j;
    char buffer[256];

    while (i < len && i < 255 && !end) {
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
    
    for (j = 0; j < i; j++)
        *ptr++ = buffer[j];

    return i;
}

int _write(int file, char *ptr, int len)
{
    int i;

    for(i = 0; i < len; i++)
      uart_putc(*ptr++);

    return len;
}

caddr_t _sbrk(int incr)
{
    extern char heap_low;
    extern char heap_top;
    static char *heap_end;
    char* prev_heap_end;

    if( heap_end == 0 )
        heap_end = &heap_low;

     prev_heap_end = heap_end;
     if ((heap_end + incr) > &heap_top)
         return (caddr_t)0;

     heap_end += incr;

     return (caddr_t)prev_heap_end;
}

int _stat(const char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

clock_t _times(struct tms *buf)
{
    errno = EACCES;
    return -1;
}

int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

