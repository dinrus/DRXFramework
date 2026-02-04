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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 DRX Module Format.md file.


 BEGIN_DRX_MODULE_DECLARATION

  ID:                 drx_core
  vendor:             drx
  version:            8.0.7
  name:               DRX core classes
  description:        The essential set of basic DRX classes, as required by all the other DRX modules. Includes text, container, memory, threading and i/o functionality.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:
  OSXFrameworks:      Cocoa Foundation IOKit Security
  iOSFrameworks:      Foundation
  linuxLibs:          rt dl pthread

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_CORE_H_INCLUDED

#include <drxtypes.h>
//==============================================================================
#ifdef _MSC_VER
 #pragma warning (push)
 // Disable warnings for i64 class names, padding, and undefined preprocessor definitions.
 #pragma warning (disable: 4251 4786 4668 4820)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 1125)
 #endif
#endif

#include <drx_core/system/drx_TargetPlatform.h>

//==============================================================================
/** Config: DRX_FORCE_DEBUG

    Normally, DRX_DEBUG is set to 1 or 0 based on compiler and project settings,
    but if you define this value, you can override this to force it to be true or false.
*/
#ifndef DRX_FORCE_DEBUG
 #define DRX_FORCE_DEBUG 0
#endif

//==============================================================================
/** Config: DRX_LOG_ASSERTIONS

    If this flag is enabled, the jassert and jassertfalse macros will always use Logger::writeToLog()
    to write a message when an assertion happens.

    Enabling it will also leave this turned on in release builds. When it's disabled,
    however, the jassert and jassertfalse macros will not be compiled in a
    release build.

    @see jassert, jassertfalse, Logger
*/
#ifndef DRX_LOG_ASSERTIONS
 #if DRX_ANDROID
  #define DRX_LOG_ASSERTIONS 1
 #else
  #define DRX_LOG_ASSERTIONS 0
 #endif
#endif

//==============================================================================
/** Config: DRX_CHECK_MEMORY_LEAKS

    Enables a memory-leak check for certain objects when the app terminates. See the LeakedObjectDetector
    class and the DRX_LEAK_DETECTOR macro for more details about enabling leak checking for specific classes.
*/
#if DRX_DEBUG && ! defined (DRX_CHECK_MEMORY_LEAKS)
 #define DRX_CHECK_MEMORY_LEAKS 1
#endif

//==============================================================================
/** Config: DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES

    In a Windows build, this can be used to stop the required system libs being
    automatically added to the link stage.
*/
#ifndef DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES
 #define DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES 0
#endif

/** Config: DRX_INCLUDE_ZLIB_CODE
    This can be used to disable Drx's embedded 3rd-party zlib code.
    You might need to tweak this if you're linking to an external zlib library in your app,
    but for normal apps, this option should be left alone.

    If you disable this, you might also want to set a value for DRX_ZLIB_INCLUDE_PATH, to
    specify the path where your zlib headers live.
*/
#ifndef DRX_INCLUDE_ZLIB_CODE
 #define DRX_INCLUDE_ZLIB_CODE 1
#endif

#ifndef DRX_ZLIB_INCLUDE_PATH
 #define DRX_ZLIB_INCLUDE_PATH <zlib.h>
#endif

/** Config: DRX_USE_CURL
    Enables http/https support via libcurl (Linux only). Enabling this will add an additional
    run-time dynamic dependency to libcurl.

    If you disable this then https/ssl support will not be available on Linux.
*/
#ifndef DRX_USE_CURL
 #define DRX_USE_CURL 1
#endif

/** Config: DRX_LOAD_CURL_SYMBOLS_LAZILY
    If enabled, DRX will load libcurl lazily when required (for example, when WebInputStream
    is used). Enabling this flag may also help with library dependency errors as linking
    libcurl at compile-time may instruct the linker to hard depend on a specific version
    of libcurl. It's also useful if you want to limit the amount of DRX dependencies and
    you are not using WebInputStream or the URL classes.
*/
#ifndef DRX_LOAD_CURL_SYMBOLS_LAZILY
 #define DRX_LOAD_CURL_SYMBOLS_LAZILY 0
#endif

/** Config: DRX_CATCH_UNHANDLED_EXCEPTIONS
    If enabled, this will add some exception-catching code to forward unhandled exceptions
    to your DRXApplicationBase::unhandledException() callback.
*/
#ifndef DRX_CATCH_UNHANDLED_EXCEPTIONS
 #define DRX_CATCH_UNHANDLED_EXCEPTIONS 0
#endif

/** Config: DRX_ALLOW_STATIC_NULL_VARIABLES
    If disabled, this will turn off dangerous static globals like Txt::empty, var::null, etc
    which can cause nasty order-of-initialisation problems if they are referenced during static
    constructor code.
*/
#ifndef DRX_ALLOW_STATIC_NULL_VARIABLES
 #define DRX_ALLOW_STATIC_NULL_VARIABLES 0
#endif

/** Config: DRX_STRICT_REFCOUNTEDPOINTER
    If enabled, this will make the ReferenceCountedObjectPtr class stricter about allowing
    itself to be cast directly to a raw pointer. By default this is disabled, for compatibility
    with old code, but if possible, you should always enable it to improve code safety!
*/
#ifndef DRX_STRICT_REFCOUNTEDPOINTER
 #define DRX_STRICT_REFCOUNTEDPOINTER 0
#endif

/** Config: DRX_ENABLE_ALLOCATION_HOOKS
    If enabled, this will add global allocation functions with built-in assertions, which may
    help when debugging allocations in unit tests.
*/
#ifndef DRX_ENABLE_ALLOCATION_HOOKS
 #define DRX_ENABLE_ALLOCATION_HOOKS 0
#endif

#ifndef DRX_STRING_UTF_TYPE
 #define DRX_STRING_UTF_TYPE 8
#endif

//==============================================================================
//==============================================================================

#if DRX_CORE_INCLUDE_NATIVE_HEADERS
 #include <drx_core/native/drx_BasicNativeHeaders.h>
#endif

#if DRX_WINDOWS
 #undef small
#endif

#include <drx_core/system/drx_StandardHeader.h>

namespace drx
{
    class StringRef;
    class MemoryBlock;
    class File;
    class InputStream;
    class OutputStream;
    class DynamicObject;
    class FileInputStream;
    class FileOutputStream;
    class XmlElement;

    extern DRX_API b8 DRX_CALLTYPE drx_isRunningUnderDebugger() noexcept;
    extern DRX_API z0 DRX_CALLTYPE logAssertion (tukk file, i32 line) noexcept;

    namespace detail
    {
        class QuickJSWrapper;
    }
}

#include <drx_core/misc/drx_EnumHelpers.h>
#include <drx_core/memory/drx_Memory.h>
#include <drx_core/maths/drx_MathsFunctions.h>
#include <drx_core/memory/drx_ByteOrder.h>
#include <drx_core/memory/drx_Atomic.h>
#include <drx_core/text/drx_CharacterFunctions.h>

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4514 4996)

#include <drx_core/text/drx_CharPointer_UTF8.h>
#include <drx_core/text/drx_CharPointer_UTF16.h>
#include <drx_core/text/drx_CharPointer_UTF32.h>
#include <drx_core/text/drx_CharPointer_ASCII.h>

DRX_END_IGNORE_WARNINGS_MSVC

#include <drx_core/text/drx_String.h>
#include <drx_core/text/drx_StringRef.h>
#include <drx_core/logging/drx_Logger.h>
#include <drx_core/memory/drx_LeakedObjectDetector.h>
#include <drx_core/memory/drx_ContainerDeletePolicy.h>
#include <drx_core/memory/drx_HeapBlock.h>
#include <drx_core/memory/drx_MemoryBlock.h>
#include <drx_core/memory/drx_CopyableHeapBlock.h>
#include <drx_core/memory/drx_ReferenceCountedObject.h>
#include <drx_core/memory/drx_ScopedPointer.h>
#include <drx_core/memory/drx_OptionalScopedPointer.h>
#include <drx_core/containers/drx_Optional.h>
#include <drx_core/containers/drx_Enumerate.h>
#include <drx_core/containers/drx_ScopedValueSetter.h>
#include <drx_core/memory/drx_Singleton.h>
#include <drx_core/memory/drx_WeakReference.h>
#include <drx_core/threads/drx_ScopedLock.h>
#include <drx_core/threads/drx_CriticalSection.h>
#include <drx_core/maths/drx_Range.h>
#include <drx_core/maths/drx_NormalisableRange.h>
#include <drx_core/maths/drx_StatisticsAccumulator.h>
#include <drx_core/containers/drx_ElementComparator.h>
#include <drx_core/containers/drx_ArrayAllocationBase.h>
#include <drx_core/containers/drx_ArrayBase.h>
#include <drx_core/containers/drx_Array.h>
#include <drx_core/containers/drx_LinkedListPointer.h>
#include <drx_core/misc/drx_ScopeGuard.h>
#include <drx_core/containers/drx_ListenerList.h>
#include <drx_core/containers/drx_OwnedArray.h>
#include <drx_core/containers/drx_ReferenceCountedArray.h>
#include <drx_core/containers/drx_SortedSet.h>
#include <drx_core/containers/drx_SparseSet.h>
#include <drx_core/containers/drx_AbstractFifo.h>
#include <drx_core/containers/drx_SingleThreadedAbstractFifo.h>
#include <drx_core/text/drx_NewLine.h>
#include <drx_core/text/drx_StringPool.h>
#include <drx_core/text/drx_Identifier.h>
#include <drx_core/text/drx_StringArray.h>
#include <drx_core/system/drx_SystemStats.h>
#include <drx_core/memory/drx_HeavyweightLeakedObjectDetector.h>
#include <drx_core/text/drx_StringPairArray.h>
#include <drx_core/text/drx_TextDiff.h>
#include <drx_core/text/drx_LocalisedStrings.h>
#include <drx_core/text/drx_Base64.h>
#include <drx_core/misc/drx_Functional.h>
#include <drx_core/containers/drx_Span.h>
#include <drx_core/misc/drx_Result.h>
#include <drx_core/misc/drx_Uuid.h>
#include <drx_core/misc/drx_ConsoleApplication.h>
#include <drx_core/containers/drx_Variant.h>
#include <drx_core/containers/drx_NamedValueSet.h>
#include <drx_core/json/drx_JSON.h>
#include <drx_core/containers/drx_DynamicObject.h>
#include <drx_core/containers/drx_HashMap.h>
#include <drx_core/containers/drx_FixedSizeFunction.h>
#include <drx_core/time/drx_RelativeTime.h>
#include <drx_core/time/drx_Time.h>
#include <drx_core/streams/drx_InputStream.h>
#include <drx_core/streams/drx_OutputStream.h>
#include <drx_core/streams/drx_BufferedInputStream.h>
#include <drx_core/streams/drx_MemoryInputStream.h>
#include <drx_core/streams/drx_MemoryOutputStream.h>
#include <drx_core/streams/drx_SubregionStream.h>
#include <drx_core/streams/drx_InputSource.h>
#include <drx_core/files/drx_File.h>
#include <drx_core/files/drx_DirectoryIterator.h>
#include <drx_core/files/drx_RangedDirectoryIterator.h>
#include <drx_core/detail/drx_NativeFileHandle.h>
#include <drx_core/files/drx_FileInputStream.h>
#include <drx_core/files/drx_FileOutputStream.h>
#include <drx_core/files/drx_FileSearchPath.h>
#include <drx_core/files/drx_MemoryMappedFile.h>
#include <drx_core/files/drx_TemporaryFile.h>
#include <drx_core/files/drx_FileFilter.h>
#include <drx_core/files/drx_WildcardFileFilter.h>
#include <drx_core/streams/drx_FileInputSource.h>
#include <drx_core/logging/drx_FileLogger.h>
#include <drx_core/json/drx_JSONUtils.h>
#include <drx_core/serialisation/drx_Serialisation.h>
#include <drx_core/json/drx_JSONSerialisation.h>
#include <drx_core/maths/drx_BigInteger.h>
#include <drx_core/maths/drx_Expression.h>
#include <drx_core/maths/drx_Random.h>
#include <drx_core/misc/drx_RuntimePermissions.h>
#include <drx_core/misc/drx_WindowsRegistry.h>
#include <drx_core/threads/drx_ChildProcess.h>
#include <drx_core/threads/drx_DynamicLibrary.h>
#include <drx_core/threads/drx_InterProcessLock.h>
#include <drx_core/threads/drx_Process.h>
#include <drx_core/threads/drx_SpinLock.h>
#include <drx_core/threads/drx_WaitableEvent.h>
#include <drx_core/threads/drx_Thread.h>
#include <drx_core/threads/drx_HighResolutionTimer.h>
#include <drx_core/threads/drx_ThreadLocalValue.h>
#include <drx_core/threads/drx_ThreadPool.h>
#include <drx_core/threads/drx_TimeSliceThread.h>
#include <drx_core/threads/drx_ReadWriteLock.h>
#include <drx_core/threads/drx_ScopedReadLock.h>
#include <drx_core/threads/drx_ScopedWriteLock.h>
#include <drx_core/network/drx_IPAddress.h>
#include <drx_core/network/drx_MACAddress.h>
#include <drx_core/network/drx_NamedPipe.h>
#include <drx_core/network/drx_Socket.h>
#include <drx_core/network/drx_URL.h>
#include <drx_core/network/drx_WebInputStream.h>
#include <drx_core/streams/drx_URLInputSource.h>
#include <drx_core/time/drx_PerformanceCounter.h>
#include <drx_core/unit_tests/drx_UnitTest.h>
#include <drx_core/xml/drx_XmlDocument.h>
#include <drx_core/xml/drx_XmlElement.h>
#include <drx_core/zip/drx_GZIPCompressorOutputStream.h>
#include <drx_core/zip/drx_GZIPDecompressorInputStream.h>
#include <drx_core/zip/drx_ZipFile.h>
#include <drx_core/containers/drx_PropertySet.h>
#include <drx_core/memory/drx_SharedResourcePointer.h>
#include <drx_core/memory/drx_AllocationHooks.h>
#include <drx_core/memory/drx_Reservoir.h>
#include <drx_core/files/drx_AndroidDocument.h>
#include <drx_core/streams/drx_AndroidDocumentInputSource.h>
#include <drx_core/misc/drx_OptionsHelpers.h>

#include <drx_core/detail/drx_CallbackListenerList.h>

#if DRX_CORE_INCLUDE_OBJC_HELPERS && (DRX_MAC || DRX_IOS)
 #include <drx_core/native/drx_CFHelpers_mac.h>
 #include <drx_core/native/drx_ObjCHelpers_mac.h>
#endif

#if DRX_CORE_INCLUDE_COM_SMART_PTR && DRX_WINDOWS
 #include <drx_core/native/drx_ComSmartPtr_windows.h>
#endif

#if DRX_CORE_INCLUDE_JNI_HELPERS && DRX_ANDROID
 #include <jni.h>
 #include <drx_core/native/drx_JNIHelpers_android.h>
#endif

#if DRX_UNIT_TESTS
 #include <drx_core/unit_tests/drx_UnitTestCategories.h>
#endif

#ifndef DOXYGEN
namespace drx
{
 /*
    As the very i64 class names here try to explain, the purpose of this code is to cause
    a linker error if not all of your compile units are consistent in the options that they
    enable before including DRX headers. The reason this is important is that if you have
    two cpp files, and one includes the drx headers with debug enabled, and another does so
    without that, then each will be generating code with different class layouts, and you'll
    get subtle and hard-to-track-down memory corruption!
 */
 #if DRX_DEBUG
  struct DRX_API this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode
  { this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode() noexcept; };
  static this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode compileUnitMismatchSentinel;
 #else
  struct DRX_API this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode
  { this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode() noexcept; };
  static this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode compileUnitMismatchSentinel;
 #endif
}
#endif

DRX_END_IGNORE_WARNINGS_MSVC

// In DLL builds, need to disable this warnings for other modules
#if defined (DRX_DLL_BUILD) || defined (DRX_DLL)
 DRX_IGNORE_MSVC (4251)
#endif
