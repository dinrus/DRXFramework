#pragma once

#include <drx_core/system/drx_TargetPlatform.h>

/** Return the Nth argument. By passing a variadic pack followed by N other
    parameters, we can select one of those N parameter based on the length of
    the parameter pack.
*/
#define DRX_NTH_ARG_(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09,        \
                      _10, _11, _12, _13, _14, _15, _16, _17, _18, _19,        \
                      _20, _21, _22, _23, _24, _25, _26, _27, _28, _29,        \
                      _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,        \
                      _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, N, ...)\
    N

#define DRX_EACH_00_(FN)
#define DRX_EACH_01_(FN, X)      FN(X)
#define DRX_EACH_02_(FN, X, ...) FN(X) DRX_EACH_01_(FN, __VA_ARGS__)
#define DRX_EACH_03_(FN, X, ...) FN(X) DRX_EACH_02_(FN, __VA_ARGS__)
#define DRX_EACH_04_(FN, X, ...) FN(X) DRX_EACH_03_(FN, __VA_ARGS__)
#define DRX_EACH_05_(FN, X, ...) FN(X) DRX_EACH_04_(FN, __VA_ARGS__)
#define DRX_EACH_06_(FN, X, ...) FN(X) DRX_EACH_05_(FN, __VA_ARGS__)
#define DRX_EACH_07_(FN, X, ...) FN(X) DRX_EACH_06_(FN, __VA_ARGS__)
#define DRX_EACH_08_(FN, X, ...) FN(X) DRX_EACH_07_(FN, __VA_ARGS__)
#define DRX_EACH_09_(FN, X, ...) FN(X) DRX_EACH_08_(FN, __VA_ARGS__)
#define DRX_EACH_10_(FN, X, ...) FN(X) DRX_EACH_09_(FN, __VA_ARGS__)
#define DRX_EACH_11_(FN, X, ...) FN(X) DRX_EACH_10_(FN, __VA_ARGS__)
#define DRX_EACH_12_(FN, X, ...) FN(X) DRX_EACH_11_(FN, __VA_ARGS__)
#define DRX_EACH_13_(FN, X, ...) FN(X) DRX_EACH_12_(FN, __VA_ARGS__)
#define DRX_EACH_14_(FN, X, ...) FN(X) DRX_EACH_13_(FN, __VA_ARGS__)
#define DRX_EACH_15_(FN, X, ...) FN(X) DRX_EACH_14_(FN, __VA_ARGS__)
#define DRX_EACH_16_(FN, X, ...) FN(X) DRX_EACH_15_(FN, __VA_ARGS__)
#define DRX_EACH_17_(FN, X, ...) FN(X) DRX_EACH_16_(FN, __VA_ARGS__)
#define DRX_EACH_18_(FN, X, ...) FN(X) DRX_EACH_17_(FN, __VA_ARGS__)
#define DRX_EACH_19_(FN, X, ...) FN(X) DRX_EACH_18_(FN, __VA_ARGS__)
#define DRX_EACH_20_(FN, X, ...) FN(X) DRX_EACH_19_(FN, __VA_ARGS__)
#define DRX_EACH_21_(FN, X, ...) FN(X) DRX_EACH_20_(FN, __VA_ARGS__)
#define DRX_EACH_22_(FN, X, ...) FN(X) DRX_EACH_21_(FN, __VA_ARGS__)
#define DRX_EACH_23_(FN, X, ...) FN(X) DRX_EACH_22_(FN, __VA_ARGS__)
#define DRX_EACH_24_(FN, X, ...) FN(X) DRX_EACH_23_(FN, __VA_ARGS__)
#define DRX_EACH_25_(FN, X, ...) FN(X) DRX_EACH_24_(FN, __VA_ARGS__)
#define DRX_EACH_26_(FN, X, ...) FN(X) DRX_EACH_25_(FN, __VA_ARGS__)
#define DRX_EACH_27_(FN, X, ...) FN(X) DRX_EACH_26_(FN, __VA_ARGS__)
#define DRX_EACH_28_(FN, X, ...) FN(X) DRX_EACH_27_(FN, __VA_ARGS__)
#define DRX_EACH_29_(FN, X, ...) FN(X) DRX_EACH_28_(FN, __VA_ARGS__)
#define DRX_EACH_30_(FN, X, ...) FN(X) DRX_EACH_29_(FN, __VA_ARGS__)
#define DRX_EACH_31_(FN, X, ...) FN(X) DRX_EACH_30_(FN, __VA_ARGS__)
#define DRX_EACH_32_(FN, X, ...) FN(X) DRX_EACH_31_(FN, __VA_ARGS__)
#define DRX_EACH_33_(FN, X, ...) FN(X) DRX_EACH_32_(FN, __VA_ARGS__)
#define DRX_EACH_34_(FN, X, ...) FN(X) DRX_EACH_33_(FN, __VA_ARGS__)
#define DRX_EACH_35_(FN, X, ...) FN(X) DRX_EACH_34_(FN, __VA_ARGS__)
#define DRX_EACH_36_(FN, X, ...) FN(X) DRX_EACH_35_(FN, __VA_ARGS__)
#define DRX_EACH_37_(FN, X, ...) FN(X) DRX_EACH_36_(FN, __VA_ARGS__)
#define DRX_EACH_38_(FN, X, ...) FN(X) DRX_EACH_37_(FN, __VA_ARGS__)
#define DRX_EACH_39_(FN, X, ...) FN(X) DRX_EACH_38_(FN, __VA_ARGS__)
#define DRX_EACH_40_(FN, X, ...) FN(X) DRX_EACH_39_(FN, __VA_ARGS__)
#define DRX_EACH_41_(FN, X, ...) FN(X) DRX_EACH_40_(FN, __VA_ARGS__)
#define DRX_EACH_42_(FN, X, ...) FN(X) DRX_EACH_41_(FN, __VA_ARGS__)
#define DRX_EACH_43_(FN, X, ...) FN(X) DRX_EACH_42_(FN, __VA_ARGS__)
#define DRX_EACH_44_(FN, X, ...) FN(X) DRX_EACH_43_(FN, __VA_ARGS__)
#define DRX_EACH_45_(FN, X, ...) FN(X) DRX_EACH_44_(FN, __VA_ARGS__)
#define DRX_EACH_46_(FN, X, ...) FN(X) DRX_EACH_45_(FN, __VA_ARGS__)
#define DRX_EACH_47_(FN, X, ...) FN(X) DRX_EACH_46_(FN, __VA_ARGS__)
#define DRX_EACH_48_(FN, X, ...) FN(X) DRX_EACH_47_(FN, __VA_ARGS__)
#define DRX_EACH_49_(FN, X, ...) FN(X) DRX_EACH_48_(FN, __VA_ARGS__)

/** Apply the macro FN to each of the other arguments. */
#define DRX_EACH(FN, ...)                                                     \
    DRX_NTH_ARG_(, __VA_ARGS__,                                               \
                  DRX_EACH_49_,                                               \
                  DRX_EACH_48_,                                               \
                  DRX_EACH_47_,                                               \
                  DRX_EACH_46_,                                               \
                  DRX_EACH_45_,                                               \
                  DRX_EACH_44_,                                               \
                  DRX_EACH_43_,                                               \
                  DRX_EACH_42_,                                               \
                  DRX_EACH_41_,                                               \
                  DRX_EACH_40_,                                               \
                  DRX_EACH_39_,                                               \
                  DRX_EACH_38_,                                               \
                  DRX_EACH_37_,                                               \
                  DRX_EACH_36_,                                               \
                  DRX_EACH_35_,                                               \
                  DRX_EACH_34_,                                               \
                  DRX_EACH_33_,                                               \
                  DRX_EACH_32_,                                               \
                  DRX_EACH_31_,                                               \
                  DRX_EACH_30_,                                               \
                  DRX_EACH_29_,                                               \
                  DRX_EACH_28_,                                               \
                  DRX_EACH_27_,                                               \
                  DRX_EACH_26_,                                               \
                  DRX_EACH_25_,                                               \
                  DRX_EACH_24_,                                               \
                  DRX_EACH_23_,                                               \
                  DRX_EACH_22_,                                               \
                  DRX_EACH_21_,                                               \
                  DRX_EACH_20_,                                               \
                  DRX_EACH_19_,                                               \
                  DRX_EACH_18_,                                               \
                  DRX_EACH_17_,                                               \
                  DRX_EACH_16_,                                               \
                  DRX_EACH_15_,                                               \
                  DRX_EACH_14_,                                               \
                  DRX_EACH_13_,                                               \
                  DRX_EACH_12_,                                               \
                  DRX_EACH_11_,                                               \
                  DRX_EACH_10_,                                               \
                  DRX_EACH_09_,                                               \
                  DRX_EACH_08_,                                               \
                  DRX_EACH_07_,                                               \
                  DRX_EACH_06_,                                               \
                  DRX_EACH_05_,                                               \
                  DRX_EACH_04_,                                               \
                  DRX_EACH_03_,                                               \
                  DRX_EACH_02_,                                               \
                  DRX_EACH_01_,                                               \
                  DRX_EACH_00_)                                               \
    (FN, __VA_ARGS__)

/** Concatenate two tokens to form a new token. */
#define DRX_CONCAT_(a, b) a##b
#define DRX_CONCAT(a, b) DRX_CONCAT_(a, b)

/** Quote the argument, turning it into a string. */
#define DRX_TO_STRING(x) #x

#if DRX_CLANG || DRX_GCC
    #define DRX_IGNORE_GCC_IMPL_(compiler, warning)
    #define DRX_IGNORE_GCC_IMPL_0(compiler, warning)
    #define DRX_IGNORE_GCC_IMPL_1(compiler, warning)                          \
        _Pragma(DRX_TO_STRING(compiler diagnostic ignored warning))

    /** If 'warning' is recognised by this compiler, ignore it. */
    #if defined (__has_warning)
        #define DRX_IGNORE_GCC_LIKE(compiler, warning)                        \
            DRX_CONCAT(DRX_IGNORE_GCC_IMPL_, __has_warning(warning))(compiler, warning)
    #else
        #define DRX_IGNORE_GCC_LIKE(compiler, warning)                        \
            DRX_IGNORE_GCC_IMPL_1(compiler, warning)
    #endif

    /** Ignore GCC/clang-specific warnings. */
    #define DRX_IGNORE_GCC(warning)   DRX_IGNORE_GCC_LIKE(GCC, warning)
    #define DRX_IGNORE_clang(warning) DRX_IGNORE_GCC_LIKE(clang, warning)

    #define DRX_IGNORE_WARNINGS_GCC_LIKE(compiler, ...)                       \
        _Pragma(DRX_TO_STRING(compiler diagnostic push))                      \
        DRX_EACH(DRX_CONCAT(DRX_IGNORE_, compiler), __VA_ARGS__)

    /** Push a new warning scope, and then ignore each warning for either clang
        or gcc. If the compiler doesn't support __has_warning, we add -Wpragmas
        as the first disabled warning because otherwise we might get complaints
        about unknown warning options.
    */
    #if defined (__has_warning)
        #define DRX_PUSH_WARNINGS_GCC_LIKE(compiler, ...)                     \
            DRX_IGNORE_WARNINGS_GCC_LIKE(compiler, __VA_ARGS__)
    #else
        #define DRX_PUSH_WARNINGS_GCC_LIKE(compiler, ...)                     \
            DRX_IGNORE_WARNINGS_GCC_LIKE(compiler, "-Wpragmas", __VA_ARGS__)
    #endif

    /** Pop the current warning scope. */
    #define DRX_POP_WARNINGS_GCC_LIKE(compiler)                               \
        _Pragma(DRX_TO_STRING(compiler diagnostic pop))

    /** Push/pop warnings on compilers with gcc-like warning flags.
        These macros expand to nothing on other compilers (like MSVC).
    */
    #if DRX_CLANG
        #define DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE(...) DRX_PUSH_WARNINGS_GCC_LIKE(clang, __VA_ARGS__)
        #define DRX_END_IGNORE_WARNINGS_GCC_LIKE DRX_POP_WARNINGS_GCC_LIKE(clang)
    #else
        #define DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE(...) DRX_PUSH_WARNINGS_GCC_LIKE(GCC, __VA_ARGS__)
        #define DRX_END_IGNORE_WARNINGS_GCC_LIKE DRX_POP_WARNINGS_GCC_LIKE(GCC)
    #endif
#else
    #define DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE(...)
    #define DRX_END_IGNORE_WARNINGS_GCC_LIKE
#endif

/** Push/pop warnings on MSVC. These macros expand to nothing on other
    compilers (like clang and gcc).
*/
#if DRX_MSVC
    #define DRX_IGNORE_MSVC(warnings) __pragma(warning(disable:warnings))
    #define DRX_BEGIN_IGNORE_WARNINGS_LEVEL_MSVC(level, warnings)              \
        __pragma(warning(push, level)) DRX_IGNORE_MSVC(warnings)
    #define DRX_BEGIN_IGNORE_WARNINGS_MSVC(warnings)                           \
        __pragma(warning(push)) DRX_IGNORE_MSVC(warnings)
    #define DRX_END_IGNORE_WARNINGS_MSVC __pragma(warning(pop))
#else
    #define DRX_IGNORE_MSVC(warnings)
    #define DRX_BEGIN_IGNORE_WARNINGS_LEVEL_MSVC(level, warnings)
    #define DRX_BEGIN_IGNORE_WARNINGS_MSVC(warnings)
    #define DRX_END_IGNORE_WARNINGS_MSVC
#endif

#if DRX_MAC || DRX_IOS
    #define DRX_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION 11
#else
    #define DRX_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION 9
#endif

#define DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS                                  \
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")           \
    DRX_BEGIN_IGNORE_WARNINGS_MSVC (4996)

#define DRX_END_IGNORE_DEPRECATION_WARNINGS                                    \
    DRX_END_IGNORE_WARNINGS_MSVC                                               \
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

/** Disable sanitizers for a range of functions.

    This functionality doesn't seem to exist on GCC yet, so at the moment this only works for clang.
*/
#if DRX_CLANG && __clang_major__ >= DRX_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION
    #define DRX_BEGIN_NO_SANITIZE(warnings)                                    \
        _Pragma (DRX_TO_STRING (clang attribute push (__attribute__ ((no_sanitize (warnings))), apply_to=function)))
    #define DRX_END_NO_SANITIZE _Pragma (DRX_TO_STRING (clang attribute pop))
#else
    #define DRX_BEGIN_NO_SANITIZE(warnings)
    #define DRX_END_NO_SANITIZE
#endif

#undef DRX_SANITIZER_ATTRIBUTE_MINIMUM_CLANG_VERSION
