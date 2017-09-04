#ifndef __TIME_H__
#define __TIME_H__

#define MIN 60
#define HOUR (60 * MIN)
#define DAY (HOUR * 24)

typedef unsigned int unixstamp_t;

struct time_v {
	int year;
	int mon;
	int day;
	int h;
	int m;
	int s;
};

unixstamp_t time2unixstamp(struct time_v *tm);
int unixstamp2time(unixstamp_t us, struct time_v *tm);

#endif

