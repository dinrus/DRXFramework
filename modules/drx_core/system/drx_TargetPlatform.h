/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

//==============================================================================
/*  В этом файле выясняется платформа построения, определяются некоторые
    макросы, которые могут использоваться в остальном коде для компиляции,
    связанной с той или иной операционной системой.

    Макросы, устанавливаемые здесь:

    - Один из DRX_WINDOWS, DRX_MAC DRX_LINUX, DRX_IOS, DRX_ANDROID и т.д.
    - Либо DRX_32BIT, либо DRX_64BIT, в зависимости от архитектуры.
    - Либо DRX_LITTLE_ENDIAN, либо DRX_BIG_ENDIAN.
    - Либо DRX_INTEL, либо DRX_ARM
    - Либо DRX_GCC, либо DRX_CLANG, либо DRX_MSVC
*/

//==============================================================================
#ifdef DRX_APP_CONFIG_HEADER
 #include DRX_APP_CONFIG_HEADER
#elif ! defined (DRX_GLOBAL_MODULE_SETTINGS_INCLUDED)
 /*
    В большей части проектов будет иметься глобальный файл-заголовок с различными
    настройками, которые применяются ко всему коду проекта. Если используется projucer,
    то он устанавливает глобальный файл-заголовок автоматически, но если работа ведётся
    в ручном режиме, то может понадобиться установить макрос DRX_APP_CONFIG_HEADER
    с именем включаемого файла, либо просто включить таковой прежде всех файлов-модулей
    Си++, в котором указано, что  DRX_GLOBAL_MODULE_SETTINGS_INCLUDED=1, чтобы
    заглушить эту ошибку.
    (Если не требется глобального заголовка, то можно просто определить макрос
    DRX_GLOBAL_MODULE_SETTINGS_INCLUDED, чтобы избежать этой ошибки).

    Note for people who hit this error when trying to compile a DRX project created by
    a pre-v4.2 version of the Introjucer/Projucer, it's very easy to fix: just re-save
    your project with the latest version of the Projucer, and it'll magically fix this!
 */
 //#error "Глобальный файл-заголовок не включен!"
 #include <drxtypes.h>
#endif

//==============================================================================
#if defined (_WIN32) || defined (_WIN64)
  #define       DRX_WINDOWS 1
#elif defined (DRX_ANDROID)
  #undef        DRX_ANDROID
  #define       DRX_ANDROID 1
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
  #define       DRX_BSD 1
#elif defined (LINUX) || defined (__linux__)
  #define       DRX_LINUX 1
#elif defined (__APPLE_CPP__) || defined (__APPLE_CC__)
  #define CF_EXCLUDE_CSTD_HEADERS 1
  #include <TargetConditionals.h> // (needed to find out what platform we're using)
  #include <AvailabilityMacros.h>

  #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define     DRX_IPHONE 1
    #define     DRX_IOS 1
  #else
    #define     DRX_MAC 1
  #endif
#elif defined (__wasm__)
  #define       DRX_WASM 1
#else
  #error "Неизвестная платформа!"
#endif

//==============================================================================
#if DRX_WINDOWS
  #ifdef _MSC_VER
    #ifdef _WIN64
      #define DRX_64BIT 1
    #else
      #define DRX_32BIT 1
    #endif
  #endif

  #ifdef _DEBUG
    #define DRX_DEBUG 1
  #endif

  #ifdef __MINGW32__
    #error "MinGW не поддерживается. Используйте альтернативный компилятор."
  #endif

  /** If defined, this indicates that the processor is little-endian. */
  #define DRX_LITTLE_ENDIAN 1

  #if defined (_M_ARM) || defined (_M_ARM64) || defined (_M_ARM64EC) || defined (__arm__) || defined (__aarch64__)
    #define DRX_ARM 1
  #else
    #define DRX_INTEL 1
  #endif
#endif

//==============================================================================
#if DRX_MAC || DRX_IOS

// Expands to true if the API of the specified version is available at build time, false otherwise
#define DRX_MAC_API_VERSION_CAN_BE_BUILT(major, minor) \
    ((major) * 10000 + (minor) * 100 <= MAC_OS_X_VERSION_MAX_ALLOWED)

// Expands to true if the API of the specified version is available at build time, false otherwise
#define DRX_IOS_API_VERSION_CAN_BE_BUILT(major, minor) \
    ((major) * 10000 + (minor) * 100 <= __IPHONE_OS_VERSION_MAX_ALLOWED)

// Expands to true if the deployment target is greater or equal to the specified version, false otherwise
#define DRX_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST(major, minor) \
    ((major) * 10000 + (minor) * 100 <= MAC_OS_X_VERSION_MIN_REQUIRED)

// Expands to true if the deployment target is greater or equal to the specified version, false otherwise
#define DRX_IOS_API_VERSION_MIN_REQUIRED_AT_LEAST(major, minor) \
    ((major) * 10000 + (minor) * 100 <= __IPHONE_OS_VERSION_MIN_REQUIRED)

#if defined (DEBUG) || defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
    #define DRX_DEBUG 1
  #endif

  #if ! (defined (DEBUG) || defined (_DEBUG) || defined (NDEBUG) || defined (_NDEBUG))
    #warning "Neither NDEBUG or DEBUG has been defined - you should set one of these to make it clear whether this is a release build,"
  #endif

  #ifdef __LITTLE_ENDIAN__
    #define DRX_LITTLE_ENDIAN 1
  #else
    #define DRX_BIG_ENDIAN 1
  #endif

  #ifdef __LP64__
    #define DRX_64BIT 1
  #else
    #define DRX_32BIT 1
  #endif

  #if defined (__ppc__) || defined (__ppc64__)
    #error "PowerPC is no longer supported by DRX!"
  #elif defined (__arm__) || defined (__arm64__)
    #define DRX_ARM 1
  #else
    #define DRX_INTEL 1
  #endif

  #if DRX_MAC
    #if ! DRX_MAC_API_VERSION_CAN_BE_BUILT (11, 1)
      #error "The macOS 11.1 SDK (Xcode 12.4+) is required to build DRX apps. You can create apps that run on macOS 10.11+ by changing the deployment target."
    #elif ! DRX_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST (10, 11)
      #error "Building for OSX 10.10 and earlier is no longer supported!"
    #endif
  #endif
#endif

//==============================================================================
#if DRX_LINUX || DRX_ANDROID || DRX_BSD

  #ifdef _DEBUG
    #define DRX_DEBUG 1
  #endif

  // Allow override for big-endian Linux platforms
  #if defined (__LITTLE_ENDIAN__) || ! defined (DRX_BIG_ENDIAN)
    #define DRX_LITTLE_ENDIAN 1
    #undef DRX_BIG_ENDIAN
  #else
    #undef DRX_LITTLE_ENDIAN
    #define DRX_BIG_ENDIAN 1
  #endif

  #if defined (__LP64__) || defined (_LP64) || defined (__arm64__) || defined (__aarch64__)
    #define DRX_64BIT 1
  #else
    #define DRX_32BIT 1
  #endif

  #if defined (__arm__) || defined (__arm64__) || defined (__aarch64__)
    #define DRX_ARM 1
  #elif __MMX__ || __SSE__ || __amd64__
    #define DRX_INTEL 1
  #endif
#endif

//==============================================================================
// Compiler type macros.

#if defined (__clang__)
  #define DRX_CLANG 1

#elif defined (__GNUC__)
  #define DRX_GCC 1

#elif defined (_MSC_VER)
  #define DRX_MSVC 1

#else
  #error Неизвестный компилятор
#endif
