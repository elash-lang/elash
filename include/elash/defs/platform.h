#pragma once

//////// Operating systems & Standards //////////
#if defined(__linux__) || defined(__unix__) || defined(__unix) || \
    (defined(__APPLE__) && defined(__MACH__)) || defined(__CYGWIN__)
    #define EL_PLATFORM_IS_POSIX 1
#else
    #define EL_PLATFORM_IS_POSIX 0
#endif

#if defined(__APPLE__)
    #define EL_PLATFORM_IS_APPLE 1

    #include <TargetConditionals.h>
    #if TARGET_OS_OSX
        #define EL_PLATFORM_IS_MACOS 1
    #else
        #define EL_PLATFORM_IS_MACOS 0
    #endif
#else
    #define EL_PLATFORM_IS_APPLE 0
    #define EL_PLATFORM_IS_MACOS 0
#endif

#if defined(__linux__)
    #define EL_PLATFORM_IS_LINUX 1
#else
    #define EL_PLATFORM_IS_LINUX 0
#endif

#if defined(_WIN32)
    #define EL_PLATFORM_IS_WINDOWS 1
#else
    #define EL_PLATFORM_IS_WINDOWS 0
#endif

///////// CPU Architectures /////////
#define EL_ARCH_UNKNOWN 0
#define EL_ARCH_X86_64  1
#define EL_ARCH_X86_32  2
#define EL_ARCH_ARM64   3
#define EL_ARCH_ARM32   4
#define EL_ARCH_RISCV64 5
#define EL_ARCH_RISCV32 6
#define EL_ARCH_PPC64   7
#define EL_ARCH_PPC32   8

#if defined(__x86_64__) || defined(_M_X64)
    #define EL_PLATFORM_ARCH EL_ARCH_X86_64

#elif defined(__i386__) || defined(_M_IX86)
    #define EL_PLATFORM_ARCH EL_ARCH_X86_32

#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #define EL_PLATFORM_ARCH EL_ARCH_ARM64

#elif defined(__arm__) || defined(_M_ARM)
    #define EL_PLATFORM_ARCH EL_ARCH_ARM32

#elif defined(__riscv)
    #if __riscv_xlen == 64
        #define EL_PLATFORM_ARCH EL_ARCH_RISCV64
    #elif __riscv_xlen == 32
        #define EL_PLATFORM_ARCH EL_ARCH_RISCV32
    #else
        #define EL_PLATFORM_ARCH EL_ARCH_UNKNOWN
    #endif

#elif defined(__powerpc64__) || defined(__ppc64__)
    #define EL_PLATFORM_ARCH EL_ARCH_PPC64

#elif defined(__powerpc__) || defined(__ppc__)
    #define EL_PLATFORM_ARCH EL_ARCH_PPC32

#else
    #define EL_PLATFORM_ARCH EL_ARCH_UNKNOWN
#endif

#define EL_PLATFORM_IS_X86_FAMILY \
    (EL_PLATFORM_ARCH == EL_ARCH_X86_64 || \
     EL_PLATFORM_ARCH == EL_ARCH_X86_32)

#define EL_PLATFORM_IS_ARM_FAMILY \
    (EL_PLATFORM_ARCH == EL_ARCH_ARM64 || \
     EL_PLATFORM_ARCH == EL_ARCH_ARM32)

#define EL_PLATFORM_IS_RISCV_FAMILY \
    (EL_PLATFORM_ARCH == EL_ARCH_RISCV64 || \
     EL_PLATFORM_ARCH == EL_ARCH_RISCV32)

#define EL_PLATFORM_IS_PPC_FAMILY \
    (EL_PLATFORM_ARCH == EL_ARCH_PPC64 || \
     EL_PLATFORM_ARCH == EL_ARCH_PPC32)
