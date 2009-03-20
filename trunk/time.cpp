#include "enetkit.h"
#include "timer.h"


Time Time::_stepped = Time::FromUsec(0);
const Time Time::InfTim(-1);
