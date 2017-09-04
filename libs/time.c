#include "time.h"

static int mon1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int mon2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

unixstamp_t get_time_unixstamp(struct time_v *tm)
{
	int ys = 0;
	ys += ((tm->year - 1970) / 4 ) * (365 * 3 + 366);
	int y_mod = (tm->year - 1970) % 4;
	switch(y_mod) {
		case 1:
			ys += 365;
			break;
		case 2:
			ys += 365 * 2;
		break;
		case 3:
			ys += 365 * 2 + 366;
		break;
	}

	int ms = 0;
	int i;
	if(tm->year % 400 == 0 || ((tm->year % 4 == 0) && (tm->year % 100 != 0))) {
		for(i = 0; i < tm->mon - 1; i++)
			ms += mon2[i];
	}
	else {
		for(i = 0; i < tm->mon - 1; i++)
			ms += mon1[i];
	}

	int ds = 0;
	ds += tm->day - 1;

	return ys * DAY + ms * DAY + ds * DAY + tm->h * HOUR + tm->m * MIN + tm->s;
}

int get_unixstamp_time(unixstamp_t us, struct time_v *tm)
{
	int days = us / DAY;
	int ys = days / (365 * 3 + 366);
	int ymod = days % (365 * 3 + 366);
	days -= (ys * (365 * 3 + 366));
	ys *= 4;
	if(ymod > 365 * 2 + 366) {
		ys += 3;
		days -= (365 * 2 + 366);
	}
	else if(ymod > 365 * 2) {
		ys += 2;
		days -= (365 * 2);
	}
	else if(ymod > 365) {
		ys += 1;
		days -= 365;
	}
	tm->year = 1970 + ys;

	int mmod = 0;
	int i;
	if(tm->year % 400 == 0 || ((tm->year % 4 == 0) && (tm->year % 100 != 0))) {
		for(i = 0; i < 12; i++) {
			if(mmod + mon1[i] > days) {
				days -= mmod;
				break;
			}
			mmod += mon2[i];
		}
	}
	else {
		for(i = 0; i < 12; i++) {
			if(mmod + mon2[i] > days) {
				days -= mmod;
				break;
			}
			mmod += mon1[i];
		}
	}
	tm->mon = i + 1;

	tm->day = days + 1;

	int sec = us % DAY;
	tm->h = sec / HOUR;
	tm->m = (sec % HOUR)/MIN;
	tm->s = sec % MIN;

	return 0;
}

