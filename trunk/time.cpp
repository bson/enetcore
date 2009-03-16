#include "enetkit.h"


Time Time::_stepped = Time::FromUsec(0);
const Time Time::InfTim(-1);

#ifdef __APPLE__
Time Time::Now()
{
	static uint64_t epoch_delta;
	static mach_timebase_info_data_t timeinfo;
	if (!timeinfo.denom) {
		mach_timebase_info(&timeinfo);
		assert(timeinfo.denom);
		assert(timeinfo.numer);

		// mach_absolute_time() has an epoch that starts at system boot,
		// so calculate it in unix time.  This is effectively the difference
		// between the two time values.
		timeval tv;
		int e = ::gettimeofday(&tv, NULL);
		assert(!e);

		const uint64_t mat = (uint64_t)mach_absolute_time() * timeinfo.numer / timeinfo.denom;
		epoch_delta = (uint64_t)tv.tv_sec * 1000000000 + tv.tv_usec * 1000 - mat;

		DMSG("Mach time epoch: %U, MAT %U hours, %u/%u",
			 epoch_delta, mat / 1000000000 / 60 / 60, timeinfo.numer, timeinfo.denom);
		assert((uint64_t)epoch_delta > 631173600000000000LL); // 1/1/1990 00:00
	}

	const uint64_t now = mach_absolute_time() * timeinfo.numer / timeinfo.denom;
	return Time::FromNsec(now + epoch_delta) + _stepped;
}
#elif defined(POSIX)
Time Time::Now()
{
	struct timeval tv;
	const int n = gettimeofday(&tv, NULL); assert(!n);
	return Time(tv, 0) + _stepped;
}
#elif defined(ENETCORE)
Time Time::Now()
{
	return 0;
}
#endif
