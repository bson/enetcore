#include "enetkit.h"
#include "ethernet.h"


// XXX this should really go into IP

const Vector<InterfaceInfo*>& GetNetworkInterfaces()
{
	static Vector<InterfaceInfo*> iflist;

	if (iflist.Empty()) {
#if 0
		InterfaceInfo* info = Ip::GetIfInfo(STR("eth0"));
		assert(info);
		iflist.PushBack(info);
#endif
	}

	return iflist;
}


bool GetIfMacAddr(const String& interface, uint8_t macaddr[6])
{
	if (interface.Equals(STR("eth0"))) {
		memcpy(macaddr, _eth0.GetMacAddr(), 6);
		return true;
	}

	return false;
}


bool GetMacAddr(uint8_t macaddr[6]) { return GetIfMacAddr(STR("eth0"), macaddr); }
