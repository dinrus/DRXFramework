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

#ifdef DRX_CORE_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1

#include <drx_core/drx_core.h>

#include <cctype>
#include <cstdarg>
#include <locale>
#include <thread>

#if ! (DRX_ANDROID || DRX_BSD)
 #include <sys/timeb.h>
 #include <cwctype>
#endif

#if DRX_WINDOWS
 #include <ctime>

 DRX_BEGIN_IGNORE_WARNINGS_MSVC (4091)
 #include <Dbghelp.h>
 DRX_END_IGNORE_WARNINGS_MSVC

 #if ! DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment (lib, "DbgHelp.lib")
 #endif

#else
 #if DRX_LINUX || DRX_BSD || DRX_ANDROID
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/errno.h>
  #include <unistd.h>
  #include <netinet/in.h>
 #endif

 #if DRX_WASM
  #include <stdio.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <errno.h>
  #include <unistd.h>
  #include <netinet/in.h>
  #include <sys/stat.h>
 #endif

 #if DRX_LINUX || DRX_BSD
  #include <stdio.h>
  #include <langinfo.h>
  #include <ifaddrs.h>
  #include <sys/resource.h>

  #if DRX_USE_CURL
   #include <curl/curl.h>
  #endif
 #endif

 #include <pwd.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <netinet/tcp.h>
 #include <sys/time.h>
 #include <net/if.h>
 #include <sys/ioctl.h>

 #if ! (DRX_ANDROID || DRX_WASM)
  #include <execinfo.h>
 #endif
#endif

#if DRX_MAC || DRX_IOS
 #include <xlocale.h>
 #include <mach/mach.h>
#endif

#if DRX_ANDROID
 #include <ifaddrs.h>
 #include <android/log.h>
#endif

#undef check

//==============================================================================
#include <drx_core/containers/drx_AbstractFifo.cpp>
#include <drx_core/containers/drx_ArrayBase.cpp>
#include <drx_core/containers/drx_NamedValueSet.cpp>
#include <drx_core/containers/drx_OwnedArray.cpp>
#include <drx_core/containers/drx_PropertySet.cpp>
#include <drx_core/containers/drx_ReferenceCountedArray.cpp>
#include <drx_core/containers/drx_SparseSet.cpp>
#include <drx_core/files/drx_DirectoryIterator.cpp>
#include <drx_core/files/drx_RangedDirectoryIterator.cpp>
#include <drx_core/files/drx_File.cpp>
#include <drx_core/files/drx_FileInputStream.cpp>
#include <drx_core/files/drx_FileOutputStream.cpp>
#include <drx_core/files/drx_FileSearchPath.cpp>
#include <drx_core/files/drx_TemporaryFile.cpp>
#include <drx_core/logging/drx_FileLogger.cpp>
#include <drx_core/logging/drx_Logger.cpp>
#include <drx_core/maths/drx_BigInteger.cpp>
#include <drx_core/maths/drx_Expression.cpp>
#include <drx_core/maths/drx_Random.cpp>
#include <drx_core/memory/drx_MemoryBlock.cpp>
#include <drx_core/memory/drx_AllocationHooks.cpp>
#include <drx_core/misc/drx_RuntimePermissions.cpp>
#include <drx_core/misc/drx_Result.cpp>
#include <drx_core/misc/drx_Uuid.cpp>
#include <drx_core/misc/drx_ConsoleApplication.cpp>
#include <drx_core/misc/drx_ScopeGuard.cpp>
#include <drx_core/network/drx_MACAddress.cpp>

#if ! DRX_WINDOWS
 #include <drx_core/native/drx_SharedCode_posix.h>
 #include <drx_core/native/drx_NamedPipe_posix.cpp>
#else
 #include <drx_core/native/drx_Files_windows.cpp>
#endif

#include <drx_core/zip/drx_zlib.h>
#include <drx_core/network/drx_NamedPipe.cpp>
#include <drx_core/network/drx_Socket.cpp>
#include <drx_core/network/drx_IPAddress.cpp>
#include <drx_core/streams/drx_BufferedInputStream.cpp>
#include <drx_core/streams/drx_FileInputSource.cpp>
#include <drx_core/streams/drx_InputStream.cpp>
#include <drx_core/streams/drx_MemoryInputStream.cpp>
#include <drx_core/streams/drx_MemoryOutputStream.cpp>
#include <drx_core/streams/drx_SubregionStream.cpp>
#include <drx_core/system/drx_SystemStats.cpp>
#include <drx_core/text/drx_CharacterFunctions.cpp>
#include <drx_core/text/drx_Identifier.cpp>
#include <drx_core/text/drx_LocalisedStrings.cpp>
#include <drx_core/text/drx_String.cpp>
#include <drx_core/streams/drx_OutputStream.cpp>
#include <drx_core/text/drx_StringArray.cpp>
#include <drx_core/text/drx_StringPairArray.cpp>
#include <drx_core/text/drx_StringPool.cpp>
#include <drx_core/text/drx_TextDiff.cpp>
#include <drx_core/text/drx_Base64.cpp>
#include <drx_core/threads/drx_ReadWriteLock.cpp>
#include <drx_core/threads/drx_Thread.cpp>
#include <drx_core/threads/drx_ThreadPool.cpp>
#include <drx_core/threads/drx_TimeSliceThread.cpp>
#include <drx_core/time/drx_PerformanceCounter.cpp>
#include <drx_core/time/drx_RelativeTime.cpp>
#include <drx_core/time/drx_Time.cpp>
#include <drx_core/unit_tests/drx_UnitTest.cpp>
#include <drx_core/containers/drx_Variant.cpp>
#include <drx_core/json/drx_JSON.cpp>
#include <drx_core/json/drx_JSONUtils.cpp>
#include <drx_core/containers/drx_DynamicObject.cpp>
#include <drx_core/xml/drx_XmlDocument.cpp>
#include <drx_core/xml/drx_XmlElement.cpp>
#include <drx_core/zip/drx_GZIPDecompressorInputStream.cpp>
#include <drx_core/zip/drx_GZIPCompressorOutputStream.cpp>
#include <drx_core/zip/drx_ZipFile.cpp>
#include <drx_core/files/drx_FileFilter.cpp>
#include <drx_core/files/drx_WildcardFileFilter.cpp>
#include <drx_core/native/drx_ThreadPriorities_native.h>
#include <drx_core/native/drx_PlatformTimerListener.h>

//==============================================================================
#if ! DRX_WINDOWS && (! DRX_ANDROID || __ANDROID_API__ >= 24)
 #include <drx_core/native/drx_IPAddress_posix.h>
#endif

//==============================================================================
#if DRX_MAC || DRX_IOS
 #include <drx_core/native/drx_Files_mac.mm>
 #include <drx_core/native/drx_Network_mac.mm>
 #include <drx_core/native/drx_Strings_mac.mm>
 #include <drx_core/native/drx_SharedCode_intel.h>
 #include <drx_core/native/drx_SystemStats_mac.mm>
 #include <drx_core/native/drx_Threads_mac.mm>
 #include <drx_core/native/drx_PlatformTimer_generic.cpp>
 #include <drx_core/native/drx_Process_mac.mm>

//==============================================================================
#elif DRX_WINDOWS
 #include <drx_core/native/drx_Network_windows.cpp>
 #include <drx_core/native/drx_Registry_windows.cpp>
 #include <drx_core/native/drx_SystemStats_windows.cpp>
 #include <drx_core/native/drx_Threads_windows.cpp>
 #include <drx_core/native/drx_PlatformTimer_generic.cpp>
 #include <drx_core/native/drx_PlatformTimer_windows.cpp>

//==============================================================================
#elif DRX_LINUX
 #include <drx_core/native/drx_CommonFile_linux.cpp>
 #include <drx_core/native/drx_Files_linux.cpp>
 #include <drx_core/native/drx_Network_linux.cpp>
 #if DRX_USE_CURL
  #include <drx_core/native/drx_Network_curl.cpp>
 #endif
 #include <drx_core/native/drx_SystemStats_linux.cpp>
 #include <drx_core/native/drx_Threads_linux.cpp>
 #include <drx_core/native/drx_PlatformTimer_generic.cpp>

//==============================================================================
#elif DRX_BSD
 #include <drx_core/native/drx_CommonFile_linux.cpp>
 #include <drx_core/native/drx_Files_linux.cpp>
 #include <drx_core/native/drx_Network_linux.cpp>
 #if DRX_USE_CURL
  #include <drx_core/native/drx_Network_curl.cpp>
 #endif
 #include <drx_core/native/drx_SharedCode_intel.h>
 #include <drx_core/native/drx_SystemStats_linux.cpp>
 #include <drx_core/native/drx_Threads_linux.cpp>
 #include <drx_core/native/drx_PlatformTimer_generic.cpp>

//==============================================================================
#elif DRX_ANDROID
 #include <drx_core/native/drx_CommonFile_linux.cpp>
 #include <drx_core/native/drx_JNIHelpers_android.cpp>
 #include <drx_core/native/drx_Files_android.cpp>
 #include <drx_core/native/drx_Misc_android.cpp>
 #include <drx_core/native/drx_Network_android.cpp>
 #include <drx_core/native/drx_SystemStats_android.cpp>
 #include <drx_core/native/drx_Threads_android.cpp>
 #include <drx_core/native/drx_RuntimePermissions_android.cpp>
 #include <drx_core/native/drx_PlatformTimer_generic.cpp>

//==============================================================================
#elif DRX_WASM
 #include <drx_core/native/drx_SystemStats_wasm.cpp>
 #include <drx_core/native/drx_PlatformTimer_generic.cpp>
#endif

#include <drx_core/files/drx_common_MimeTypes.h>
#include <drx_core/files/drx_common_MimeTypes.cpp>
#include <drx_core/native/drx_AndroidDocument_android.cpp>
#include <drx_core/threads/drx_HighResolutionTimer.cpp>
#include <drx_core/threads/drx_WaitableEvent.cpp>
#include <drx_core/network/drx_URL.cpp>

#if ! DRX_WASM
 #include <drx_core/threads/drx_ChildProcess.cpp>
 #include <drx_core/network/drx_WebInputStream.cpp>
 #include <drx_core/streams/drx_URLInputSource.cpp>
#endif

//==============================================================================
#if DRX_UNIT_TESTS
 #include <drx_core/containers/drx_HashMap_test.cpp>
 #include <drx_core/containers/drx_Optional_test.cpp>
 #include <drx_core/containers/drx_Enumerate_test.cpp>
 #include <drx_core/containers/drx_ListenerList_test.cpp>
 #include <drx_core/maths/drx_MathsFunctions_test.cpp>
 #include <drx_core/misc/drx_EnumHelpers_test.cpp>
 #include <drx_core/containers/drx_FixedSizeFunction_test.cpp>
 #include <drx_core/json/drx_JSONSerialisation_test.cpp>
 #include <drx_core/memory/drx_SharedResourcePointer_test.cpp>
 #include <drx_core/text/drx_CharPointer_UTF8_test.cpp>
 #include <drx_core/text/drx_CharPointer_UTF16_test.cpp>
 #include <drx_core/text/drx_CharPointer_UTF32_test.cpp>
 #if DRX_MAC || DRX_IOS
  #include <drx_core/native/drx_ObjCHelpers_mac_test.mm"
 #endif
#endif

//==============================================================================
namespace drx
{
/*
    As the very i64 class names here try to explain, the purpose of this code is to cause
    a linker error if not all of your compile units are consistent in the options that they
    enable before including DRX headers. The reason this is important is that if you have
    two cpp files, and one includes the drx headers with debug enabled, and the other doesn't,
    then each will be generating code with different memory layouts for the classes, and
    you'll get subtle and hard-to-track-down memory corruption bugs!
*/
#if DRX_DEBUG
 this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode
    ::this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode() noexcept {}
#else
 this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode
    ::this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode() noexcept {}
#endif
}

//#ifdef NEED_DLLIBSCOMPENSATION

#ifndef CMPtype
# define CMPtype i32
#endif

typedef f32 TFtype;

extern "C"{
///
CMPtype
__unordtf2 (TFtype a, TFtype b)
{
  return 0;
}
///
TFtype
__multf3 (TFtype a, TFtype b)
{
  return a*b;
}
///
TFtype
__addtf3 (TFtype a, TFtype b)
{
  return a+b;
}
///
CMPtype
__letf2 (TFtype a, TFtype b)
{
  return 0;
}
///
i32 th_uni2tis(){return 0;}
///
z0 th_brk (){}


}//extern C


