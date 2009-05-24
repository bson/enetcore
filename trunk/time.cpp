#include "enetkit.h"


Time Time::_stepped = Time::FromUsec(0);
const Time Time::InfTim(-1);

// * static
uint Time::GetMonthSize(uint year, uint month) 
{
	static const int month_size[24] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
		31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	
	assert(month <= 11);
	return month_size[(IsLeapYear(year)? 12 : 0) + month];
}


void Time::ToCalendar(struct tm& tm) const
{
	const uint32_t time = GetSec();
	const uint32_t tod = time % SECS_PER_DAY;

	tm.tm_sec = tod % 60;
	tm.tm_min = (tod % 3600) / 60;
	tm.tm_hour = tod / 3600;

	uint32_t dayofyear = time / SECS_PER_DAY;
	// 1/1/1970 = Th
	// See http://www.calendarhome.com/tyc
	tm.tm_wday = (dayofyear + 4) % 7;

	// Figure out the year
	int year = 1970;			// We use the Unix epoch
	for (;;) {
		const uint yearsize = DaysInYear(year);
		if (dayofyear < yearsize) break;
		dayofyear -= DaysInYear(year);
		++year;
	}

	tm.tm_year = year - 1900;
	tm.tm_yday = dayofyear;

	// Figure out the month
	uint mon = 0;
	for (;;) {
		const uint monthsize = GetMonthSize(year, mon);
		if (dayofyear < monthsize) break;
		dayofyear -= monthsize;
		++mon;
	}
	tm.tm_mon = mon;
	tm.tm_mday = dayofyear + 1;
}
