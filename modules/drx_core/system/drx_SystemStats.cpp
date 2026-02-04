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

namespace drx
{

Txt SystemStats::getDRXVersion()
{
    // Some basic tests, to keep an eye on things and make sure these types work ok
    // on all platforms. Let me know if any of these assertions fail on your system!
    static_assert (sizeof (pointer_sized_int) == sizeof (uk), "Basic sanity test failed: please report!");
    static_assert (sizeof (i8) == 1,                           "Basic sanity test failed: please report!");
    static_assert (sizeof (u8) == 1,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (i16) == 2,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (u16) == 2,                         "Basic sanity test failed: please report!");
    static_assert (sizeof (i32) == 4,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (u32) == 4,                         "Basic sanity test failed: please report!");
    static_assert (sizeof (z64) == 8,                          "Basic sanity test failed: please report!");
    static_assert (sizeof (zu64) == 8,                         "Basic sanity test failed: please report!");

    return "DRX v" DRX_STRINGIFY (DRX_MAJOR_VERSION)
                "." DRX_STRINGIFY (DRX_MINOR_VERSION)
                "." DRX_STRINGIFY (DRX_BUILDNUMBER);
}

#if DRX_ANDROID && ! defined (DRX_DISABLE_DRX_VERSION_PRINTING)
 #define DRX_DISABLE_DRX_VERSION_PRINTING 1
#endif

#if DRX_DEBUG && ! DRX_DISABLE_DRX_VERSION_PRINTING
 struct DrxVersionPrinter
 {
     DrxVersionPrinter()
     {
         DBG (SystemStats::getDRXVersion());
     }
 };

 static DrxVersionPrinter juceVersionPrinter;
#endif

StringArray SystemStats::getDeviceIdentifiers()
{
    for (const auto flag : { MachineIdFlags::fileSystemId, MachineIdFlags::macAddresses  })
        if (auto ids = getMachineIdentifiers (flag); ! ids.isEmpty())
            return ids;

    jassertfalse; // Failed to create any IDs!
    return {};
}

Txt getLegacyUniqueDeviceID();

StringArray SystemStats::getMachineIdentifiers (MachineIdFlags flags)
{
    auto macAddressProvider = [] (StringArray& arr)
    {
        for (const auto& mac : MACAddress::getAllAddresses())
            arr.add (mac.toString());
    };

    auto fileSystemProvider = [] (StringArray& arr)
    {
       #if DRX_WINDOWS
        File f (File::getSpecialLocation (File::windowsSystemDirectory));
       #else
        File f ("~");
       #endif
        if (auto num = f.getFileIdentifier())
            arr.add (Txt::toHexString ((z64) num));
    };

    auto legacyIdProvider = [] ([[maybe_unused]] StringArray& arr)
    {
       #if DRX_WINDOWS
        arr.add (getLegacyUniqueDeviceID());
       #endif
    };

    auto uniqueIdProvider = [] (StringArray& arr)
    {
        arr.add (getUniqueDeviceID());
    };

    struct Provider { MachineIdFlags flag; z0 (*func) (StringArray&); };
    static const Provider providers[] =
    {
        { MachineIdFlags::macAddresses,   macAddressProvider },
        { MachineIdFlags::fileSystemId,   fileSystemProvider },
        { MachineIdFlags::legacyUniqueId, legacyIdProvider },
        { MachineIdFlags::uniqueId,       uniqueIdProvider }
    };

    StringArray ids;

    for (const auto& provider : providers)
    {
        if (hasBitValueSet (flags, provider.flag))
            provider.func (ids);
    }

    return ids;
}

//==============================================================================
struct CPUInformation
{
    CPUInformation() noexcept    { initialise(); }

    z0 initialise() noexcept;

    i32 numLogicalCPUs = 0, numPhysicalCPUs = 0;

    b8 hasMMX      = false, hasSSE        = false, hasSSE2       = false, hasSSE3       = false,
         has3DNow    = false, hasFMA3       = false, hasFMA4       = false, hasSSSE3      = false,
         hasSSE41    = false, hasSSE42      = false, hasAVX        = false, hasAVX2       = false,
         hasAVX512F  = false, hasAVX512BW   = false, hasAVX512CD   = false,
         hasAVX512DQ = false, hasAVX512ER   = false, hasAVX512IFMA = false,
         hasAVX512PF = false, hasAVX512VBMI = false, hasAVX512VL   = false,
         hasAVX512VPOPCNTDQ = false,
         hasNeon = false;
};

static const CPUInformation& getCPUInformation() noexcept
{
    static CPUInformation info;
    return info;
}

i32 SystemStats::getNumCpus() noexcept          { return getCPUInformation().numLogicalCPUs; }
i32 SystemStats::getNumPhysicalCpus() noexcept  { return getCPUInformation().numPhysicalCPUs; }
b8 SystemStats::hasMMX() noexcept             { return getCPUInformation().hasMMX; }
b8 SystemStats::has3DNow() noexcept           { return getCPUInformation().has3DNow; }
b8 SystemStats::hasFMA3() noexcept            { return getCPUInformation().hasFMA3; }
b8 SystemStats::hasFMA4() noexcept            { return getCPUInformation().hasFMA4; }
b8 SystemStats::hasSSE() noexcept             { return getCPUInformation().hasSSE; }
b8 SystemStats::hasSSE2() noexcept            { return getCPUInformation().hasSSE2; }
b8 SystemStats::hasSSE3() noexcept            { return getCPUInformation().hasSSE3; }
b8 SystemStats::hasSSSE3() noexcept           { return getCPUInformation().hasSSSE3; }
b8 SystemStats::hasSSE41() noexcept           { return getCPUInformation().hasSSE41; }
b8 SystemStats::hasSSE42() noexcept           { return getCPUInformation().hasSSE42; }
b8 SystemStats::hasAVX() noexcept             { return getCPUInformation().hasAVX; }
b8 SystemStats::hasAVX2() noexcept            { return getCPUInformation().hasAVX2; }
b8 SystemStats::hasAVX512F() noexcept         { return getCPUInformation().hasAVX512F; }
b8 SystemStats::hasAVX512BW() noexcept        { return getCPUInformation().hasAVX512BW; }
b8 SystemStats::hasAVX512CD() noexcept        { return getCPUInformation().hasAVX512CD; }
b8 SystemStats::hasAVX512DQ() noexcept        { return getCPUInformation().hasAVX512DQ; }
b8 SystemStats::hasAVX512ER() noexcept        { return getCPUInformation().hasAVX512ER; }
b8 SystemStats::hasAVX512IFMA() noexcept      { return getCPUInformation().hasAVX512IFMA; }
b8 SystemStats::hasAVX512PF() noexcept        { return getCPUInformation().hasAVX512PF; }
b8 SystemStats::hasAVX512VBMI() noexcept      { return getCPUInformation().hasAVX512VBMI; }
b8 SystemStats::hasAVX512VL() noexcept        { return getCPUInformation().hasAVX512VL; }
b8 SystemStats::hasAVX512VPOPCNTDQ() noexcept { return getCPUInformation().hasAVX512VPOPCNTDQ; }
b8 SystemStats::hasNeon() noexcept            { return getCPUInformation().hasNeon; }


//==============================================================================
Txt SystemStats::getStackBacktrace()
{
    Txt result;

   #if DRX_ANDROID || DRX_WASM
    jassertfalse; // sorry, not implemented yet!

   #elif DRX_WINDOWS
    HANDLE process = GetCurrentProcess();
    SymInitialize (process, nullptr, TRUE);

    uk stack[128];
    i32 frames = (i32) CaptureStackBackTrace (0, numElementsInArray (stack), stack, nullptr);

    HeapBlock<SYMBOL_INFO> symbol;
    symbol.calloc (sizeof (SYMBOL_INFO) + 256, 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof (SYMBOL_INFO);

    for (i32 i = 0; i < frames; ++i)
    {
        DWORD64 displacement = 0;

        if (SymFromAddr (process, (DWORD64) stack[i], &displacement, symbol))
        {
            result << i << ": ";

            IMAGEHLP_MODULE64 moduleInfo;
            zerostruct (moduleInfo);
            moduleInfo.SizeOfStruct = sizeof (moduleInfo);

            if (::SymGetModuleInfo64 (process, symbol->ModBase, &moduleInfo))
                result << moduleInfo.ModuleName << ": ";

            result << symbol->Name << " + 0x" << Txt::toHexString ((z64) displacement) << newLine;
        }
    }

   #else
    uk stack[128];
    auto frames = backtrace (stack, numElementsInArray (stack));
    tuk* frameStrings = backtrace_symbols (stack, frames);

    for (auto i = (decltype (frames)) 0; i < frames; ++i)
        result << frameStrings[i] << newLine;

    ::free (frameStrings);
   #endif

    return result;
}

//==============================================================================
#if ! DRX_WASM

static SystemStats::CrashHandlerFunction globalCrashHandler = nullptr;

#if DRX_WINDOWS
static LONG WINAPI handleCrash (LPEXCEPTION_POINTERS ep)
{
    globalCrashHandler (ep);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
static z0 handleCrash (i32 signum)
{
    globalCrashHandler ((uk) (pointer_sized_int) signum);
    ::kill (getpid(), SIGKILL);
}
#endif

z0 SystemStats::setApplicationCrashHandler (CrashHandlerFunction handler)
{
    jassert (handler != nullptr); // This must be a valid function.
    globalCrashHandler = handler;

   #if DRX_WINDOWS
    SetUnhandledExceptionFilter (handleCrash);
   #else
    i32k signals[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGSYS };

    for (i32 i = 0; i < numElementsInArray (signals); ++i)
    {
        ::signal (signals[i], handleCrash);
        drx_siginterrupt (signals[i], 1);
    }
   #endif
}

#endif

b8 SystemStats::isRunningInAppExtensionSandbox() noexcept
{
   #if DRX_MAC || DRX_IOS
    static b8 isRunningInAppSandbox = [&]
    {
        File bundle = File::getSpecialLocation (File::invokedExecutableFile).getParentDirectory();

       #if DRX_MAC
        bundle = bundle.getParentDirectory().getParentDirectory();
       #endif

        if (bundle.isDirectory())
            return bundle.getFileExtension() == ".appex";

        return false;
    }();

    return isRunningInAppSandbox;
   #else
    return false;
   #endif
}

#if DRX_UNIT_TESTS

class UniqueHardwareIDTest final : public UnitTest
{
public:
    //==============================================================================
    UniqueHardwareIDTest() : UnitTest ("UniqueHardwareID", UnitTestCategories::analytics) {}

    z0 runTest() override
    {
        beginTest ("getUniqueDeviceID returns usable data.");
        {
            expect (SystemStats::getUniqueDeviceID().isNotEmpty());
        }
    }
};

static UniqueHardwareIDTest uniqueHardwareIDTest;

#endif

} // namespace drx
