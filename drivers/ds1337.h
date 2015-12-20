#ifndef __DS1337_H__
#define __DS1337_H__

typedef unsigned char uint8_t;

struct ds1337_time {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
};

void ds1337_init(void);
void ds1337_set_time(struct ds1337_time time);
void ds1337_get_time(struct ds1337_time *time);

#endif /* __DS1337_H__ */
