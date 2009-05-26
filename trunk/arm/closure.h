#ifndef __CLOSURE_H__
#define __CLOSURE_H__

// Simple closure abstraction for ARMv4 and up.
// The function must not be virtual, but the class may have a vptr.
//
// This code is likely similar on other platforms.
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
		_func = (Func)*(native_uint_t*)&memberfunc;
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
