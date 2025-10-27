/**
 * @file acp_config.h
 * @brief ACP library build configuration and feature detection
 *
 * This header provides compile-time configuration options and feature
 * detection for the ACP library. It's automatically generated or configured
 * based on the build system settings.
 */

#ifndef ACP_CONFIG_H
#define ACP_CONFIG_H

/* Version information (set by build system) */
#ifndef ACP_VERSION_MAJOR
#define ACP_VERSION_MAJOR 0
#endif
#ifndef ACP_VERSION_MINOR
#define ACP_VERSION_MINOR 3
#endif
#ifndef ACP_VERSION_PATCH
#define ACP_VERSION_PATCH 0
#endif

/* Platform detection */
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define ACP_PLATFORM_WINDOWS 1
#define ACP_PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#define ACP_PLATFORM_MACOS 1
#define ACP_PLATFORM_NAME "macOS"
#endif
#elif defined(__linux__)
#define ACP_PLATFORM_LINUX 1
#define ACP_PLATFORM_NAME "Linux"
#elif defined(__unix__) || defined(__unix)
#define ACP_PLATFORM_UNIX 1
#define ACP_PLATFORM_NAME "Unix"
#else
#define ACP_PLATFORM_UNKNOWN 1
#define ACP_PLATFORM_NAME "Unknown"
#endif

/* Compiler detection */
#if defined(__GNUC__)
#define ACP_COMPILER_GCC 1
#define ACP_COMPILER_NAME "GCC"
#define ACP_COMPILER_VERSION __GNUC__
#elif defined(__clang__)
#define ACP_COMPILER_CLANG 1
#define ACP_COMPILER_NAME "Clang"
#define ACP_COMPILER_VERSION __clang_major__
#elif defined(_MSC_VER)
#define ACP_COMPILER_MSVC 1
#define ACP_COMPILER_NAME "MSVC"
#define ACP_COMPILER_VERSION _MSC_VER
#else
#define ACP_COMPILER_UNKNOWN 1
#define ACP_COMPILER_NAME "Unknown"
#define ACP_COMPILER_VERSION 0
#endif

/* Architecture detection */
#if defined(__x86_64__) || defined(_M_X64)
#define ACP_ARCH_X86_64 1
#define ACP_ARCH_NAME "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#define ACP_ARCH_X86 1
#define ACP_ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ACP_ARCH_ARM64 1
#define ACP_ARCH_NAME "ARM64"
#elif defined(__arm__) || defined(_M_ARM)
#define ACP_ARCH_ARM 1
#define ACP_ARCH_NAME "ARM"
#else
#define ACP_ARCH_UNKNOWN 1
#define ACP_ARCH_NAME "Unknown"
#endif

/* Feature detection */

/* C99 standard library features */
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
#define ACP_HAVE_C99 1
#endif
#if __STDC_VERSION__ >= 201112L
#define ACP_HAVE_C11 1
#endif
#endif

/* Thread support */
#if defined(__STDC_NO_THREADS__)
#define ACP_HAVE_THREADS 0
#elif defined(_POSIX_THREADS) || defined(PTHREAD_H)
#define ACP_HAVE_THREADS 1
#define ACP_HAVE_PTHREADS 1
#elif defined(_WIN32)
#define ACP_HAVE_THREADS 1
#define ACP_HAVE_WIN32_THREADS 1
#else
#define ACP_HAVE_THREADS 0
#endif

/* Memory features */
#ifndef ACP_NO_HEAP
#define ACP_ENABLE_HEAP 1
#else
#define ACP_ENABLE_HEAP 0
#endif

/* Endianness detection */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ACP_LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ACP_BIG_ENDIAN 1
#endif
#elif defined(_WIN32)
#define ACP_LITTLE_ENDIAN 1
#elif defined(__APPLE__)
#include <machine/endian.h>
#if BYTE_ORDER == LITTLE_ENDIAN
#define ACP_LITTLE_ENDIAN 1
#elif BYTE_ORDER == BIG_ENDIAN
#define ACP_BIG_ENDIAN 1
#endif
#endif

/* Build type detection */
#ifdef NDEBUG
#define ACP_BUILD_RELEASE 1
#define ACP_BUILD_TYPE "Release"
#else
#define ACP_BUILD_DEBUG 1
#define ACP_BUILD_TYPE "Debug"
#endif

/* Library type being built */
#ifdef ACP_BUILDING_SHARED
#define ACP_LIBRARY_SHARED 1
#define ACP_LIBRARY_TYPE "Shared"
#else
#define ACP_LIBRARY_STATIC 1
#define ACP_LIBRARY_TYPE "Static"
#endif

/* Compiler attributes and optimizations */
#if defined(ACP_COMPILER_GCC) || defined(ACP_COMPILER_CLANG)
#define ACP_LIKELY(x) __builtin_expect(!!(x), 1)
#define ACP_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define ACP_INLINE inline __attribute__((always_inline))
#define ACP_NOINLINE __attribute__((noinline))
#define ACP_PURE __attribute__((pure))
#define ACP_CONST __attribute__((const))
#define ACP_MALLOC __attribute__((malloc))
#elif defined(ACP_COMPILER_MSVC)
#define ACP_LIKELY(x) (x)
#define ACP_UNLIKELY(x) (x)
#define ACP_INLINE __forceinline
#define ACP_NOINLINE __declspec(noinline)
#define ACP_PURE
#define ACP_CONST
#define ACP_MALLOC __declspec(restrict)
#else
#define ACP_LIKELY(x) (x)
#define ACP_UNLIKELY(x) (x)
#define ACP_INLINE inline
#define ACP_NOINLINE
#define ACP_PURE
#define ACP_CONST
#define ACP_MALLOC
#endif

/* Configuration validation */
#if !defined(ACP_HAVE_C99)
#error "ACP requires C99 or later"
#endif

/* Configuration summary macro */
#define ACP_CONFIG_STRING \
    "ACP " ACP_STRINGIFY(ACP_VERSION_MAJOR) "." ACP_STRINGIFY(ACP_VERSION_MINOR) "." ACP_STRINGIFY(ACP_VERSION_PATCH) " (" ACP_PLATFORM_NAME "/" ACP_ARCH_NAME "/" ACP_COMPILER_NAME "/" ACP_BUILD_TYPE ")"

/* Helper macros */
#define ACP_STRINGIFY_HELPER(x) #x
#define ACP_STRINGIFY(x) ACP_STRINGIFY_HELPER(x)

#endif /* ACP_CONFIG_H */
