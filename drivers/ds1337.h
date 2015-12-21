#ifndef __DS1337_H__
#define __DS1337_H__

typedef unsigned char uint8_t;

struct ds1337_time {
        unsigned int    year;
        unsigned int    month;
        unsigned int    day;
        unsigned int    hours;
        unsigned int    minutes;
        unsigned int    seconds;
};

void ds1337_init(void);
void ds1337_set_time(struct ds1337_time time);
void ds1337_get_time(struct ds1337_time *time);

#endif /* __DS1337_H__ */
