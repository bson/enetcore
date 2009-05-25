#include "enetkit.h"

Thread* _main_thread;
void* _main_thread_stack;
void* _intr_thread_stack;

extern "C" {
void coreinit();
}

void coreinit()
{
	extern char _build_rev[], _build_user[], _build_date[];

	_console.Write("Enetcore 0.1 (build #");
	_console.Write(_build_rev);
	_console.Write(" ");
	_console.Write(_build_user);
	_console.Write(") ");
	_console.Write(_build_date);
#ifdef DEBUG
	extern char _build_url[], _build_config[];

	_console.Write("\r\nsvn: ");
	_console.Write(_build_url);
	_console.Write("\r\nconfig: ");
	_console.Write(_build_config);
#endif
	_console.Write("\r\nCopyright (c) 2009 Jan Brittenson\r\nAll Rights Reserved\r\n\n");
//	_console.SyncDrain();

	void *sp;
	asm volatile("mov %0, sp" : "=r" (sp) : : "memory");
	DMSG("Main thread stack at %p (sp=%p); interrupt thread stack at %p",
		 _main_thread_stack, sp, _intr_thread_stack);

	DMSG("Random uint: 0x%x", Util::Random<uint>());

//	_net_thread = new Thread(NetThread, NULL, NET_THREAD_STACK);
}
