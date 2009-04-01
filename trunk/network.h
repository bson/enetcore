#ifndef __NETWORK_H__
#define __NETWORK_H__

struct InterfaceInfo {
	String _name;			// e.g. "en0"
	NetAddr _addr;			// Configured address (port == 0)
	NetAddr _mask;			// Netmask
	uint16_t _mtu;			// MTU
	bool _up:1;				// Interface is enabled
	bool _loopback:1;		// Interface is loopback
};

const Vector<InterfaceInfo*>& GetNetworkInterfaces();

// Utility function to get MAC address of first interface
bool GetIfMacAddr(const String& interface, uint8_t macaddr[6]);
bool GetMacAddr(uint8_t macaddr[6]);

#endif // __NETWORK_H__
