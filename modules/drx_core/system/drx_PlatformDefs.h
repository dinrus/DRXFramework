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

namespace drx
{

//==============================================================================
/*  This file defines miscellaneous macros for debugging, assertions, etc.
*/

//==============================================================================
#ifdef DRX_FORCE_DEBUG
 #undef DRX_DEBUG

 #if DRX_FORCE_DEBUG
  #define DRX_DEBUG 1
 #endif
#endif

/** This macro defines the C calling convention used as the standard for DRX calls. */
#if DRX_WINDOWS
 #define DRX_CALLTYPE   __stdcall
 #define DRX_CDECL      __cdecl
#else
 #define DRX_CALLTYPE
 #define DRX_CDECL
#endif

//==============================================================================
// Debugging and assertion macros

#ifndef DRX_LOG_CURRENT_ASSERTION
 #if DRX_LOG_ASSERTIONS || DRX_DEBUG
  #define DRX_LOG_CURRENT_ASSERTION    drx::logAssertion (__FILE__, __LINE__);
 #else
  #define DRX_LOG_CURRENT_ASSERTION
 #endif
#endif

//==============================================================================
#if DRX_IOS || DRX_LINUX || DRX_BSD
  /** This will try to break into the debugger if the app is currently being debugged.
      If called by an app that's not being debugged, the behaviour isn't defined - it may
      crash or not, depending on the platform.
      @see jassert()
  */
  #define DRX_BREAK_IN_DEBUGGER        { ::kill (0, SIGTRAP); }
#elif DRX_MSVC
  #ifndef __INTEL_COMPILER
    #pragma intrinsic (__debugbreak)
  #endif
  #define DRX_BREAK_IN_DEBUGGER        { __debugbreak(); }
#elif DRX_INTEL && (DRX_GCC || DRX_CLANG || DRX_MAC)
  #if DRX_NO_INLINE_ASM
   #define DRX_BREAK_IN_DEBUGGER       { }
  #else
   #define DRX_BREAK_IN_DEBUGGER       { asm ("int $3"); }
  #endif
#elif DRX_ARM && DRX_MAC
  #define DRX_BREAK_IN_DEBUGGER        { __builtin_debugtrap(); }
#elif DRX_ANDROID
  #define DRX_BREAK_IN_DEBUGGER        { __builtin_trap(); }
#else
  #define DRX_BREAK_IN_DEBUGGER        { __asm int32 3 }
#endif

#if DRX_CLANG && defined (__has_feature) && ! defined (DRX_ANALYZER_NORETURN)
 #if __has_feature (attribute_analyzer_noreturn)
  inline z0 __attribute__ ((analyzer_noreturn)) drx_assert_noreturn() {}
  #define DRX_ANALYZER_NORETURN drx::drx_assert_noreturn();
 #endif
#endif

#ifndef DRX_ANALYZER_NORETURN
 #define DRX_ANALYZER_NORETURN
#endif

/** Used to silence Wimplicit-fallthrough on Clang and GCC where available
    as there are a few places in the codebase where we need to do this
    deliberately and want to ignore the warning.
*/
#if DRX_CLANG
 #if __has_cpp_attribute(clang::fallthrough)
  #define DRX_FALLTHROUGH [[clang::fallthrough]];
 #else
  #define DRX_FALLTHROUGH
 #endif
#elif DRX_GCC
 #if __GNUC__ >= 7
  #define DRX_FALLTHROUGH [[gnu::fallthrough]];
 #else
  #define DRX_FALLTHROUGH
 #endif
#else
 #define DRX_FALLTHROUGH
#endif

//==============================================================================
#if DRX_MSVC && ! defined (DOXYGEN)
 #define DRX_BLOCK_WITH_FORCED_SEMICOLON(x) \
   __pragma(warning(push)) \
   __pragma(warning(disable:4127)) \
   do { x } while (false) \
   __pragma(warning(pop))
#else
 /** This is the good old C++ trick for creating a macro that forces the user to put
    a semicolon after it when they use it.
 */
 #define DRX_BLOCK_WITH_FORCED_SEMICOLON(x) do { x } while (false)
#endif

//==============================================================================
#if (DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS) || DOXYGEN
  /** Writes a string to the standard error stream.
      Note that as well as a single string, you can use this to write multiple items
      as a stream, e.g.
      @code
        DBG ("foo = " << foo << "bar = " << bar);
      @endcode
      The macro is only enabled in a debug build, so be careful not to use it with expressions
      that have important side-effects!
      @see Logger::outputDebugString
  */
  #define DBG(textToWrite)              DRX_BLOCK_WITH_FORCED_SEMICOLON (drx::Txt tempDbgBuf; tempDbgBuf << textToWrite; drx::Logger::outputDebugString (tempDbgBuf);)

  //==============================================================================
  /** This will always cause an assertion failure.
      It is only compiled in a debug build, (unless DRX_LOG_ASSERTIONS is enabled for your build).
      @see jassert
  */
  #define jassertfalse                  DRX_BLOCK_WITH_FORCED_SEMICOLON (DRX_LOG_CURRENT_ASSERTION; if (drx::drx_isRunningUnderDebugger()) DRX_BREAK_IN_DEBUGGER; DRX_ANALYZER_NORETURN)

  //==============================================================================
  /** Platform-independent assertion macro.

      This macro gets turned into a no-op when you're building with debugging turned off, so be
      careful that the expression you pass to it doesn't perform any actions that are vital for the
      correct behaviour of your program!
      @see jassertfalse
  */
  #define jassert(expression)           DRX_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)

  /** Platform-independent assertion macro which suppresses ignored-variable
      warnings in all build modes. You should probably use a plain jassert()
      and [[maybe_unused]] by default.
  */
  #define jassertquiet(expression)      DRX_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)

  #define DRX_ASSERTIONS_ENABLED       1

#else
  //==============================================================================
  // If debugging is disabled, these dummy debug and assertion macros are used..

  #define DBG(textToWrite)
  #define jassertfalse                  DRX_BLOCK_WITH_FORCED_SEMICOLON (DRX_LOG_CURRENT_ASSERTION;)
  #define DRX_ASSERTIONS_ENABLED       0

  #if DRX_LOG_ASSERTIONS
   #define jassert(expression)          DRX_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)
   #define jassertquiet(expression)     DRX_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)
  #else
   #define jassert(expression)          DRX_BLOCK_WITH_FORCED_SEMICOLON ( ; )
   #define jassertquiet(expression)     DRX_BLOCK_WITH_FORCED_SEMICOLON (if (false) (z0) (expression);)
  #endif

#endif

#define DRX_ASSERTIONS_ENABLED_OR_LOGGED   DRX_ASSERTIONS_ENABLED || DRX_LOG_ASSERTIONS

//==============================================================================
#ifndef DOXYGEN
 #define DRX_JOIN_MACRO_HELPER(a, b) a ## b
 #define DRX_STRINGIFY_MACRO_HELPER(a) #a
#endif

/** A good old-fashioned C macro concatenation helper.
    This combines two items (which may themselves be macros) into a single string,
    avoiding the pitfalls of the ## macro operator.
*/
#define DRX_JOIN_MACRO(item1, item2)  DRX_JOIN_MACRO_HELPER (item1, item2)

/** A handy C macro for stringifying any symbol, rather than just a macro parameter. */
#define DRX_STRINGIFY(item)  DRX_STRINGIFY_MACRO_HELPER (item)

//==============================================================================
/** This is a shorthand macro for deleting a class's copy constructor and
    copy assignment operator.

    For example, instead of
    @code
    class MyClass
    {
        etc..

    private:
        MyClass (const MyClass&);
        MyClass& operator= (const MyClass&);
    };@endcode

    ..you can just write:

    @code
    class MyClass
    {
        etc..

    private:
        DRX_DECLARE_NON_COPYABLE (MyClass)
    };@endcode
*/
#define DRX_DECLARE_NON_COPYABLE(className) \
    className (const className&) = delete;\
    className& operator= (const className&) = delete;

/** This is a shorthand macro for deleting a class's move constructor and
    move assignment operator.
*/
#define DRX_DECLARE_NON_MOVEABLE(className) \
    className (className&&) = delete;\
    className& operator= (className&&) = delete;

/** This is a shorthand way of writing both a DRX_DECLARE_NON_COPYABLE and
    DRX_LEAK_DETECTOR macro for a class.
*/
#define DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
    DRX_DECLARE_NON_COPYABLE(className) \
    DRX_LEAK_DETECTOR(className)

/** This macro can be added to class definitions to disable the use of new/delete to
    allocate the object on the heap, forcing it to only be used as a stack or member variable.
*/
#define DRX_PREVENT_HEAP_ALLOCATION \
   private: \
    static uk operator new (size_t) = delete; \
    static z0 operator delete (uk) = delete;

//==============================================================================
#if DRX_MSVC && ! defined (DOXYGEN)
 #define DRX_COMPILER_WARNING(msg) __pragma (message (__FILE__ "(" DRX_STRINGIFY (__LINE__) ") : Warning: " msg))
#else

 /** This macro allows you to emit a custom compiler warning message.
     Very handy for marking bits of code as "to-do" items, or for shaming
     code written by your co-workers in a way that's hard to ignore.

     GCC and Clang provide the \#warning directive, but MSVC doesn't, so this
     macro is a cross-compiler way to get the same functionality as \#warning.

     Unlike the \#warning directive in GCC and Clang this macro requires the
     argument to passed as a quoted string.
 */
 #define DRX_COMPILER_WARNING(msg)  _Pragma (DRX_STRINGIFY (message (msg)))
#endif


//==============================================================================
#if DRX_DEBUG || DOXYGEN
  /** A platform-independent way of forcing an inline function.
      Use the syntax: @code
      forcedinline z0 myfunction (i32 x)
      @endcode
  */
  #define forcedinline  inline
#else
  #if DRX_MSVC
   #define forcedinline       __forceinline
  #else
   #define forcedinline       inline __attribute__ ((always_inline))
  #endif
#endif

#if DRX_MSVC || DOXYGEN
  /** This can be placed before a stack or member variable declaration to tell the compiler
      to align it to the specified number of bytes. */
  #define DRX_ALIGN(bytes)   __declspec (align (bytes))
#else
  #define DRX_ALIGN(bytes)   __attribute__ ((aligned (bytes)))
#endif

//==============================================================================
#if DRX_ANDROID && ! defined (DOXYGEN)
 #define DRX_MODAL_LOOPS_PERMITTED 0
#elif ! defined (DRX_MODAL_LOOPS_PERMITTED)
 /** Some operating environments don't provide a modal loop mechanism, so this flag can be
     used to disable any functions that try to run a modal loop. */
 #define DRX_MODAL_LOOPS_PERMITTED 0
#endif

//==============================================================================
#if DRX_GCC || DRX_CLANG
 #define DRX_PACKED __attribute__ ((packed))
#elif ! defined (DOXYGEN)
 #define DRX_PACKED
#endif

//==============================================================================
#if DRX_GCC || DOXYGEN
 /** This can be appended to a function declaration to tell gcc to disable associative
     math optimisations which break some floating point algorithms. */
 #define DRX_NO_ASSOCIATIVE_MATH_OPTIMISATIONS   __attribute__ ((__optimize__ ("no-associative-math")))
#else
 #define DRX_NO_ASSOCIATIVE_MATH_OPTIMISATIONS
#endif

} // namespace drx
