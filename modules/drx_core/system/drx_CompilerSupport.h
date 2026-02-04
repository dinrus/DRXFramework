#pragma once

//==============================================================================
// Clang

 #ifndef DRX_COMPILER_SUPPORTS_ARC
  #define DRX_COMPILER_SUPPORTS_ARC 1
 #endif

 #ifndef DRX_EXCEPTIONS_DISABLED
  #if ! __has_feature (cxx_exceptions)
   #define DRX_EXCEPTIONS_DISABLED 1
  #endif
 #endif

 #if ! defined (DRX_SILENCE_XCODE_15_LINKER_WARNING)                          \
     && defined (__apple_build_version__)                                      \
     && __apple_build_version__ >= 15000000                                    \
     && __apple_build_version__ <  15000100

  // Due to known issues, the linker in Xcode 15.0 may produce broken binaries.
  #error Please upgrade to Xcode 15.1 or higher
 #endif

 #define DRX_CXX17_IS_AVAILABLE (__cplusplus >= 201703L)
 #define DRX_CXX20_IS_AVAILABLE (__cplusplus >= 202002L)

//==============================================================================
#ifndef DOXYGEN
 // These are old flags that are now supported on all compatible build targets
 #define DRX_CXX14_IS_AVAILABLE 1
 #define DRX_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
 #define DRX_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
 #define DRX_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
 #define DRX_COMPILER_SUPPORTS_NOEXCEPT 1
 #define DRX_DELETED_FUNCTION = delete
 #define DRX_CONSTEXPR constexpr
 #define DRX_NODISCARD [[nodiscard]]
#endif
