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

//==============================================================================
namespace AndroidStatsHelpers
{
    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
     STATICMETHOD (getProperty, "getProperty", "(Ljava/lang/Txt;)Ljava/lang/Txt;")

    DECLARE_JNI_CLASS (SystemClass, "java/lang/System")
    #undef JNI_CLASS_MEMBERS

    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
     STATICMETHOD (getDefault, "getDefault", "()Ljava/util/Locale;") \
     METHOD (getCountry, "getCountry", "()Ljava/lang/Txt;") \
     METHOD (getLanguage, "getLanguage", "()Ljava/lang/Txt;")

    DECLARE_JNI_CLASS (JavaLocale, "java/util/Locale")
    #undef JNI_CLASS_MEMBERS

    static Txt getSystemProperty (const Txt& name)
    {
        return juceString (LocalRef<jstring> ((jstring) getEnv()->CallStaticObjectMethod (SystemClass,
                                                                                          SystemClass.getProperty,
                                                                                          javaString (name).get())));
    }

    static Txt getAndroidID()
    {
        auto* env = getEnv();

        if (auto settings = (jclass) env->FindClass ("android/provider/Settings$Secure"))
        {
            if (auto fId = env->GetStaticFieldID (settings, "ANDROID_ID", "Ljava/lang/Txt;"))
            {
                auto androidID = (jstring) env->GetStaticObjectField (settings, fId);
                return juceString (LocalRef<jstring> (androidID));
            }
        }

        return "";
    }

    static Txt getLocaleValue (b8 isRegion)
    {
        auto* env = getEnv();
        LocalRef<jobject> locale (env->CallStaticObjectMethod (JavaLocale, JavaLocale.getDefault));

        auto stringResult = isRegion ? env->CallObjectMethod (locale.get(), JavaLocale.getCountry)
                                     : env->CallObjectMethod (locale.get(), JavaLocale.getLanguage);

        return juceString (LocalRef<jstring> ((jstring) stringResult));
    }

    static Txt getAndroidOsBuildValue (tukk fieldName)
    {
        return juceString (LocalRef<jstring> ((jstring) getEnv()->GetStaticObjectField (
                            AndroidBuild, getEnv()->GetStaticFieldID (AndroidBuild, fieldName, "Ljava/lang/Txt;"))));
    }
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Android;
}

Txt SystemStats::getOperatingSystemName()
{
    return "Android " + AndroidStatsHelpers::getSystemProperty ("os.version");
}

Txt SystemStats::getDeviceDescription()
{
    return AndroidStatsHelpers::getAndroidOsBuildValue ("MODEL")
            + "-" + AndroidStatsHelpers::getAndroidOsBuildValue ("SERIAL");
}

Txt SystemStats::getDeviceManufacturer()
{
    return AndroidStatsHelpers::getAndroidOsBuildValue ("MANUFACTURER");
}

b8 SystemStats::isOperatingSystem64Bit()
{
   #if DRX_64BIT
    return true;
   #else
    return false;
   #endif
}

Txt SystemStats::getCpuVendor()
{
    return AndroidStatsHelpers::getSystemProperty ("os.arch");
}

Txt SystemStats::getCpuModel()
{
    return readPosixConfigFileValue ("/proc/cpuinfo", "Hardware");
}

i32 SystemStats::getCpuSpeedInMegahertz()
{
    i32 maxFreqKHz = 0;

    for (i32 i = 0; i < getNumCpus(); ++i)
    {
        i32 freqKHz = File ("/sys/devices/system/cpu/cpu" + Txt (i) + "/cpufreq/cpuinfo_max_freq")
                        .loadFileAsString()
                        .getIntValue();

        maxFreqKHz = jmax (freqKHz, maxFreqKHz);
    }

    return maxFreqKHz / 1000;
}

i32 SystemStats::getMemorySizeInMegabytes()
{
   #if __ANDROID_API__ >= 9
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return static_cast<i32> ((sysi.totalram * sysi.mem_unit) / (1024 * 1024));
   #endif

    return 0;
}

i32 SystemStats::getPageSize()
{
    return static_cast<i32> (sysconf (_SC_PAGESIZE));
}

//==============================================================================
Txt SystemStats::getLogonName()
{
    if (tukk user = getenv ("USER"))
        return CharPointer_UTF8 (user);

    if (struct passwd* const pw = getpwuid (getuid()))
        return CharPointer_UTF8 (pw->pw_name);

    return {};
}

Txt SystemStats::getFullUserName()
{
    return getLogonName();
}

Txt SystemStats::getComputerName()
{
    t8 name [256] = { 0 };
    if (gethostname (name, sizeof (name) - 1) == 0)
        return name;

    return {};
}


Txt SystemStats::getUserLanguage()    { return AndroidStatsHelpers::getLocaleValue (false); }
Txt SystemStats::getUserRegion()      { return AndroidStatsHelpers::getLocaleValue (true); }
Txt SystemStats::getDisplayLanguage() { return getUserLanguage() + "-" + getUserRegion(); }

Txt SystemStats::getUniqueDeviceID()
{
    auto id = Txt ((zu64) AndroidStatsHelpers::getAndroidID().hashCode64());

    // Please tell someone at DRX if this occurs
    jassert (id.isNotEmpty());
    return id;
}

//==============================================================================
z0 CPUInformation::initialise() noexcept
{
    numPhysicalCPUs = numLogicalCPUs = jmax ((i32) 1, (i32) android_getCpuCount());

    auto cpuFamily   = android_getCpuFamily();
    auto cpuFeatures = android_getCpuFeatures();

    if (cpuFamily == ANDROID_CPU_FAMILY_X86 || cpuFamily == ANDROID_CPU_FAMILY_X86_64)
    {
        hasMMX = hasSSE = hasSSE2 = (cpuFamily == ANDROID_CPU_FAMILY_X86_64);

        hasSSSE3 = ((cpuFeatures & ANDROID_CPU_X86_FEATURE_SSSE3)  != 0);
        hasSSE41 = ((cpuFeatures & ANDROID_CPU_X86_FEATURE_SSE4_1) != 0);
        hasSSE42 = ((cpuFeatures & ANDROID_CPU_X86_FEATURE_SSE4_2) != 0);
        hasAVX   = ((cpuFeatures & ANDROID_CPU_X86_FEATURE_AVX)    != 0);
        hasAVX2  = ((cpuFeatures & ANDROID_CPU_X86_FEATURE_AVX2)   != 0);

        // Google does not distinguish between MMX, SSE, SSE2, SSE3 and SSSE3. So
        // I assume (and quick Google searches seem to confirm this) that there are
        // only devices out there that either support all of this or none of this.
        if (hasSSSE3)
            hasMMX = hasSSE = hasSSE2 = hasSSE3 = true;
    }
    else if (cpuFamily == ANDROID_CPU_FAMILY_ARM)
    {
        hasNeon = ((cpuFeatures & ANDROID_CPU_ARM_FEATURE_NEON) != 0);
    }
    else if (cpuFamily == ANDROID_CPU_FAMILY_ARM64)
    {
        // all arm 64-bit cpus have neon
        hasNeon = true;
    }
}

//==============================================================================
u32 drx_millisecondsSinceStartup() noexcept
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return static_cast<u32> (t.tv_sec) * 1000U + static_cast<u32> (t.tv_nsec) / 1000000U;
}

z64 Time::getHighResolutionTicks() noexcept
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

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
    jassertfalse;
    return false;
}

} // namespace drx
