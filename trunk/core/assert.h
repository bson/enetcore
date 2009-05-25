#ifndef __ASSERT_H__
#define __ASSERT_H__

void AssertFailed(const char* expr, const char* file, int linenum) __noreturn;
void WaitForDebugger() __noreturn;

// Panic
#ifdef DEBUG
#define panic(MSG)  PanicStop((const uchar*)(MSG), __FILE__, __LINE__)
void PanicStop(const uchar* msg, const char* file, int linenum) __noreturn;
#else
#define panic(MSG) PanicStop((const uchar*)(MSG))
void PanicStop(const uchar* msg) __noreturn;
#endif


#ifdef DEBUG
#define assert(EXPR)  ((EXPR) || (AssertFailed(#EXPR, __FILE__, __LINE__), 0))
#else
#define assert(EXPR)  0
#endif

#endif // __ASSERT_H__
