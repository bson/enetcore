#ifndef __TIME_H__
#define __TIME_H__

#ifdef ENETCORE
#define TIME_RESOLUTION 20
#define TIMEBASE (1 << TIME_RESOLUTION)
#define HZ 64
#else
#define TIMEBASE 1000000
#endif

#ifndef POSIX
struct tm {
	uint tm_sec;     // seconds (0 - 60)
	uint tm_min;     // minutes (0 - 59)
	uint tm_hour;    // hours (0 - 23)
	uint tm_mday;    // day of month (1 - 31)
	uint tm_mon;     // month of year (0 - 11)
	uint tm_year;    // year - 1900
	uint tm_wday;    // day of week (Sunday = 0)
	uint tm_yday;    // day of year (0 - 365)
};
#endif

class Time {
	uint64_t _t;			// Time in usec

	static Time __coredata _stepped; // Tracks backwards time steps

public:
#ifdef POSIX
	Time(const struct timeval& tv, int) : _t((int64_t)tv.tv_sec * 1000000 + tv.tv_usec) { }
	Time(const struct timespec& ts) : _t((int64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000) { }
#endif

	Time(int64_t t) : _t(t) { }
	Time() : _t(0) { }
	~Time() { }

	Time& assign(const Time& arg) { _t = arg._t; return *this; }
	Time(const Time& arg) : _t(arg._t) {  }
	Time& operator=(const Time& arg) { return assign(arg); }

	Time operator+(const Time& rhs) const { return _t + rhs._t; }
	Time operator-(const Time& rhs) const { return _t - rhs._t; }
	uint operator/(const Time& rhs) const { return _t / rhs._t; }
	Time& operator+=(const Time& rhs) { _t += rhs._t; return *this; }
	Time& operator-=(const Time& rhs) { _t -= rhs._t; return *this; }

	bool operator==(const Time& rhs) const { return _t == rhs._t; }
	bool operator!=(const Time& rhs) const { return _t != rhs._t; }
	bool operator<(const Time& rhs) const { return _t < rhs._t; }
	bool operator<=(const Time& rhs) const { return _t <= rhs._t; }
	bool operator>(const Time& rhs) const { return _t > rhs._t; }
	bool operator>=(const Time& rhs) const { return _t >= rhs._t; }


#ifdef POSIX
	const struct timeval GetTV() const {
		struct timeval tv = { _t / 1000000, _t % 1000000 };
		return tv;
	}

	const struct timespec GetTS() const {
		struct timespec ts = { _t / 1000000, (_t % 1000000) * 1000 };
		return ts;
	}
#endif

	time_t GetPosixTime() const { return GetSec(); }
	int64_t GetSec() const { return _t / TIMEBASE; }
#ifdef POSIX
	int64_t GetMsec() const { return _t / 1000; }
#else
	int64_t GetMsec() const { return _t * 1000 / TIMEBASE; }
#endif
	// This only retains 34.20 precision if TIMEBASE != 1000000
	int64_t GetUsec() const { return _t * 1000000 / TIMEBASE; }

	static Time Now() { return _clock.GetTime(); }

	static const Time FromNsec(int64_t nsec) { return FromMsec(nsec * 1000); }
	static const Time FromUsec(int64_t usec) { return usec * TIMEBASE / 1000000; }
	static const Time FromMsec(int64_t msec) { return msec * (TIMEBASE / 1000); }
	static const Time FromSec(uint sec) { return (uint64_t)sec * (uint64_t)TIMEBASE; }

#ifdef POSIX
	// Detected backwards time step
	static void DetectedStep(uint usec) { _stepped._t += usec; }
#endif

#ifndef POSIX
	void ToCalendar(struct tm& tm) const;
#endif

	static const Time __coredata InfTim;
private:
#ifndef POSIX
	enum { SECS_PER_DAY = 24*60*60 };

	static bool IsLeapYear(uint year) {
		return !(year & 3) && ((year % 100) || !(year % 400));
	}
	static uint DaysInYear(uint year) { return IsLeapYear(year) ? 366 : 365; }
	static uint GetMonthSize(uint year, uint month);
#endif
};


inline void udelay(uint usec) {
	for (Time x = Time::Now() + Time::FromUsec(usec); Time::Now() < x; ) ;
}

#endif // __TIME_H__

