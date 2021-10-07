// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __CLOSURE_H__
#define __CLOSURE_H__

// Simple closure abstraction.
// The function must not be virtual, but the instance may have a vptr.
//
// Tested on: gcc4: ARM, i386, x86_64
//
// Canonical usage:
//    class Driver {
//      public:
//           void Interrupt();
//           void Init(uint channel) {
//               _intr.InstallHandler(channel, Closure(&Interrupt, this));
//           }
//    };

#ifdef __GNUC__

class Closure {
	typedef void (*Func)(void*);

	Func _func;					// Function
	void* _cx;					// Context
public:
	template <typename T> Closure(const T& memberfunc, void* arg) {
		assert(!(((uintptr_t*)&memberfunc)[1])); // 0: absolute ptr
		_func = (Func)*(uintptr_t*)&memberfunc;
		_cx = arg;
	}

	Closure(const Closure& arg) { *this = arg; }

	Closure& operator=(const Closure& arg) {
		if (&arg != this) {
			_func = arg._func;
			_cx = arg._cx;
		}
		return *this;
	}

	void Invoke() { _func(_cx); }
private:
	Closure();
};

#else
#error "Unsupported compiler"
#endif

#endif // __CLOSURE_H__
