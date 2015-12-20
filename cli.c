#include "clib.h"
#include "uart.h"
#include "led.h"
#include "drivers/ds1337.h"
#include "cmd.h"
#include "cli.h"

static void print_header(void)
{
    printf("\n############################################\n");
    printf("##                TedPi 1.0.1             ##\n");
    printf("############################################\n\n");
}

static void prompt(void)
{
    printf("tedpi~> ");
}

static void print_help(void)
{
    printf("\nCommands available:\n");
    printf("\t* help: prints this menu\n");
    printf("\t* led: LED control\n");
    printf("\t\ton/off: LED on/off\n");
    printf("\t\tblink <ms>: blinks LED every with <ms> freq\n");
    printf("\t* gettime: get DS1337 time\n");
    printf("\t* settime hh:mm:ss: set DS1337 time\n\n");
}

static void parse_led(char *args)
{
    char param1[256];
    int param2 = 0;

    sscanf(args, "%s", param1);
    if (strcmp(param1, "on") == 0) {
        led_blink_disable();
        led_on();
        printf(" * LED On\n");
    } else if (strcmp(param1, "off") == 0) {
        led_blink_disable();
        led_off();
        printf(" * LED Off\n");
    } else if (strcmp(param1, "blink") == 0) {
        sscanf(args+6, "%d", &param2);
        if (param2 <= 0)
            printf(" * Invalid blink time!\n");
        else
            led_blink(param2);
    } else {
        printf(" * Invalid parameter!\n");
    }
}

static void print_time(struct ds1337_time *time)
{
    //printf("-> Time: %02d:%02d:%02d\n", (int)time->hours,
    //        (int)time->minutes, (int)time->seconds);
    printf("-> Time: %d:%d:%d\n", (int)time->hours,
            (int)time->minutes, (int)time->seconds);
}

static void parse_time(char *args, struct ds1337_time *time)
{
    int hh, mm, ss;
    sscanf(args, "%d:%d:%d", &hh, &mm, &ss);
    time->hours = (uint8_t) hh;
    time->minutes = (uint8_t) mm;
    time->seconds = (uint8_t) ss;
}

static void parse(char *buf)
{
    char cmd[256];
    struct ds1337_time time;

    printf("parse: buf = %s\n", buf);
    sscanf(buf, "%s", cmd);
    printf("parse: cmd = %s\n", cmd);
    
    if (strcmp(cmd, "led") == 0) {
        parse_led((char*)buf + 4);  
    } else if (strcmp(cmd, "gettime") == 0) {
        ds1337_get_time(&time);
        print_time(&time);
    } else if (strcmp(cmd, "settime") == 0) {
        parse_time((char*)buf + 8, &time);
        ds1337_set_time(time);
    } else if (strcmp(cmd, "help") == 0) {
        print_help();
    } else {
        printf(" * Unknown command! Type 'help' for help...\n");
    }
}

void cli_start(void)
{
    char buf[257];
    int ret;

    uart_enable(115200, 8);
    
    print_header();
    print_help();
    
    while(1) {
        prompt();
        gets(buf);
        parse(buf);
        //ret = cmd_exec(buf);
        //printf(" * cmd_exec returned %d\n", ret);
    }
}


