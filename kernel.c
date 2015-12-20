#include "clib.h"
#include "irq.h"
#include "cli.h"
#include "kernel.h"

int quiet = 0;
int verbose = 0;

int kernel_main(unsigned int r0, unsigned int r1, unsigned int atags)
{
    //int d = -116;
    //unsigned int val = 42;
    //char c1 = 'L';
    //char c2 = 'O';
    //char str[] = "Hello! This is a string.";
    
    enable_interrupts();
    cli_start();

    while(1);

    return 0;
}
