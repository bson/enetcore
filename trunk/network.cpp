#include "enetkit.h"
#include "network.h"
#include "ip.h"
#include "dhcp.h"
#include "ethernet.h"


EventObject _net_event;
Thread* _net_thread;


// XXX move this stuff to IP

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


void* NetThread(void*)
{
	Self().SetPriority(NET_THREAD_PRIORITY);
	Self().ReadyToWork();

	BufferPool::Initialize(20, _eth0.GetBufSize());

// Moved to hwinit - needs to be done before EINT2 is enabled
//	_eth0.Initialize();

	_ip.Initialize();
	_dhcp0.Reset();

	Time dhcp_next = _dhcp0.GetServiceTime();
	Time ip_next = _ip.GetServiceTime();
	bool link = !_eth0.GetLinkStatus();

	for (;;) {
//		DMSG("NetThread: wake");

		const bool newlink = _eth0.GetLinkStatus();
		if (newlink != link) {
			link = newlink;
			DMSG("eth0: link status: %s", link ? "up" : "down");
			// if (link)  _dhcp0.Reset();
		}

		Time now = Time::Now();
		Time next = min(dhcp_next, ip_next);
		if (!link) next = min(next, now + Time::FromSec(1));

		if (now < next)
			_net_event.Wait(next - now);

		IOBuffer* packet;

		uint16_t et;
		while ((packet = _eth0.Receive(et))) {
			switch (et) {
			case ETHERTYPE_IP:
				if (!_dhcp0.Receive(packet))
					_ip.Receive(packet);
				break;
			case ETHERTYPE_ARP:
				_ip.ArpReceive(packet);
				break;
			}
			BufferPool::FreeBuffer(packet);
		}

		now = Time::Now();

		if (now >= dhcp_next) dhcp_next = _dhcp0.Service();
		if (now >= ip_next)  ip_next = _ip.Service();
	}
}
