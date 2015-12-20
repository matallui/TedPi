#include "../clib.h"
#include "../twi.h"
#include "ds1337.h"

#define DS1337_TWI_ADDR 0x68
//#define DS1337_TWI_ADDR 0xD0

struct ds1337_time_data {
    union {
        struct {
            uint8_t _seconds        : 4,
                    _10_secs        : 3,
                    _reserved       : 1;
        };
        uint8_t seconds;
    };
    union {
        struct {
            uint8_t _minutes        : 4,
                    _10_mins        : 3,
                    _reserved_1     : 1;
        };
        uint8_t minutes;
    };
    union {
        struct {
            uint8_t _hours          : 4,
                    _10_hours       : 2,
                    _12_24_mode     : 1,    // always 0 for 24h
                    _reserved_2     : 1;
        };
        uint8_t hours;
    };
};


static void time_to_data(struct ds1337_time time, struct ds1337_time_data *data)
{
    // translate seconds to data
    data->seconds = 0;
    while (time.seconds >= 10) {
        data->_10_secs++;
        time.seconds -= 10;
    }
    //data->seconds = data->seconds << 4;
    data->_seconds += time.seconds;

    // translate minutes to data
    data->minutes = 0;
    while (time.minutes >= 10) {
        data->_10_mins++;
        time.minutes -= 10;
    }
    data->_minutes += time.minutes;

    // translate hours to data
    data->hours = 0;
    while (time.hours >= 10) {
        data->_10_hours++;
        time.hours -= 10;
    }
    data->_hours += time.hours;
}

static void data_to_time(struct ds1337_time_data data, struct ds1337_time *time)
{
    time->seconds = data._seconds + data._10_secs * 10;
    time->minutes = data._minutes + data._10_mins * 10;
    time->hours = data._hours + data._10_hours * 10;
}

static void twi_set_time(struct ds1337_time time)
{
    int r;
    struct ds1337_time_data data = {};
    unsigned char buffer[5];

    struct twi_packet packet = {
        .addr           = DS1337_TWI_ADDR,
        .data           = (void*)buffer,
        .dlen           = 3,
        .offset         = 0,
        .olen           = 1,
    };

    twi_enable();

    time_to_data(time, &data);
    buffer[0] = data.seconds;
    buffer[1] = data.minutes;
    buffer[2] = data.hours;

    r = twi_write(&packet);
    if (r)
        printf(" * Error: twi_write (code: %d)\n", r);
}

static void twi_get_time(struct ds1337_time *time)
{
    int r;
    struct ds1337_time_data data = {};
    unsigned char buffer[5];

    struct twi_packet packet = {
        .addr           = DS1337_TWI_ADDR,
        .data           = (void*)buffer,
        .dlen           = 3,
        .offset         = 0,
        .olen           = 1,
    };

    twi_enable();
    r = twi_read(&packet);
    
    if (r)
        printf(" * Error: twi_read (code: %d)\n", r);

    data.seconds = buffer[0];
    data.minutes = buffer[1];
    data.hours = buffer[2];
    data_to_time(data, time);
}

void ds1337_init(void)
{
}

void ds1337_set_time(struct ds1337_time time)
{
    twi_set_time(time);
}

void ds1337_get_time(struct ds1337_time *time)
{
    twi_get_time(time);
}


