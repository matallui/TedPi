#include "clib.h"
#include "uart.h"
#include "led.h"
#include "drivers/ds1337.h"
#include "cmd.h"
#include "cli.h"

static void print_header(void)
{
    printf("\n############################################\n");
    printf("##                TedPi 0.0.1             ##\n");
    printf("############################################\n\n");
}

static void prompt(void)
{
    printf("tedpi~> ");
}


void cli_start(void)
{
    char buf[STR_MAX];

    uart_enable(115200, 8);
    
    print_header();
    cmd_usage();
    
    while(1) {
        prompt();
        gets(buf);
        cmd_exec(buf);
    }
}


