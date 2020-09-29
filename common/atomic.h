#ifndef _platform_atomic_h_
#define _platform_atomic_h_

#if defined(OS_WINDOWS)
#include <Windows.h>
typedef int		int32_t;
typedef __int64 int64_t;
#elif defined(OS_MAC)
#include <libkern/OSAtomic.h>
#endif

#include <assert.h>

//-------------------------------------------------------------------------------------
// int32_t atomic_increment32(volatile int32_t *value)
// int32_t atomic_decrement32(volatile int32_t *value)
// int32_t atomic_add32(volatile int32_t *value, int32_t incr)
// int atomic_cas32(volatile int32_t *value, int32_t oldvalue, int32_t newvalue)
// int atomic_cas_ptr(void* volatile *value, void *oldvalue, void *newvalue)
//-------------------------------------------------------------------------------------
// required: Windows >= Vista
// int64_t atomic_increment64(volatile int64_t *value)
// int64_t atomic_decrement64(volatile int64_t *value)
// int64_t atomic_add64(volatile int64_t *value, int64_t incr)
// int atomic_cas64(volatile int64_t *value, int64_t oldvalue, int64_t newvalue)
//-------------------------------------------------------------------------------------

#if defined(OS_WINDOWS)
inline int32_t atomic_increment32(volatile int32_t *value)
{
	assert((int32_t)value % 4 == 0);
	assert(sizeof(LONG) == sizeof(int32_t));
	return InterlockedIncrement((LONG volatile*)value);
}

inline int32_t atomic_decrement32(volatile int32_t *value)
{
	assert((int32_t)value % 4 == 0);
	assert(sizeof(LONG) == sizeof(int32_t));
	return InterlockedDecrement((LONG volatile*)value);
}

inline int32_t atomic_add32(volatile int32_t *value, int32_t incr)
{
	assert((int32_t)value % 4 == 0);
	assert(sizeof(LONG) == sizeof(int32_t));
	return InterlockedExchangeAdd((LONG volatile*)value, incr) + incr; // The function returns the initial value of the Addend parameter.
}

inline int atomic_cas32(volatile int32_t *value, int32_t oldvalue, int32_t newvalue)
{
	assert((int32_t)value % 4 == 0);
	assert(sizeof(LONG) == sizeof(int32_t));
	return oldvalue == InterlockedCompareExchange((LONG volatile*)value, newvalue, oldvalue) ? 1 : 0;
}

inline int atomic_cas_ptr(void* volatile *value, void *oldvalue, void *newvalue)
{
#if defined(_WIN64) || defined(WIN64)
	assert(0 == (int64_t)value % 8 && 0 == (int64_t)oldvalue % 8 && 0 == (int64_t)newvalue % 8);
#else
	assert(0 == (int32_t)value % 4 && 0 == (int32_t)oldvalue % 4 && 0 == (int32_t)newvalue % 4);
#endif
	return oldvalue == InterlockedCompareExchangePointer(value, newvalue, oldvalue) ? 1 : 0;
}

#if (_WIN32_WINNT == 0x0501)

inline int64_t atomic_increment64(volatile int64_t *value)
{
	assert((int32_t)value % 8 == 0);
	assert(sizeof(LONGLONG) == sizeof(int64_t));
	return InterlockedIncrement(value);
}

#else if (_WIN32_WINNT >= 0x0502)

inline int64_t atomic_increment64(volatile int64_t *value)
{
	assert((int32_t)value % 8 == 0);
	assert(sizeof(LONGLONG) == sizeof(int64_t));
	return InterlockedIncrement64(value);
}

inline int64_t atomic_decrement64(volatile int64_t *value)
{
	assert((int32_t)value % 8 == 0);
	assert(sizeof(LONGLONG) == sizeof(int64_t));
	return InterlockedDecrement64(value);
}

inline int64_t atomic_add64(volatile int64_t *value, int64_t incr)
{
	assert((int32_t)value % 8 == 0);
	assert(sizeof(LONGLONG) == sizeof(int64_t));
	return InterlockedExchangeAdd64(value, incr) + incr; // The function returns the initial value of the Addend parameter.
}

inline int atomic_cas64(volatile int64_t *value, int64_t oldvalue, int64_t newvalue)
{
	assert((int32_t)value % 8 == 0);
	assert(sizeof(LONGLONG) == sizeof(int64_t));
	return oldvalue == InterlockedCompareExchange64(value, newvalue, oldvalue) ? 1 : 0;
}

#endif /* _WIN32_WINNT >= 0x0502 */

//-------------------------------------OS_MAC------------------------------------------
#elif defined(OS_MAC)
inline int32_t atomic_increment32(volatile int32_t *value)
{
	assert((int32_t)value % 4 == 0);
	return OSAtomicIncrement32(value);
}

inline int64_t atomic_increment64(volatile int64_t *value)
{
	assert((int32_t)value % 8 == 0);
	return OSAtomicIncrement64(value);
}

inline int32_t atomic_decrement32(volatile int32_t *value)
{
	assert((int32_t)value % 4 == 0);
	return OSAtomicDecrement32(value);
}

inline int64_t atomic_decrement64(volatile int64_t *value)
{
	assert((int32_t)value % 8 == 0);
	return OSAtomicDecrement64(value);
}

inline int32_t atomic_add32(volatile int32_t *value, int32_t incr)
{
	assert((int32_t)value % 4 == 0);
	return OSAtomicAdd32(incr, value);
}

inline int64_t atomic_add64(volatile int64_t *value, int64_t incr)
{
	assert((int32_t)value % 8 == 0);
	return OSAtomicAdd64(incr, value);
}

inline int atomic_cas32(volatile int32_t *value, int32_t oldvalue, int32_t newvalue)
{
	assert((int32_t)value % 4 == 0);
	return OSAtomicCompareAndSwap32(oldvalue, newvalue, value) ? 1 : 0;
}

inline int atomic_cas64(volatile int64_t *value, int64_t oldvalue, int64_t newvalue)
{
	assert((int32_t)value % 8 == 0);
	return OSAtomicCompareAndSwap64(oldvalue, newvalue, value) ? 1 : 0;
}

inline int atomic_cas_ptr(void* volatile *value, void *oldvalue, void *newvalue)
{
#if TARGET_CPU_X86_64 || TARGET_CPU_PPC64
	assert(0 == (int32_t)value % 8 && 0 == (int32_t)oldvalue % 8 && 0 == (int32_t)newvalue % 8);
#else
	assert(0 == (int32_t)value % 4 && 0 == (int32_t)oldvalue % 4 && 0 == (int32_t)newvalue % 4);
#endif
	return OSAtomicCompareAndSwapPtr(oldvalue, newvalue, value) ? 1 : 0;
}

#else

//-------------------------------------GCC----------------------------------------
// -march=i486 32-bits
// -march=i586 64-bits
// compile with CFLAGS += -march=i586

#if __GNUC__>=4 && __GNUC_MINOR__>=1
inline int32_t atomic_increment32(volatile int32_t *value)
{
	assert((long)value % 4 == 0);
	return __sync_add_and_fetch_4(value, 1);
}

inline int32_t atomic_decrement32(volatile int32_t *value)
{
	assert((long)value % 4 == 0);
	return __sync_sub_and_fetch_4(value, 1);
}

inline int32_t atomic_add32(volatile int32_t *value, int32_t incr)
{
	assert((long)value % 4 == 0);
	return __sync_add_and_fetch_4(value, incr);
}

inline int atomic_cas32(volatile int32_t *value, int32_t oldvalue, int32_t newvalue)
{
	assert((long)value % 4 == 0);
	return __sync_bool_compare_and_swap_4(value, oldvalue, newvalue) ? 1 : 0;
}

inline int atomic_cas_ptr(void* volatile *value, void *oldvalue, void *newvalue)
{
	assert(0 == (long)value % 4 && 0 == (long)oldvalue % 4 && 0 == (long)newvalue % 4);
	return __sync_bool_compare_and_swap(value, oldvalue, newvalue);
}

inline int64_t atomic_increment64(volatile int64_t *value)
{
	assert((long)value % 8 == 0);
	return __sync_add_and_fetch_8(value, 1);
}

inline int64_t atomic_decrement64(volatile int64_t *value)
{
	assert((long)value % 8 == 0);
	return __sync_sub_and_fetch_8(value, 1);
}

inline int64_t atomic_add64(volatile int64_t *value, int64_t incr)
{
	assert((long)value % 8 == 0);
	return __sync_add_and_fetch_8(value, incr);
}

inline int atomic_cas64(volatile int64_t *value, int64_t oldvalue, int64_t newvalue)
{
	assert((long)value % 8 == 0);
	return __sync_bool_compare_and_swap_8(value, oldvalue, newvalue) ? 1 : 0;
}

//-------------------------------------ARM----------------------------------------
#elif defined(__ARM__) || defined(__arm__)

inline int32_t atomic_add32(volatile int32_t *value, int32_t incr)
{
	assert((int32_t)value % 4 == 0);
	int a, b, c;
	asm volatile(	"0:\n\t"
					"ldr %0, [%3]\n\t"
					"add %1, %0, %4\n\t"
					"swp %2, %1, [%3]\n\t"
					"cmp %0, %2\n\t"
					"swpne %1, %2, [%3]\n\t"
					"bne 0b"
					: "=&r" (a), "=&r" (b), "=&r" (c)
					: "r" (value), "r" (incr)
					: "cc", "memory");
	return a;
}

inline int32_t atomic_increment32(volatile int32_t *value)
{
	assert((int32_t)value % 4 == 0);
	return atomic_add32(value, 1);
}

inline int32_t atomic_decrement32(volatile int32_t *value)
{
	assert((int32_t)value % 4 == 0);
	return atomic_add32(value, -1);
}

//-------------------------------------INTEL----------------------------------------
#else
inline int32_t atomic_add32(volatile int32_t *value, int32_t incr)
{
	int32_t r = incr;
	assert((long)value % 4 == 0);
	asm volatile ("lock; xaddl %0,%1"
		: "+r" (r), "+m" (*value));
	return r + incr;
}

inline int32_t atomic_increment32(volatile int32_t *value)
{
	assert((long)value % 4 == 0);
	return atomic_add32(value, 1);
}

inline int32_t atomic_decrement32(volatile int32_t *value)
{
	assert((long)value % 4 == 0);
	return atomic_add32(value, -1);
}

inline int atomic_cas32(volatile int32_t *value, int32_t oldvalue, int32_t newvalue)
{
	int32_t prev = oldvalue;
	assert((long)value % 4 == 0);
	asm volatile ("lock; cmpxchgl %2, %1"
		: "+a" (prev), "+m" (*value)
		: "r" (newvalue));
	return prev == oldvalue ? 1 : 0;
}

inline int atomic_cas64(volatile int64_t *value, int64_t oldvalue, int64_t newvalue)
{
	int64_t prev = oldvalue;
#if defined(__LP64__)
	assert((long)value % 8 == 0);
	asm volatile ("lock ; cmpxchgq %2, %1"
		: "+a" (prev), "+m" (*value)
		: "q" (newvalue));
	return prev == oldvalue ? 1 : 0;
#else
	// http://src.chromium.org/svn/trunk/src/third_party/tcmalloc/chromium/src/base/atomicops-internals-x86.h
	asm volatile ("push %%ebx\n\t"
		"movl (%3), %%ebx\n\t"    // Move 64-bit new_value into
		"movl 4(%3), %%ecx\n\t"   // ecx:ebx
		"lock; cmpxchg8b (%1)\n\t"// If edx:eax (old_value) same
		"pop %%ebx\n\t"
		: "=A" (prev)             // as contents of ptr:
		: "D" (value),              //   ecx:ebx => ptr
		"0" (oldvalue),        // else:
		"S" (&newvalue)        //   old *ptr => edx:eax
		: "memory", "%ecx");
#endif
	return prev == oldvalue ? 1 : 0;
}

inline int64_t atomic_add64(volatile int64_t *value, int64_t incr)
{
#if defined(__LP64__)
	int64_t r = incr;
	assert((long)value % 8 == 0);
	asm volatile ("lock; xaddq %0,%1"
		: "+r" (r), "+m" (*value));
	return r + incr;
#else
	int64_t old_val, new_val;
	do {
		old_val = *value;
		new_val = old_val + incr;
	} while (atomic_cas64(value, old_val, new_val) != 1);
#endif
	return new_val;
}

inline int64_t atomic_increment64(volatile int64_t *value)
{
	assert((long)value % 8 == 0);
	return atomic_add64(value, 1);
}

inline int64_t atomic_decrement64(volatile int64_t *value)
{
	assert((long)value % 8 == 0);
	return atomic_add64(value, -1);
}

inline int atomic_cas_ptr(void* volatile *value, void *oldvalue, void *newvalue)
{
	void *prev = oldvalue;
#if defined(__LP64__)
	asm volatile ("lock ; cmpxchgq %2, %1"
		: "+a" (prev), "+m" (*value)
		: "q" (newvalue));
	return prev == oldvalue ? 1 : 0;
#else
	asm volatile ("lock; cmpxchgl %2, %1"
		: "+a" (prev), "+m" (*value)
		: "r" (newvalue));
#endif
	return prev == oldvalue ? 1 : 0;
}
#endif

#endif /* OS_WINDOWS */

#endif /* !_platform_atomic_h_ */
