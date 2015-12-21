#include "clib.h"
#include "twi.h"
#include "led.h"
#include "drivers/ds1337.h"
#include "cmd.h"

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define COMMAND(_name, _usage, _desc, _opts, _nopts, _exec) { \
    .name = _name, \
    .usage = _usage, \
    .desc = _desc, \
    .opts = _opts, \
    .nopts = _nopts, \
    .exec = _exec, \
    }

#define CMD_OPT(_short, _long, _usage, _desc, _type) { \
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
    int             nopts;
    int             (*exec)(char *args, struct optval *opts, int nopts);
};


extern int quiet;
extern int verbose;


void cmd_help(struct cli_cmd *cmd);
void cmd_usage(void);

static int help_exec(char *args, struct optval *vals, int nvals);
static int i2c_exec(char *args, struct optval *vals, int nvals);
static int led_exec(char *args, struct optval *vals, int nvals);
static int rtc_exec(char *args, struct optval *vals, int nvals);

static struct cli_opt i2c_opts[] = {
    CMD_OPT('q', "quiet", "-q|--quiet", "supress output messages", OPT_NONE),
    CMD_OPT('v', "verbose", "-v|--verbose", "verbose output", OPT_NONE),
    CMD_OPT('r', "read", "-r|--read ADDR OFF N", "i2c read command", OPT_NONE),
    CMD_OPT('w', "write", "-w|--write ADDR OFF N Byte0 Byte1 ...", "i2c write command", OPT_NONE),
};

static struct cli_opt led_opts[] = {
    CMD_OPT('q', "quiet", "-q|--quiet", "supress output messages", OPT_NONE),
    CMD_OPT('v', "verbose", "-v|--verbose", "verbose output", OPT_NONE),
    CMD_OPT('s', "set", "-s|--set N", "turns RPi LED on (N=1) or off (N=0)", OPT_INT),
    CMD_OPT('b', "blink", "-b|--blink MS", "blinks RPi LED with MS miliseconds interval", OPT_INT),
};

static struct cli_opt rtc_opts[] = {
    CMD_OPT('q', "quiet", "-q|--quiet", "supress output messages", OPT_NONE),
    CMD_OPT('v', "verbose", "-v|--verbose", "verbose output", OPT_NONE),
    CMD_OPT('t', "gettime", "-t|--gettime", "print ds1337 time hh:mm:ss", OPT_NONE),
    CMD_OPT('d', "getdate", "-d|--getdate", "print ds1337 date dd:mm:yyyy", OPT_NONE),
    CMD_OPT('T', "settime", "-T|--settime HH:MM:SS", "set ds1337 time", OPT_STR),
    CMD_OPT('D', "setdate", "-D|--setdate DD:MM:YYYY", "set ds1337 date", OPT_STR),
};

static struct cli_cmd cmds[] = {
    COMMAND("help", "help [CMD]", "print this menu or CMD usage", NULL, 0, help_exec),
    COMMAND("i2c", "i2c OPTS", "send i2c command", i2c_opts, ARRAY_SIZE(i2c_opts), i2c_exec),
    COMMAND("led", "led [OPTS]", "control RPi LED (default: print LED status)", led_opts, ARRAY_SIZE(led_opts), led_exec),
    COMMAND("rtc", "rtc [OPTS]", "control ds1337 rtc (default: print time and date)", rtc_opts, ARRAY_SIZE(rtc_opts), rtc_exec),
};



static int opt_find(struct cli_cmd *cmd, char *opt, int is_short) {
    int i;
    struct cli_opt *opts;
    int nopts;

    opts = cmd->opts;
    nopts = cmd->nopts;

    for (i = 0; i < nopts; i++) {
        if (is_short) {
            if (opts[i].opt_short == *opt)
                return i;
        } else {
            if (strncmp(opts[i].opt_long, opt, ARG_SIZE) == 0)
                return i;
        }
    }

    return -1;
}

static int parse_opts(char *str, struct cli_cmd *cmd, struct optval *vals)
{
    char *tok;
    int optind;
    char remain[STR_MAX];
    int count;
    struct cli_opt *opts;

    if (!cmd || !vals || !str || str[0] == '\0')
        return 0;

    opts = cmd->opts;
    if (!opts)
        return 0;

    for (count = 0; count < STR_MAX; count++)
        remain[count] = '\0';

    count = 0;

    tok = strtok(str, " ");
    while(tok && count < MAX_ARGS) {
        if (tok[0] == '-') {
        
            if (tok[1] == '-') {
                optind = opt_find(cmd, tok+2, 0);   
            } else {
                if (strlen(tok) > 2) {
                    printf(" * invalid option %s\n", tok);
                    return -1;
                }
                optind = opt_find(cmd, tok+1, 1);
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
        strncpy(str, remain, strlen(remain));
        str[strlen(remain)] = '\0';
    }

    return count;
}


static int rtc_exec(char *args, struct optval *opts, int nopts) {
    struct ds1337_time time;
    int i;
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;
    int tflag = 0;
    int dflag = 0;
    int Tflag = 0;
    int Dflag = 0;
    
    for (i = 0; i < nopts; i++) {
       switch (opts[i].opt) {
            case 'q':
                quiet = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 't':
                if (tflag) {
                    puts(" * Repeated option!\n");
                    return -1;
                }
                tflag = 1;
                break;
            case 'd':
                if (dflag) {
                    puts(" * Repeated option!\n");
                    return -1;
                }
                dflag = 1;
                break;
            case 'T':
                if (tflag) {
                    puts(" * Repeated option!\n");
                    return -1;
                }
                sscanf(opts[i].v_str, "%d:%d:%d", &hours, &minutes, &seconds);
                Tflag = 1;
                break;
            case 'D':
                if (dflag) {
                    puts(" * Repeated option!\n");
                    return -1;
                }
                sscanf(opts[i].v_str, "%d:%d:%d", &day, &month, &year);
                Dflag = 1;
                break;
            default:
                break;
       } 
    }

    if (!tflag && !dflag && !Tflag && !Dflag) {
        puts(" * You need to provide at least one option\n");
        return -1;
    }

    if (Tflag) {
        if (hours < 0 || hours > 23) {
            printf(" * Invalid value for hours (%d)\n", hours);
            return -1;
        }
        if (minutes < 0 || minutes > 59) {
            printf(" * Invalid value for minutes (%d)\n", minutes);
            return -1;
        }
        if (seconds < 0 || seconds > 59) {
            printf(" * Invalid value for seconds (%d)\n", seconds);
            return -1;
        }
        
        time.hours = hours;
        time.minutes = minutes;
        time.seconds = seconds;

        ds1337_set_time(time);
    }

    if (tflag) {
        ds1337_get_time(&time);
        printf("-> time: %d:%d:%d\n", (int)time.hours,
                (int)time.minutes, (int)time.seconds);
    }

    return 0;
}

static int led_exec(char *args, struct optval *opts, int nopts) {
    int sflag = 0;
    int bflag = 0;
    int val = 0;
    int i;

    for (i = 0; i < nopts; i++) {
       switch (opts[i].opt) {
            case 'q':
                quiet = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                if (bflag) {
                    puts(" * Cannot use -s and -b at the same time!\n");
                    return -1;
                }

                if (sflag) {
                    puts(" * Repeated option!\n");
                    return -1;
                }
                val = opts[i].v_int;
                sflag = 1;
                break;
            case 'b':
                if (sflag) {
                    puts(" * Cannot use -s and -b at the same time!\n");
                    return -1;
                }
                if (bflag) {
                    puts(" * Repeated option!\n");
                    return -1;
                }
                val = opts[i].v_int;
                bflag = 1;
                break;
            default:
                break;
       } 
    }

    if (!sflag && !bflag) {
        puts(" * You need to specify one option!\n");
        return -1;
    }

    if (sflag) {
        led_blink_disable();
        if (val)
            led_on();
        else
            led_off();
    } else if (bflag) {
        if (val <= 0) {
            puts(" * Invalid value --blink\n");
            return -1;
        }
        led_blink(val);
    }

    return 0;
}

static int i2c_exec(char *args, struct optval *opts, int nopts)
{
    int wflag = 0, rflag = 0;
    unsigned int addr, offset, nbytes;
    unsigned char bytes[MAX_ARGS];
    int i;
    int ret = 0;
    char *tok;
    struct twi_packet packet;

    if (!args)
        return -1;

    for (i = 0; i < nopts; i++) {
       switch (opts[i].opt) {
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

    tok = strtok(args, " ");
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

static int help_exec(char *args, struct optval *opts, int nopts)
{
    char cmd[ARG_SIZE];
    int i;

    if (!args)
        goto _noargs;

    sscanf(args, "%s", cmd);
    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        if (strncmp(cmd, cmds[i].name, ARG_SIZE) != 0)
            continue;

        cmd_help(&cmds[i]);
        return 0;
    }

_noargs:
    cmd_usage();
    return 0;
}

void cmd_help(struct cli_cmd *cmd)
{
    struct cli_opt *opts;
    int nopts;
    int i;

    if (!cmd)
        return;

    opts = cmd->opts;

    printf("\nCMD\n\t%s - %s\n\n", cmd->name, cmd->desc);
    printf("USAGE\n\t%s\n\n", cmd->usage);

    if (opts) {
        nopts = cmd->nopts;
        puts("OPTS\n\n");
        for (i = 0; i < nopts; i++) {
            printf("\t%s\n\t\t%s\n", opts[i].usage, opts[i].desc);
        }
        putc('\n');
    }
}

void cmd_usage(void)
{
    int i;

    puts("\nCommands available:\n\n");
    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        printf("\t%s \t\t%s\n", cmds[i].usage, cmds[i].desc);
    }
    putc('\n');
}

int cmd_exec(char *str)
{
    struct optval opts[MAX_ARGS];
    int nopts;
    char cmd[ARG_SIZE];
    int i;
    int ret;

    if (str[0] == '\0')
        goto _invalid;

    cmd[ARG_SIZE-1] = '\0';
    sscanf(str, "%s", cmd);
    
    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        if (strcmp(cmd, cmds[i].name) != 0)
            continue;

        nopts = parse_opts(str + strlen(cmd), &cmds[i], opts);

        if (*(str+strlen(cmd)) == '\0')
            ret = cmds[i].exec(NULL, opts, nopts);
        else
            ret = cmds[i].exec(str + strlen(cmd), opts, nopts);

        return ret;
    }

_invalid:
    puts(" * Invalid command!\n");
    cmd_usage();
    return -1;
}
