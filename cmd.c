#include "clib.h"
#include "twi.h"
#include "cmd.h"

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define COMMAND(_name, _usage, _desc, _opts, _exec, _help) { \
    .name = _name, \
    .desc = _desc, \
    .exec = _exec, \
    .help = _help, \
    }

#define CMD_OPTION(_short, _long, _usage, _desc, _type) { \
    .opt_short = _short, \
    .opt_long = _long, \
    .desc = _desc, \
    .usage = _usage, \
    .type = _type, \
    }

#define ARG_SIZE    32
#define MAX_ARGS    32

typedef enum {
    OPT_NONE = 0,
    OPT_INT,
    OPT_STR,
} opt_type_t;

struct cli_opt {
    char    opt_short;
    char    *opt_long;
    char    *desc;
    char    *usage;
    opt_type_t type;     
};

struct optval {
    char    opt;
    union {
            int  v_int;
            char v_str[ARG_SIZE];
    };
};

struct cli_cmd {
    char            *name;
    char            *desc;
    char            *usage;
    struct cli_opt  *opts;
    int             (*exec)(char *buf);
    void            (*help)(void);
};

extern int quiet;
extern int verbose;

static void help_help(void);
static int help_exec(char *buf);
static void i2c_help(void);
static int i2c_exec(char *buf);
static void led_help(void);
static int led_exec(char *buf);
static void rtc_help(void);
static int rtc_exec(char *buf);

static struct cli_opt i2c_opts[] = {
    CMD_OPTION('q', "quiet", "-q|--quiet", "supress output messages", OPT_NONE),
    CMD_OPTION('v', "verbose", "-v|--verbose", "verbose output", OPT_NONE),
    CMD_OPTION('r', "read", "-r|--read ADDR OFF N", "i2c read command", OPT_NONE),
    CMD_OPTION('w', "write", "-w|--write ADDR OFF N Byte0 Byte1 ...", "i2c write command", OPT_NONE),
};

static struct cli_opt led_opts[] = {
    CMD_OPTION('q', "quiet", "-q|--quiet", "supress output messages", OPT_NONE),
    CMD_OPTION('v', "verbose", "-v|--verbose", "verbose output", OPT_NONE),
    CMD_OPTION('s', "set", "-s|--set N", "turns RPi LED on (N=1) or off (N=0)", OPT_INT),
    CMD_OPTION('b', "blink", "-b|--blink MS", "blinks RPi LED with MS miliseconds interval", OPT_INT),
};

static struct cli_opt rtc_opts[] = {
    CMD_OPTION('q', "quiet", "-q|--quiet", "supress output messages", OPT_NONE),
    CMD_OPTION('v', "verbose", "-v|--verbose", "verbose output", OPT_NONE),
    CMD_OPTION('t', "gettime", "-t|--gettime", "print ds1337 time hh:mm:ss", OPT_NONE),
    CMD_OPTION('d', "getdate", "-d|--getdate", "print ds1337 date dd:mm:yyyy", OPT_NONE),
    CMD_OPTION('T', "settime", "-t|--settime HH:MM:SS", "set ds1337 time", OPT_STR),
    CMD_OPTION('D', "setdate", "-d|--setdate DD:MM:YYYY", "set ds1337 date", OPT_STR),
};

static struct cli_cmd cmds[] = {
    COMMAND("help", "help [CMD]", "print this menu or CMD usage", NULL, help_exec, help_help),
    COMMAND("i2c", "i2c OPTS", "send i2c command", i2c_opts, i2c_exec, i2c_help),
    COMMAND("led", "led [OPTS]", "control RPi LED (default: print LED status)", led_opts, led_exec, led_help),
    COMMAND("rtc", "rtc [OPTS]", "control ds1337 rtc (default: print time and date)", rtc_opts, rtc_exec, rtc_help),
};



static int opt_find(struct cli_opt *opts, char *opt, int is_short) {
    int i;

    //printf("ARRAY_SIZE(opts) = %d", ARRAY_SIZE(opts));
    for (i = 0; i < 4; i++) {
        if (is_short) {
            //printf(" #%d: compare %c against %c\n", i, *opt, opts[i].opt_short);
            if (opts[i].opt_short == *opt)
                return i;
        } else {
            if (strncmp(opts[i].opt_long, opt, ARG_SIZE) == 0)
                return i;
        }
    }

    return -1;
}

static int parse_opts(char *buf, struct cli_opt *opts, struct optval *vals)
{
    char *tok;
    int optind;
    char remain[512];
    int count;

    if (!opts || !vals)
        return 0;

    for (count = 0; count < 512; count++)
        remain[count] = '\0';

    count = 0;

    tok = strtok(buf, " ");
    while(tok && count < MAX_ARGS) {
        if (tok[0] == '-') {
        
            if (tok[1] == '-') {
                optind = opt_find(opts, tok+2, 0);   
            } else {
                if (strlen(tok) > 2) {
                    printf(" * invalid option %s\n", tok);
                    return -1;
                }
                optind = opt_find(opts, tok+1, 1);
            }
            if (optind >= 0) {
                switch(opts[optind].type) {
                    case OPT_INT:
                        tok = strtok(NULL, " ");
                        if (!tok) {
                            printf(" * option missing argument\n");
                            return -1;
                        }
                        vals[count].opt = opts[optind].opt_short;
                        vals[count++].v_int = (int)strtol(tok, NULL, 0);
                        break;
                    case OPT_STR:
                        tok = strtok(NULL, " ");
                        if (!tok) {
                            printf(" * option missing argument\n");
                            return -1;
                        }
                        vals[count].opt = opts[optind].opt_short;
                        strncpy(vals[count++].v_str, tok, ARG_SIZE);
                        break;
                    default:
                        vals[count++].opt = opts[optind].opt_short;
                        break;
                }
            } else {
                printf(" * invalid option!\n");
                return -1;
            }
        } else {
            remain[strlen(remain)] = ' ';
            strncpy(remain + strlen(remain), tok, ARG_SIZE);
        }

        tok = strtok(NULL, " ");
    }

    if (strlen(remain) > 0) {
        strncpy(buf, remain, strlen(remain));
    }

    return count;
}

static void rtc_help(void) {
    printf("RTC HELP\n");
}

static int rtc_exec(char *buf) {
    printf("RTC EXEC\n");
    return 0;
}

static void led_help(void) {
    printf("LED HELP\n");
}

static int led_exec(char *buf) {
    printf("LED EXEC\n");
    return 0;
}

static void i2c_help(void)
{
    printf("I2C HELP\n");
}

static int i2c_exec(char *buf)
{
    struct optval vals[MAX_ARGS];
    int nopts;
    int wflag = 0, rflag = 0;
    unsigned int addr, offset, nbytes;
    unsigned char bytes[MAX_ARGS];
    unsigned char c;
    int i;
    int ret = 0;
    char *tok;
    struct twi_packet packet;

    if (!buf)
        return -1;

    nopts = parse_opts(buf, i2c_opts, vals);
    if (nopts < 0)
        return nopts;

    for (i = 0; i < nopts; i++) {
       switch (vals[i].opt) {
            case 'q':
                quiet = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'r':
                if (wflag) {
                    printf(" * Cannot use -r and -w at the same time!\n");
                    return -1;
                }
                rflag = 1;
                break;
            case 'w':
                if (rflag) {
                    printf(" * Cannot use -r and -w at the same time!\n");
                    return -1;
                }
                wflag = 1;
                break;
            default:
                break;
       } 
    }

    if (!rflag && !wflag) {
        printf(" * -w OR -r required!\n");
        return -1;
    }
    
    tok = strtok(buf, " ");
    if (!tok) {
        printf(" * ADDR missing!\n");
        return -1;
    }
    addr = (unsigned int)strtol(tok, NULL, 0);

    tok = strtok(NULL, " ");
    if (!tok) {
        printf(" * OFF missing!\n");
        return -1;
    }
    offset = (unsigned int)strtol(tok, NULL, 0);

    tok = strtok(NULL, " ");
    if (!tok) {
        printf(" * N missing!\n");
        return -1;
    }
    nbytes = (unsigned int)strtol(tok, NULL, 0);
    if (nbytes < 1)
        printf(" * N must be greater than 0\n");
    
    if (wflag) {
        tok = strtok(NULL, " ");
        for (i = 0; tok && i < nbytes; i++, tok=strtok(NULL, " ")) {
            bytes[i] = (char)strtol(tok, NULL, 0);
        }
        if (i < nbytes) {
            printf(" * Byte(s) missing!\n");
            return -1;
        } 
    }

    packet.addr = addr;
    packet.data = (void*)bytes;
    packet.dlen = nbytes;
    packet.olen = 1;
    packet.offset = offset;
    twi_enable();
    
    if (rflag) {
        ret = twi_read(&packet);
        if (ret) {
            if (!quiet)
                printf("i2c: error on reading %d bytes (code = %d)\n", nbytes, ret);
            return -1;
        }
        if (!quiet) {
            printf("READ: ");
            for (i = 0; i < nbytes; i++) {
                printf("%x ", (unsigned int)bytes[i]);
            }
            printf("\n");
        }
    } else {
        ret = twi_write(&packet);
        if (ret) {
            if (!quiet)
                printf("i2c: error on writing\n");
            return -1;
        }
    }

    return 0;
}

void help_help(void)
{
    printf("\nCommands available:\n");
    printf("\t* help: prints this menu\n");
    printf("\t* led: LED control\n");
    printf("\t\ton/off: LED on/off\n");
    printf("\t\tblink <ms>: blinks LED every with <ms> freq\n");
    printf("\t* gettime: get DS1337 time\n");
    printf("\t* settime hh:mm:ss: set DS1337 time\n\n");
}

static int help_exec(char *buf)
{
    char *cmd;
    int i;

    if (!buf)
        return -1;

    cmd = strtok(buf, " ");
    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        if (strncmp(cmd, cmds[i].name, ARG_SIZE) == 0) {
            cmds[i].help();
            return 0;
        }
    }

    help_help();

    return 0;
}

void cmd_usage(void)
{
    help_help();
}

int cmd_exec(char *buf)
{
    char *cmd;
    int i;

    cmd = strtok(buf, " ");
    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        if (strncmp(cmd, cmds[i].name, ARG_SIZE) == 0)
            return cmds[i].exec(buf + strlen(cmd) + 1);
    }

    printf(" * Invalid command. Run 'help'!\n");
    return -1;
}
