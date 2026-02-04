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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#ifdef _WIN32
 #undef UNICODE
 #undef _UNICODE

 #define UNICODE 1
 #define _UNICODE 1

 #include <windows.h>
 #include <tchar.h>
 HMODULE dlopen (const TCHAR* filename, i32) { return LoadLibrary (filename); }
 FARPROC dlsym (HMODULE handle, tukk name) { return GetProcAddress (handle, name); }
 static z0 printError()
 {
     constexpr DWORD numElements = 256;
     TCHAR messageBuffer[numElements]{};

     FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    GetLastError(),
                    MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                    messageBuffer,
                    numElements - 1,
                    nullptr);

     _tprintf (_T ("%s"), messageBuffer);
 }

 enum { RTLD_LAZY = 0 };

 class ArgList
 {
 public:
     ArgList (i32, tukk*) {}
     ArgList (const ArgList&) = delete;
     ArgList (ArgList&&) = delete;
     ArgList& operator= (const ArgList&) = delete;
     ArgList& operator= (ArgList&&) = delete;
     ~ArgList() { LocalFree (argv); }

     LPWSTR get (i32 i) const { return argv[i]; }

     i32 size() const { return argc; }

 private:
     i32 argc = 0;
     LPWSTR* argv = CommandLineToArgvW (GetCommandLineW(), &argc);
 };

 static std::vector<t8> toUTF8 (const TCHAR* str)
 {
     const auto numBytes = WideCharToMultiByte (CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
     std::vector<t8> result (numBytes);
     WideCharToMultiByte (CP_UTF8, 0, str, -1, result.data(), static_cast<i32> (result.size()), nullptr, nullptr);
     return result;
 }

#else
 #include <dlfcn.h>
 static z0 printError() { printf ("%s\n", dlerror()); }
 class ArgList
 {
 public:
     ArgList (i32 argcIn, tukk* argvIn) : argc (argcIn), argv (argvIn) {}
     ArgList (const ArgList&) = delete;
     ArgList (ArgList&&) = delete;
     ArgList& operator= (const ArgList&) = delete;
     ArgList& operator= (ArgList&&) = delete;
     ~ArgList() = default;

     tukk get (i32 i) const { return argv[i]; }

     i32 size() const { return argc; }

 private:
     i32 argc = 0;
     tukk* argv = nullptr;
 };

 static std::vector<t8> toUTF8 (tukk str) { return std::vector<t8> (str, str + std::strlen (str) + 1); }
#endif

// Replicating part of the LV2 header here so that we don't have to set up any
// custom include paths for this file.
// Normally this would be a bad idea, but the LV2 API has to keep these definitions
// in order to remain backwards-compatible.

extern "C"
{
    typedef struct LV2_Descriptor
    {
        ukk a;
        ukk b;
        ukk c;
        ukk d;
        ukk e;
        ukk f;
        ukk g;
        ukk (*extension_data)(tukk uri);
    } LV2_Descriptor;
}

i32 main (i32 argc, tukk* argv)
{
    const ArgList argList { argc, argv };

    if (argList.size() != 2)
        return 1;

    const auto* libraryPath = argList.get (1);

    struct RecallFeature
    {
        i32 (*doRecall) (tukk);
    };

    if (auto* handle = dlopen (libraryPath, RTLD_LAZY))
    {
        if (auto* getDescriptor = reinterpret_cast<const LV2_Descriptor* (*) (u32)> (dlsym (handle, "lv2_descriptor")))
        {
            if (auto* descriptor = getDescriptor (0))
            {
                if (auto* extensionData = descriptor->extension_data)
                {
                    if (auto* recallFeature = reinterpret_cast<const RecallFeature*> (extensionData ("https://lv2-extensions.drx.com/turtle_recall")))
                    {
                        if (auto* doRecall = recallFeature->doRecall)
                        {
                            const auto converted = toUTF8 (libraryPath);
                            return doRecall (converted.data());
                        }
                    }
                }
            }
        }
    }
    else
    {
        printError();
    }

    return 1;
}
