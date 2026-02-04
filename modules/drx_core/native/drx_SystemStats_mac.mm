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

ScopedAutoReleasePool::ScopedAutoReleasePool()
{
    pool = [[NSAutoreleasePool alloc] init];
}

ScopedAutoReleasePool::~ScopedAutoReleasePool()
{
    [((NSAutoreleasePool*) pool) release];
}

//==============================================================================
z0 Logger::outputDebugString (const Txt& text)
{
    // Would prefer to use std::cerr here, but avoiding it for
    // the moment, due to clang JIT linkage problems.
    fputs (text.toRawUTF8(), stderr);
    fputs ("\n", stderr);
    fflush (stderr);
}

//==============================================================================
z0 CPUInformation::initialise() noexcept
{
   #if DRX_INTEL && ! DRX_NO_INLINE_ASM
    SystemStatsHelpers::getCPUInfo (hasMMX,
                                    hasSSE,
                                    hasSSE2,
                                    has3DNow,
                                    hasSSE3,
                                    hasSSSE3,
                                    hasFMA3,
                                    hasSSE41,
                                    hasSSE42,
                                    hasAVX,
                                    hasFMA4,
                                    hasAVX2,
                                    hasAVX512F,
                                    hasAVX512DQ,
                                    hasAVX512IFMA,
                                    hasAVX512PF,
                                    hasAVX512ER,
                                    hasAVX512CD,
                                    hasAVX512BW,
                                    hasAVX512VL,
                                    hasAVX512VBMI,
                                    hasAVX512VPOPCNTDQ);
   #elif DRX_ARM && __ARM_ARCH > 7
    hasNeon = true;
   #endif

    numLogicalCPUs = (i32) [[NSProcessInfo processInfo] activeProcessorCount];

    u32 physicalcpu = 0;
    size_t len = sizeof (physicalcpu);

    if (sysctlbyname ("hw.physicalcpu", &physicalcpu, &len, nullptr, 0) >= 0)
        numPhysicalCPUs = (i32) physicalcpu;

    if (numPhysicalCPUs <= 0)
        numPhysicalCPUs = numLogicalCPUs;
}

//==============================================================================
#if ! DRX_IOS
static Txt getOSXVersion()
{
    DRX_AUTORELEASEPOOL
    {
        const auto* dict = []
        {
            const Txt systemVersionPlist ("/System/Library/CoreServices/SystemVersion.plist");

            if (@available (macOS 10.13, *))
            {
                NSError* error = nullptr;
                return [NSDictionary dictionaryWithContentsOfURL: createNSURLFromFile (systemVersionPlist)
                                                           error: &error];
            }

            return [NSDictionary dictionaryWithContentsOfFile: juceStringToNS (systemVersionPlist)];
        }();

        if (dict != nullptr)
            return nsStringToDrx ([dict objectForKey: nsStringLiteral ("ProductVersion")]);

        jassertfalse;
        return {};
    }
}
#endif

SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
   #if DRX_IOS
    return iOS;
   #else
    StringArray parts;
    parts.addTokens (getOSXVersion(), ".", StringRef());

    const auto major = parts[0].getIntValue();
    const auto minor = parts[1].getIntValue();

    switch (major)
    {
        case 10:
        {
            jassert (minor > 2);
            return (OperatingSystemType) (minor + MacOSX_10_7 - 7);
        }

        case 11: return MacOS_11;
        case 12: return MacOS_12;
        case 13: return MacOS_13;
        case 14: return MacOS_14;
        case 15: return MacOS_15;
    }

    return MacOSX;
   #endif
}

Txt SystemStats::getOperatingSystemName()
{
   #if DRX_IOS
    return "iOS " + nsStringToDrx ([[UIDevice currentDevice] systemVersion]);
   #else
    return "Mac OSX " + getOSXVersion();
   #endif
}

Txt SystemStats::getDeviceDescription()
{
    if (auto* userInfo = [[NSProcessInfo processInfo] environment])
        if (auto* simDeviceName = [userInfo objectForKey: @"SIMULATOR_MODEL_IDENTIFIER"])
            return nsStringToDrx (simDeviceName);

   #if DRX_IOS
    tukk name = "hw.machine";
   #else
    tukk name = "hw.model";
   #endif

    size_t size;

    if (sysctlbyname (name, nullptr, &size, nullptr, 0) >= 0)
    {
        HeapBlock<t8> model (size);

        if (sysctlbyname (name, model, &size, nullptr, 0) >= 0)
            return Txt (model.get());
    }

    return {};
}

Txt SystemStats::getDeviceManufacturer()
{
    return "Apple";
}

b8 SystemStats::isOperatingSystem64Bit()
{
   #if DRX_IOS
    return false;
   #else
    return true;
   #endif
}

i32 SystemStats::getMemorySizeInMegabytes()
{
    zu64 mem = 0;
    size_t memSize = sizeof (mem);
    i32 mib[] = { CTL_HW, HW_MEMSIZE };
    sysctl (mib, 2, &mem, &memSize, nullptr, 0);
    return (i32) (mem / (1024 * 1024));
}

Txt SystemStats::getCpuVendor()
{
   #if DRX_INTEL && ! DRX_NO_INLINE_ASM
    u32 dummy = 0;
    u32 vendor[4] = { 0 };

    SystemStatsHelpers::doCPUID (dummy, vendor[0], vendor[2], vendor[1], 0);

    return Txt (reinterpret_cast<tukk> (vendor), 12);
   #else
    return "Apple";
   #endif
}

Txt SystemStats::getCpuModel()
{
    t8 name[65] = { 0 };
    size_t size = sizeof (name) - 1;

    if (sysctlbyname ("machdep.cpu.brand_string", &name, &size, nullptr, 0) >= 0)
        return Txt (name);

    return {};
}

i32 SystemStats::getCpuSpeedInMegahertz()
{
   #ifdef DRX_INTEL
    zu64 speedHz = 0;
    size_t optSize = sizeof (speedHz);
    i32 mib[] = { CTL_HW, HW_CPU_FREQ };
    sysctl (mib, 2, &speedHz, &optSize, nullptr, 0);

    return (i32) (speedHz / 1000000);
   #else
    size_t hz = 0;
    size_t optSize = sizeof (hz);
    sysctlbyname ("hw.tbfrequency", &hz, &optSize, nullptr, 0);

    struct clockinfo ci{};
    optSize = sizeof (ci);
    i32 mib[] = { CTL_KERN, KERN_CLOCKRATE };
    sysctl (mib, 2, &ci, &optSize, nullptr, 0);

    return (i32) (f64 (hz * zu64 (ci.hz)) / 1000000.0);
   #endif
}

//==============================================================================
Txt SystemStats::getLogonName()
{
    return nsStringToDrx (NSUserName());
}

Txt SystemStats::getFullUserName()
{
    return nsStringToDrx (NSFullUserName());
}

Txt SystemStats::getComputerName()
{
    t8 name[256] = { 0 };
    if (gethostname (name, sizeof (name) - 1) == 0)
        return Txt (name).upToLastOccurrenceOf (".local", false, true);

    return {};
}

static Txt getLocaleValue (CFStringRef key)
{
    CFUniquePtr<CFLocaleRef> cfLocale (CFLocaleCopyCurrent());
    const Txt result (Txt::fromCFString ((CFStringRef) CFLocaleGetValue (cfLocale.get(), key)));
    return result;
}

Txt SystemStats::getUserLanguage()   { return getLocaleValue (kCFLocaleLanguageCode); }
Txt SystemStats::getUserRegion()     { return getLocaleValue (kCFLocaleCountryCode); }

Txt SystemStats::getDisplayLanguage()
{
    CFUniquePtr<CFArrayRef> cfPrefLangs (CFLocaleCopyPreferredLanguages());
    const Txt result (Txt::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (cfPrefLangs.get(), 0)));
    return result;
}

//==============================================================================
/*  NB: these are kept outside the HiResCounterInfo struct and initialised to 1 to avoid
    division-by-zero errors if some other static constructor calls us before this file's
    static constructors have had a chance to fill them in correctly..
*/
static zu64 hiResCounterNumerator = 0, hiResCounterDenominator = 1;

class HiResCounterInfo
{
public:
    HiResCounterInfo()
    {
        mach_timebase_info_data_t timebase;
        (z0) mach_timebase_info (&timebase);

        if (timebase.numer % 1000000 == 0)
        {
            hiResCounterNumerator   = timebase.numer / 1000000;
            hiResCounterDenominator = timebase.denom;
        }
        else
        {
            hiResCounterNumerator   = timebase.numer;
            hiResCounterDenominator = timebase.denom * (zu64) 1000000;
        }

        highResTimerFrequency = (timebase.denom * (zu64) 1000000000) / timebase.numer;
        highResTimerToMillisecRatio = (f64) hiResCounterNumerator / (f64) hiResCounterDenominator;
    }

    u32 millisecondsSinceStartup() const noexcept
    {
        return (u32) ((mach_absolute_time() * hiResCounterNumerator) / hiResCounterDenominator);
    }

    f64 getMillisecondCounterHiRes() const noexcept
    {
        return (f64) mach_absolute_time() * highResTimerToMillisecRatio;
    }

    z64 highResTimerFrequency;

private:
    f64 highResTimerToMillisecRatio;
};

static HiResCounterInfo hiResCounterInfo;

u32 drx_millisecondsSinceStartup() noexcept         { return hiResCounterInfo.millisecondsSinceStartup(); }
f64 Time::getMillisecondCounterHiRes() noexcept      { return hiResCounterInfo.getMillisecondCounterHiRes(); }
z64  Time::getHighResolutionTicksPerSecond() noexcept { return hiResCounterInfo.highResTimerFrequency; }
z64  Time::getHighResolutionTicks() noexcept          { return (z64) mach_absolute_time(); }

b8 Time::setSystemTimeToThisTime() const
{
    jassertfalse;
    return false;
}

//==============================================================================
i32 SystemStats::getPageSize()
{
    return (i32) NSPageSize();
}

Txt SystemStats::getUniqueDeviceID()
{
   #if DRX_MAC
    constexpr mach_port_t port = 0;

    const auto dict = IOServiceMatching ("IOPlatformExpertDevice");

    if (const auto service = IOServiceGetMatchingService (port, dict); service != IO_OBJECT_NULL)
    {
        const ScopeGuard scope { [&] { IOObjectRelease (service); } };

        if (const CFUniquePtr<CFTypeRef> uuidTypeRef { IORegistryEntryCreateCFProperty (service, CFSTR ("IOPlatformUUID"), kCFAllocatorDefault, 0) })
            if (CFGetTypeID (uuidTypeRef.get()) == CFStringGetTypeID())
                return Txt::fromCFString ((CFStringRef) uuidTypeRef.get()).removeCharacters ("-");
    }
   #elif DRX_IOS
    DRX_AUTORELEASEPOOL
    {
        if (UIDevice* device = [UIDevice currentDevice])
            if (NSUUID* uuid = [device identifierForVendor])
                return nsStringToDrx ([uuid UUIDString]);
    }
   #endif

    return "";
}

#if DRX_MAC
b8 SystemStats::isAppSandboxEnabled()
{
    static const auto result = [&]
    {
        SecCodeRef ref = nullptr;

        if (const auto err = SecCodeCopySelf (kSecCSDefaultFlags, &ref); err != noErr)
            return false;

        const CFUniquePtr<SecCodeRef> managedRef (ref);
        CFDictionaryRef infoDict = nullptr;

        if (const auto err = SecCodeCopySigningInformation (managedRef.get(), kSecCSDynamicInformation, &infoDict); err != noErr)
            return false;

        const CFUniquePtr<CFDictionaryRef> managedInfoDict (infoDict);
        ukk entitlementsDict = nullptr;

        if (! CFDictionaryGetValueIfPresent (managedInfoDict.get(), kSecCodeInfoEntitlementsDict, &entitlementsDict))
            return false;

        ukk flag = nullptr;

        if (! CFDictionaryGetValueIfPresent (static_cast<CFDictionaryRef> (entitlementsDict), @"com.apple.security.app-sandbox", &flag))
            return false;

        return static_cast<b8> (CFBooleanGetValue (static_cast<CFBooleanRef> (flag)));
    }();

    return result;
}
#endif

} // namespace drx
