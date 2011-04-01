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

#ifdef DEBUG
inline void AssertNotInterrupt() { assert(InSystemMode()); }
#else
#define AssertNotInterrupt() 0
#endif


#ifndef CHECK_BOUNDS
#  ifdef DEBUG
#    define CHECK_BOUNDS 1
#  else
#    define CHECK_BOUNDS 0
#  endif
#endif

#if CHECK_BOUNDS
#define assert_bounds(EXPR)  ((EXPR) || (AssertFailed(#EXPR, __FILE__, __LINE__), 0))
#else
#define assert_bounds(EXPR)  0
#endif


#ifndef CHECK_ALLOC
#  ifdef DEBUG
#    define CHECK_ALLOC 1
#  else
#    define CHECK_ALLOC 0
#  endif
#endif

#if CHECK_ALLOC
#define assert_alloc(EXPR)  ((EXPR) || (AssertFailed(#EXPR, __FILE__, __LINE__), 0))
#else
#define assert_alloc(EXPR)  0
#endif

#endif // __ASSERT_H__
