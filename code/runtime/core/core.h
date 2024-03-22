#ifndef CORE_H
#define CORE_H

#if defined(_MSC_VER)
// Win32
#define OS_WIN32
#define COMPILER_MSVC
    
#if defined(_M_X64)
#define CPU_X64
#define ENVIRONMENT64
#elif defined(_M_IX86)
#define CPU_X86
#define ENVIRONMENT32
#else
#error "Unrecognized platform!"
#endif
#elif defined(__GNUC__)
// GCC Compiler family
#define COMPILER_GCC

#if defined(__APPLE__)
#define OS_OSX
#define OS_POSIX
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#define OS_IOS
#endif
#endif

#if defined(__FreeBSD__)
#define OS_POSIX
#define KERNEL_FREEBSD
#endif

#if defined(__linux__)
#define OS_POSIX
#define KERNEL_LINUX
#endif

#if defined(__MACH__)
#define KERNEL_MACH
#endif

#if defined(__MINGW32__) || defined(_MINGW64__)
#define OS_MINGW
#define OS_POSIX
#endif

#if defined(__ANDROID__ )
#define OS_ANDROID
#endif

#if defined(__x86_64__)
#define CPU_X64
#define ENVIRONMENT64
#elif defined(__i386__)
#define CPU_X86
#define ENVIRONMENT32
#elif defined(__arm__)
#define CPU_ARM
#define ENVIRONMENT32
#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
// ARMv7
#define CPU_ARM_VERSION 7
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
// ARMv6
#define CPU_ARM_VERSION 6
#else
// Could support earlier ARM versions at some point using compiler barriers and swp instruction
#error "Unrecognized ARM CPU architecture version!"
#endif
#if defined(__thumb__)
// Thumb instruction set mode
#define CPU_ARM_THUMB 1
#endif
#elif defined(__aarch64__)
#define CPU_AARCH64
#define ENVIRONMENT64
#define CPU_ARM_VERSION 8
#elif defined(__powerpc__) || defined(__POWERPC__) || defined(__PPC__)
#define CPU_POWERPC 1
#if defined(__powerpc64__)
#define ENVIRONMENT64 8
#else
#define ENVIRONMENT32 4 // 32-bit architecture
#endif
#else
#error "Unrecognized target CPU!"
#endif
#else
#error "Unrecognized compiler!"
#endif

#if defined(ENVIRONMENT32)
#define PTR_SIZE 4
#elif defined(ENVIRONMENT64)
#define PTR_SIZE 8
#else
#error "Invalid bitness!"
#endif

#include <stdint.h>
#include <wchar.h>
#include <float.h>
#include <stb_sprintf.h>
#include <ak_atomic.h>

#define DEFAULT_ALIGNMENT (PTR_SIZE*2)

//Assertions are programmer only state checking macros. If the assertion fails, this will open the 
//debugger. If there is no debugger running this will crash the program.
#ifdef DEBUG_BUILD
# include <assert.h>
# define Assert_With_Message(c, message) assert(c)
# define Assert_False_With_Message(c, message) assert(!(c))
# define Assert(c) assert(c)
# define Assert_False(c) assert(!(c))
#else
# define Assert_With_Message(c, message)
# define Assert_False_With_Message(c, message)
# define Assert(c)
# define Assert_False(c)
#endif

#define Null_Check(ptr) Assert_With_Message((ptr) != nullptr, "Null pointer!")
#define Not_Implemented() Assert_With_Message(false, "Not Implemented!")
#define Invalid_Default_Case() default: { Not_Implemented(); } break
#define Invalid_Code() Assert_With_Message(false, "Invalid code!")

//Generic link list macros
#define SLL_Pop_Front_N(First, Next) (First = First->Next)
#define SLL_Push_Front_N(First, Node, Next) (Node->Next = First, First = Node)

#define SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))
#define SLL_Push_Front(First, Node) SLL_Push_Front_N(First, Node, Next)
#define SLL_Pop_Front(First) SLL_Pop_Front_N(First, Next)


#define DLL_Push_Back_NP(First, Last, Node, Next, Prev) (!(First) ? ((First) = (Last) = (Node)) : ((Node)->Prev = (Last), (Last)->Next = (Node), (Last) = (Node)))
#define DLL_Remove_NP(First, Last, Node, Next, Prev) \
    do { \
        if(First == Node) { \
            First = First->Next; \
            if(First) First->Prev = NULL; \
        } \
        if(Last == Node) { \
            Last = Last->Prev; \
            if(Last) Last->Next = NULL; \
        } \
        if(Node->Prev) Node->Prev->Next = Node->Next; \
        if(Node->Next) Node->Next->Prev = Node->Prev; \
        Node->Prev = NULL; \
        Node->Next = NULL; \
    } while(0)

#define DLL_Insert_After_NP(First, Last, Target, Node, Next, Prev) \
    do { \
        if(Target->Next) { \
            Target->Next->Prev = Node; \
            Node->Next = Target->Next; \
        } \
        else { \
            Assert(Target == Last, "Invalid"); \
            Last = Node; \
        } \
        Node->Prev = Target; \
        Target->Next = Node; \
    } while(0)

#define DLL_Insert_Prev_NP(First, Last, Target, Node, Next, Prev) \
    do { \
        if(Target->Prev) { \
            Target->Prev->Next = Node; \
            Node->Prev = Target->Prev; \
        } \
        else { \
            Assert(Target == First, "Invalid"); \
            First = Node; \
        } \
        Node->Next = Target; \
        Target->Prev = Node; \
    } while(0)

#define DLL_Push_Back(First, Last, Node) DLL_Push_Back_NP(First, Last, Node, Next, Prev)
#define DLL_Remove(First, Last, Node) DLL_Remove_NP(First, Last, Node, Next, Prev)

//Some generic macros
#define Array_Count(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Clamp(min, v, max) Min(Max(min, v), max)
#define Abs(a) ((a) < 0 ? -(a) : (a))
#define Is_Pow2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define Sign_Of(a) ((a < 0) ? -1 : 1)
#define Reverse_Sign_Of(a) ((a < 0) ? 1 : -1)
#define Is_Positive_Sign(a) (Sign_Of(a) == 1)
#define Sq(v) ((v)*(v))
#define KB(x) ((x)*1024)
#define MB(x) (KB(x)*1024)
#define Align_Pow2(value, alignment) (((value) + (alignment)-1) & ~((alignment)-1))
#define _Stringify(x) #x
#define Stringify(x) _Stringify(x)

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

typedef s32 b32; 

typedef float     f32;
typedef double    f64;
typedef uintptr_t uptr;
typedef intptr_t  sptr;

#define local_persist static
#define internal static
#define global static

#include "utility.h"
#include "strings.h"
#include "hash.h"
#include "memory.h"
#include "threads.h"
#include "datetime.h"
#include "log.h"
#include "fixed_array.h"
#include "array.h"

struct allocator_tracker_manager {
	ak_mutex 		   TrackAllocLock;
	ak_mutex           TrackTreeLock;
	arena*             TrackerArena;
	allocator_tracker* FirstFreeTracker;
	allocator_tracker* RootTracker;
};

struct core {
    virtual_allocator*        VirtualAllocator;
    lock_allocator*           MainAllocator;
    thread_manager*           ThreadManager;
    log_manager*              LogManager;
    allocator_tracker_manager AllocatorTrackerManager;
};

core* 	   	   Core_Create();
void  	   	   Core_Delete();
allocator* 	   Core_Get_Base_Allocator();
ak_job_system* Core_Create_Job_System(uint32_t MaxJobCount, uint32_t NumThreads, uint32_t NumDependencies);
void           Core_Delete_Job_System(ak_job_system* JobSystem);
core* 	       Core_Get();
void  	   	   Core_Set(core* Core);


#endif