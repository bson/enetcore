#ifndef __TIME_H__
#define __TIME_H__


class Time {
	uint64_t _t;			// Time in usec

	static Time _stepped;	// Tracks backwards time steps

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

	Time operator+(const Time& rhs) const { return FromUsec(_t + rhs._t); }
	Time operator-(const Time& rhs) const { return FromUsec(_t - rhs._t); }
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
	int64_t GetSec() const { return _t / 1000000; }
	int64_t GetMsec() const { return _t / 1000; }
	int64_t GetUsec() const { return _t; }

	static Time Now();

	static const Time FromNsec(int64_t nsec) { return nsec / 1000; }
	static const Time FromUsec(int64_t usec) { return usec; }
	static const Time FromMsec(int64_t msec) { return msec * 1000; }
	static const Time FromSec(uint sec) { return (uint64_t)sec * 1000000LL; }

	// Detected backwards time step
	static void DetectedStep(uint usec) { _stepped._t += usec; }

	static const Time InfTim;
};


#endif // __TIME_H__

