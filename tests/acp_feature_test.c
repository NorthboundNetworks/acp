/**
 * @file acp_feature_test.c
 * @brief Build configuration and feature validation utility
 *
 * This utility displays the detected build configuration and validates
 * that all required features are available.
 */

#include <stdio.h>
#include <stdlib.h>
#include "acp_config.h"
#include "acp_protocol.h"
#include "acp_crc16.h"

int main(void)
{
    printf("ACP Library Configuration Test\n");
    printf("==============================\n\n");

    printf("Version: %d.%d.%d\n", ACP_VERSION_MAJOR, ACP_VERSION_MINOR, ACP_VERSION_PATCH);
    printf("Config String: %s\n\n", ACP_CONFIG_STRING);

    printf("Platform Information:\n");
    printf("  Platform: %s\n", ACP_PLATFORM_NAME);
    printf("  Architecture: %s\n", ACP_ARCH_NAME);
    printf("  Compiler: %s (version %d)\n", ACP_COMPILER_NAME, ACP_COMPILER_VERSION);
    printf("  Build Type: %s\n", ACP_BUILD_TYPE);
    printf("  Library Type: %s\n\n", ACP_LIBRARY_TYPE);

    printf("Feature Detection:\n");

#ifdef ACP_HAVE_C99
    printf("  ✓ C99 support available\n");
#else
    printf("  ✗ C99 support NOT available\n");
#endif

#ifdef ACP_HAVE_C11
    printf("  ✓ C11 support available\n");
#else
    printf("  - C11 support not detected\n");
#endif

#if ACP_HAVE_THREADS
    printf("  ✓ Threading support available");
#ifdef ACP_HAVE_PTHREADS
    printf(" (POSIX threads)\n");
#elif defined(ACP_HAVE_WIN32_THREADS)
    printf(" (Win32 threads)\n");
#else
    printf(" (unknown type)\n");
#endif
#else
    printf("  - Threading support not detected\n");
#endif

#if ACP_ENABLE_HEAP
    printf("  ✓ Heap allocation enabled\n");
#else
    printf("  ✓ No-heap mode enabled (embedded-friendly)\n");
#endif

    printf("\nEndianness:\n");
#ifdef ACP_LITTLE_ENDIAN
    printf("  ✓ Little endian detected\n");
#elif defined(ACP_BIG_ENDIAN)
    printf("  ✓ Big endian detected\n");
#else
    printf("  ? Endianness not detected\n");
#endif

    printf("\nCompiler Features:\n");
    printf("  Branch prediction hints: ");
#if defined(ACP_COMPILER_GCC) || defined(ACP_COMPILER_CLANG)
    printf("available\n");
#else
    printf("not available\n");
#endif

    printf("  Function inlining: ");
#ifdef ACP_INLINE
    printf("available\n");
#else
    printf("basic only\n");
#endif

    /* Test core functionality */
    printf("\nCore Library Test:\n");

    /* Initialize CRC table */
    acp_crc16_init_table();
    printf("  ✓ CRC16 initialization successful\n");

    /* Test crypto self-check */
    if (acp_crypto_self_test() == ACP_OK)
    {
        printf("  ✓ Crypto self-test passed\n");
    }
    else
    {
        printf("  ✗ Crypto self-test FAILED\n");
        return 1;
    }

    printf("\nConfiguration validation: PASSED\n");
    printf("ACP library is properly configured for this platform.\n");

    return 0;
}