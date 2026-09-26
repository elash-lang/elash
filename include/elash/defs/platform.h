#pragma once

#include <elash/defs/stringify.h>

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
        #define EL_OS_STRING "MacOS"
    #else
        #define EL_PLATFORM_IS_MACOS 0
    #endif
#else
    #define EL_PLATFORM_IS_APPLE 0
    #define EL_PLATFORM_IS_MACOS 0
#endif

#if defined(__linux__)
    #define EL_PLATFORM_IS_LINUX 1
    #define EL_OS_STRING "Linux"
#else
    #define EL_PLATFORM_IS_LINUX 0
#endif

#if defined(_WIN32)
    #define EL_PLATFORM_IS_WINDOWS 1
    #define EL_OS_STRING "Windows"
#else
    #define EL_PLATFORM_IS_WINDOWS 0
#endif

#ifndef EL_OS_STRING
    #define EL_OS_STRING "unknown"
#endif

/////////////// Compilers ///////////////
#define EL_COMPILER_UNKNOWN 0
#define EL_COMPILER_CLANG   1
#define EL_COMPILER_GCC     2
#define EL_COMPILER_ICX     3
#define EL_COMPILER_MSVC    4


#if defined(__ICX_COMPILER) || defined(__INTEL_LLVM_COMPILER)
    #define EL_COMPILER EL_COMPILER_ICX
    #define EL_COMPILER_NAME "icx"
    #if defined(__INTEL_LLVM_COMPILER)
        #define EL_COMPILER_VERSION_STRING EL_STRINGIFY(__INTEL_LLVM_COMPILER)
    #else
        #define EL_COMPILER_VERSION_STRING EL_STRINGIFY(__ICX_COMPILER)
    #endif

#elif defined(__clang__)
    #define EL_COMPILER EL_COMPILER_CLANG
    #define EL_COMPILER_NAME "clang"

    // __clang_version__ also contains the github url which we don't want
    #define EL_COMPILER_VERSION_STRING    \
        EL_STRINGIFY(__clang_major__) "." \
        EL_STRINGIFY(__clang_minor__) "." \
        EL_STRINGIFY(__clang_patchlevel__)

#elif defined(__GNUC__)
    #define EL_COMPILER EL_COMPILER_GCC
    #define EL_COMPILER_NAME "GCC"

    // GCC's __VERSION__ also contains some additional information but
    // it's shorter and more useful than link to the llvm-project repo
    // so that's what we actually want
    #define EL_COMPILER_VERSION_STRING __VERSION__

#elif defined(_MSC_VER)
    #define EL_COMPILER EL_COMPILER_MSVC
    #define EL_COMPILER_NAME "msvc"
    #if defined(_MSC_FULL_VER)
        #define EL_COMPILER_VERSION_STRING EL_STRINGIFY(_MSC_FULL_VER)
    #else
        #define EL_COMPILER_VERSION_STRING EL_STRINGIFY(_MSC_VER)
    #endif

#else
    #define EL_COMPILER EL_COMPILER_UNKNOWN
    #define EL_COMPILER_NAME "unknown"
    #define EL_COMPILER_VERSION_STRING "unknown"
#endif

#define EL_COMPILER_IS_CLANG   (EL_COMPILER == EL_COMPILER_CLANG)
#define EL_COMPILER_IS_GCC     (EL_COMPILER == EL_COMPILER_GCC)
#define EL_COMPILER_IS_MSVC    (EL_COMPILER == EL_COMPILER_MSVC)
#define EL_COMPILER_IS_ICX     (EL_COMPILER == EL_COMPILER_ICX)
#define EL_COMPILER_IS_UNKNOWN (EL_COMPILER == EL_COMPILER_UNKNOWN)

#define EL_GCC_EXTENSIONS (EL_COMPILER_IS_GCC || EL_COMPILER_IS_CLANG || EL_COMPILER_IS_ICX)

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
    #define EL_ARCH_STRING "x86_64"

#elif defined(__i386__) || defined(_M_IX86)
    #define EL_PLATFORM_ARCH EL_ARCH_X86_32
    #define EL_ARCH_STRING "x86_32"

#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #define EL_PLATFORM_ARCH EL_ARCH_ARM64
    #define EL_ARCH_STRING "ARM64"

#elif defined(__arm__) || defined(_M_ARM)
    #define EL_PLATFORM_ARCH EL_ARCH_ARM32
    #define EL_ARCH_STRING "ARM32"

#elif defined(__riscv)
    #if __riscv_xlen == 64
        #define EL_PLATFORM_ARCH EL_ARCH_RISCV64
        #define EL_ARCH_STRING "RISCV64"
    #elif __riscv_xlen == 32
        #define EL_PLATFORM_ARCH EL_ARCH_RISCV32
        #define EL_ARCH_STRING "RISCV32"
    #else
        #define EL_PLATFORM_ARCH EL_ARCH_UNKNOWN
        #define EL_ARCH_STRING "unknown"
    #endif

#elif defined(__powerpc64__) || defined(__ppc64__)
    #define EL_PLATFORM_ARCH EL_ARCH_PPC64
    #define EL_ARCH_STRING "PowerPC 64"

#elif defined(__powerpc__) || defined(__ppc__)
    #define EL_PLATFORM_ARCH EL_ARCH_PPC32
    #define EL_ARCH_STRING "PowerPC 32"

#else
    #define EL_PLATFORM_ARCH EL_ARCH_UNKNOWN
    #define EL_ARCH_STRING "unknown"
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

#define EL_ARCH_IS_UNKNOWN \
    (EL_PLATFORM_ARCH == EL_ARCH_UNKNOWN)
