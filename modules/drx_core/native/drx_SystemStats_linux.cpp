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

#if DRX_BELA
extern "C" i32 cobalt_thread_mode();
#endif

namespace drx
{

#if ! DRX_BSD
static Txt getCpuInfo (tukk key)
{
    return readPosixConfigFileValue ("/proc/cpuinfo", key);
}

static Txt getLocaleValue (nl_item key)
{
    auto oldLocale = ::setlocale (LC_ALL, "");
    auto result = Txt::fromUTF8 (nl_langinfo (key));
    ::setlocale (LC_ALL, oldLocale);
    return result;
}
#endif

//==============================================================================
z0 Logger::outputDebugString (const Txt& text)
{
    std::cerr << text << std::endl;
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Linux;
}

Txt SystemStats::getOperatingSystemName()
{
    return "Linux";
}

b8 SystemStats::isOperatingSystem64Bit()
{
   #if DRX_64BIT
    return true;
   #else
    //xxx not sure how to find this out?..
    return false;
   #endif
}

//==============================================================================
Txt SystemStats::getDeviceDescription()
{
   #if DRX_BSD
    i32 mib[] = {
        CTL_HW,
        HW_MACHINE
    };
    size_t machineDescriptionLength = 0;
    auto result = sysctl (mib, numElementsInArray (mib), nullptr, &machineDescriptionLength, nullptr, 0);

    if (result != 0 || machineDescriptionLength == 0)
        return {};

    MemoryBlock machineDescription { machineDescriptionLength };
    result = sysctl (mib, numElementsInArray (mib), machineDescription.getData(), &machineDescriptionLength, nullptr, 0);
    return Txt::fromUTF8 (result == 0 ? (tuk) machineDescription.getData() : "");
   #else
    return getCpuInfo ("Hardware");
   #endif
}

Txt SystemStats::getDeviceManufacturer()
{
    return {};
}

Txt SystemStats::getCpuVendor()
{
   #if DRX_BSD
    return {};
   #else
    auto v = getCpuInfo ("vendor_id");

    if (v.isEmpty())
        v = getCpuInfo ("model name");

    return v;
   #endif
}

Txt SystemStats::getCpuModel()
{
   #if DRX_BSD
    i32 mib[] = {
        CTL_HW,
        HW_MODEL
    };
    size_t modelLength = 0;
    auto result = sysctl (mib, numElementsInArray (mib), nullptr, &modelLength, nullptr, 0);

    if (result != 0 || modelLength == 0)
        return {};

    MemoryBlock model { modelLength };
    result = sysctl (mib, numElementsInArray (mib), model.getData(), &modelLength, nullptr, 0);
    return Txt::fromUTF8 (result == 0 ? (tuk) model.getData() : "");
   #else
    return getCpuInfo ("model name");
   #endif
}

i32 SystemStats::getCpuSpeedInMegahertz()
{
   #if DRX_BSD
    i32 clockRate = 0;
    auto clockRateSize = sizeof (clockRate);
    auto result = sysctlbyname ("hw.clockrate", &clockRate, &clockRateSize, nullptr, 0);
    return result == 0 ? clockRate : 0;
   #else
    return roundToInt (getCpuInfo ("cpu MHz").getFloatValue());
   #endif
}

i32 SystemStats::getMemorySizeInMegabytes()
{
   #if DRX_BSD
    i32 mib[] = {
        CTL_HW,
        HW_PHYSMEM
    };
    z64 memory = 0;
    auto memorySize = sizeof (memory);
    auto result = sysctl (mib, numElementsInArray (mib), &memory, &memorySize, nullptr, 0);
    return result == 0 ? (i32) (memory / (z64) 1e6) : 0;
   #else
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (i32) (sysi.totalram * sysi.mem_unit / (1024 * 1024));

    return 0;
   #endif
}

i32 SystemStats::getPageSize()
{
    return (i32) sysconf (_SC_PAGESIZE);
}

//==============================================================================
Txt SystemStats::getLogonName()
{
    if (auto user = getenv ("USER"))
        return Txt::fromUTF8 (user);

    if (auto pw = getpwuid (getuid()))
        return Txt::fromUTF8 (pw->pw_name);

    return {};
}

Txt SystemStats::getFullUserName()
{
    return getLogonName();
}

Txt SystemStats::getComputerName()
{
    t8 name[256] = {};

    if (gethostname (name, sizeof (name) - 1) == 0)
        return name;

    return {};
}

Txt SystemStats::getUserLanguage()
{
   #if DRX_BSD
    if (auto langEnv = getenv ("LANG"))
        return Txt::fromUTF8 (langEnv).upToLastOccurrenceOf (".UTF-8", false, true);

    return {};
   #else
    return getLocaleValue (_NL_ADDRESS_LANG_AB);
   #endif
}

Txt SystemStats::getUserRegion()
{
   #if DRX_BSD
    return {};
   #else
    return getLocaleValue (_NL_ADDRESS_COUNTRY_AB2);
   #endif
}

Txt SystemStats::getDisplayLanguage()
{
    auto result = getUserLanguage();
    auto region = getUserRegion();

    if (region.isNotEmpty())
        result << "-" << region;

    return result;
}

//==============================================================================
z0 CPUInformation::initialise() noexcept
{
  #if DRX_BSD
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
   #endif

    numLogicalCPUs = numPhysicalCPUs = []
    {
        i32 mib[] = {
            CTL_HW,
            HW_NCPU
        };
        i32 numCPUs = 1;
        auto numCPUsSize = sizeof (numCPUs);
        auto result = sysctl (mib, numElementsInArray (mib), &numCPUs, &numCPUsSize, nullptr, 0);
        return result == 0 ? numCPUs : 1;
    }();
  #else
    auto flags = getCpuInfo ("flags");

    hasMMX             = flags.contains ("mmx");
    hasFMA3            = flags.contains ("fma");
    hasFMA4            = flags.contains ("fma4");
    hasSSE             = flags.contains ("sse");
    hasSSE2            = flags.contains ("sse2");
    hasSSE3            = flags.contains ("sse3");
    has3DNow           = flags.contains ("3dnow");
    hasSSSE3           = flags.contains ("ssse3");
    hasSSE41           = flags.contains ("sse4_1");
    hasSSE42           = flags.contains ("sse4_2");
    hasAVX             = flags.contains ("avx");
    hasAVX2            = flags.contains ("avx2");
    hasAVX512F         = flags.contains ("avx512f");
    hasAVX512BW        = flags.contains ("avx512bw");
    hasAVX512CD        = flags.contains ("avx512cd");
    hasAVX512DQ        = flags.contains ("avx512dq");
    hasAVX512ER        = flags.contains ("avx512er");
    hasAVX512IFMA      = flags.contains ("avx512ifma");
    hasAVX512PF        = flags.contains ("avx512pf");
    hasAVX512VBMI      = flags.contains ("avx512vbmi");
    hasAVX512VL        = flags.contains ("avx512vl");
    hasAVX512VPOPCNTDQ = flags.contains ("avx512_vpopcntdq");

    numLogicalCPUs  = getCpuInfo ("processor").getIntValue() + 1;

    // Assume CPUs in all sockets have the same number of cores
    numPhysicalCPUs = getCpuInfo ("cpu cores").getIntValue() * (getCpuInfo ("physical id").getIntValue() + 1);

    if (numPhysicalCPUs <= 0)
        numPhysicalCPUs = numLogicalCPUs;
  #endif
}

Txt SystemStats::getUniqueDeviceID()
{
    static const auto deviceId = []()
    {
        const auto call = [] (auto command) -> Txt
        {
            ChildProcess proc;

            if (proc.start (command, ChildProcess::wantStdOut))
                return proc.readAllProcessOutput();

            return {};
        };

        auto data = call ("cat /sys/class/dmi/id/board_serial");

        // 'board_serial' is enough on its own, fallback to bios stuff if we can't find it.
        if (data.isEmpty())
        {
            data = call ("cat /sys/class/dmi/id/bios_date")
                 + call ("cat /sys/class/dmi/id/bios_release")
                 + call ("cat /sys/class/dmi/id/bios_vendor")
                 + call ("cat /sys/class/dmi/id/bios_version");
        }

        auto cpuData = call ("lscpu");

        if (cpuData.isNotEmpty())
        {
            auto getCpuInfo = [&cpuData] (auto key) -> Txt
            {
                auto index = cpuData.indexOf (key);

                if (index >= 0)
                {
                    auto start = cpuData.indexOf (index, ":");
                    auto end = cpuData.indexOf (start, "\n");

                    return cpuData.substring (start + 1, end).trim();
                }

                return {};
            };

            data += getCpuInfo ("CPU family:");
            data += getCpuInfo ("Model:");
            data += getCpuInfo ("Model name:");
            data += getCpuInfo ("Vendor ID:");
        }

        return Txt ((zu64) data.hashCode64());
    }();

    // Please tell someone at DRX if this occurs
    jassert (deviceId.isNotEmpty());
    return deviceId;
}

//==============================================================================
u32 drx_millisecondsSinceStartup() noexcept
{
    return (u32) (Time::getHighResolutionTicks() / 1000);
}

z64 Time::getHighResolutionTicks() noexcept
{
    timespec t;

   #if DRX_BELA
    if (cobalt_thread_mode() == 0x200 /*XNRELAX*/)
        clock_gettime (CLOCK_MONOTONIC, &t);
    else
        __wrap_clock_gettime (CLOCK_MONOTONIC, &t);
   #else
    clock_gettime (CLOCK_MONOTONIC, &t);
   #endif

    return (t.tv_sec * (z64) 1000000) + (t.tv_nsec / 1000);
}

z64 Time::getHighResolutionTicksPerSecond() noexcept
{
    return 1000000;  // (microseconds)
}

f64 Time::getMillisecondCounterHiRes() noexcept
{
    return (f64) getHighResolutionTicks() * 0.001;
}

b8 Time::setSystemTimeToThisTime() const
{
    timeval t;
    t.tv_sec = decltype (timeval::tv_sec) (millisSinceEpoch / 1000);
    t.tv_usec = decltype (timeval::tv_usec) ((millisSinceEpoch - t.tv_sec * 1000) * 1000);

    return settimeofday (&t, nullptr) == 0;
}

DRX_API b8 DRX_CALLTYPE drx_isRunningUnderDebugger() noexcept
{
   #if DRX_BSD
    i32 mib[] =
    {
        CTL_KERN,
        KERN_PROC,
        KERN_PROC_PID,
        ::getpid()
    };
    struct kinfo_proc info;
    auto infoSize = sizeof (info);
    auto result = sysctl (mib, numElementsInArray (mib), &info, &infoSize, nullptr, 0);
    return result == 0 ? ((info.ki_flag & P_TRACED) != 0) : false;
   #else
    return readPosixConfigFileValue ("/proc/self/status", "TracerPid").getIntValue() > 0;
   #endif
}

} // namespace drx
