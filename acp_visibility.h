/**
 * @file acp_visibility.h
 * @brief Symbol visibility macros for ACP shared library builds
 * 
 * Defines macros to control symbol visibility in shared library builds,
 * ensuring proper API export/import semantics across platforms.
 */

#ifndef ACP_VISIBILITY_H
#define ACP_VISIBILITY_H

/* Symbol visibility for shared libraries */
#ifdef ACP_BUILDING_SHARED
    /* Building the shared library - export symbols */
    #if defined(__GNUC__) || defined(__clang__)
        #define ACP_API __attribute__((visibility("default")))
        #define ACP_HIDDEN __attribute__((visibility("hidden")))
    #elif defined(_WIN32) && defined(_MSC_VER)
        #define ACP_API __declspec(dllexport)
        #define ACP_HIDDEN
    #else
        #define ACP_API
        #define ACP_HIDDEN
    #endif
#elif defined(ACP_USING_SHARED)
    /* Using the shared library - import symbols */
    #if defined(_WIN32) && defined(_MSC_VER)
        #define ACP_API __declspec(dllimport)
        #define ACP_HIDDEN
    #else
        #define ACP_API
        #define ACP_HIDDEN
    #endif
#else
    /* Static linking - no visibility attributes needed */
    #define ACP_API
    #define ACP_HIDDEN
#endif

#endif /* ACP_VISIBILITY_H */