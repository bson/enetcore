
// Enetcore configuration parameters

#define VARIANT "Enetcore"

#define USE_LITERALS			// Optimize dup/free of literals

// #define GCC_TLS

// ConnectedSocket timeout
enum { SHUTDOWN_TIMEOUT = 10000 };


// dlmalloc config - see dlmalloc.h
#define USE_LOCKS 1
#define MLOCK_T Spinlock
#define INITIAL_LOCK(l) ((void)0)
#define ACQUIRE_LOCK(l) ((l)->Lock(), 0)
#define RELEASE_LOCK(l) ((l)->Unlock(), 0)
#define LOCK_INITIALIZER

#define HAVE_MORECORE 1
#define MORECORE(X)  _malloc_region.GetCore((X), MALLOC_REGION_SIZE)
#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define malloc_getpagesize _malloc_region.GetPageSize()
#define DEFAULT_TRIM_THRESHOLD 16384 /* Trim is dirt cheap */
#define MORECORE_CANNOT_TRIM 1		 // But even cheaper is not to do it at all
#define INSECURE 0					 // Integrity checks
#define MSPACES 0					 // No malloc spaces
#define LACKS_ERRNO_H 1
#define LACKS_STRING_H 1
#define LACKS_UNISTD_H 1
#define LACKS_SYS_MMAN_H 1
#define LACKS_STDLIB_H 1
#define LACKS_FCNTL_H 1
#define LACKS_STDIO_H 1
#define LACKS_SYS_TYPES_H 1
#define LACKS_SBRK 1
#define MALLOC_FAILURE_ACTION

#ifdef DEBUG
#define MALLOC_DEBUG				 // Extra debug checking
#endif

// HashTable default reservation, eviction depth (for cuckoo)
// See hashtable.h for more info
enum { HASHTABLE_DEFAULT_RESERVE = 64 };
enum { HASHTABLE_EVICTION_DEPTH = 10 };

// HTTP defaults
enum { HTTP_LISTEN_PORT = 8080 }; // Default inbound
enum { HTTP_DEFAULT_PORT = 80 };  // Default outbound

#define HTTP_SERVER "EnetKit/1.0" // Server string
#define HTTP_CLIENT "EnetKit/1.0" // Client string

enum { HTTP_PIPELINE_DEPTH = 4 }; // For client

enum { HTTP_DISC_TIMER = 10 };		// Idle connection timeout
enum { HTTP_MAX_WAIT_TIMER = 120 };	// Non-idle timeout

// Failure codes specific to HTTP
enum { HTTP_CONN_TIMEOUT = 1001 };

enum { MAIN_THREAD_STACK = 1400 };
enum { INTR_THREAD_STACK = 1400 };

enum { NET_THREAD_STACK = 1400 };	  // Network thread stack size
enum { NET_THREAD_PRIORITY = 200 };	  // Network thread priority

enum { THREAD_DEFAULT_STACK = 1400 }; // Default thread stack size
enum { THREAD_DEFAULT_PRIORITY = 128 }; // Default thread priority

// IOScheduler settings
enum { IO_FUDGE = 10 };	  // I/O scheduler timing granularity, in msec
enum { IO_STACK_SIZE = 2048 };	// Stack size for I/O threads

// String
#define STRING_FREELIST			// Enable freelisting of String objects

// Task priorities
// Add or remove as needed, but PRIO_NORM is required even if it's the
// only priority.
enum {
	PRIO_INTR = 0,				//  Highest - interrupt thread
	PRIO_HIGH,
	PRIO_NORM,
	PRIO_LOW,
	NUM_PRIO					// Lowest
};


// Watchdog ping interval
enum { WATCHDOG_PING_INTERVAL = 5 };	// Time between pings, in seconds

