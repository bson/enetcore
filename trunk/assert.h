#ifndef __ASSERT_H__
#define __ASSERT_H__

void NORETURN AssertFailed(const char* expr, const char* file, int linenum);
void NORETURN WaitForDebugger();

// Panic
#ifdef DEBUG
#define panic(MSG)  PanicStop((const uchar*)(MSG), __FILE__, __LINE__)
void NORETURN PanicStop(const uchar* msg, const char* file, int linenum);
#else
#define panic(MSG) PanicStop((const uchar*)(MSG))
void NORETURN PanicStop(const uchar* msg);
#endif


#ifdef DEBUG
#define assert(EXPR)  ((EXPR) || (AssertFailed(#EXPR, __FILE__, __LINE__), 0))
#else
#define assert(EXPR)  0
#endif

#endif // __ASSERT_H__
