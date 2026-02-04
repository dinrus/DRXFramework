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

z0 Logger::outputDebugString (const Txt& text)
{
    std::cerr << text << std::endl;
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()  { return WASM; }
Txt SystemStats::getOperatingSystemName()    { return "WASM"; }
b8 SystemStats::isOperatingSystem64Bit()      { return true; }
Txt SystemStats::getDeviceDescription()      { return "Web-browser"; }
Txt SystemStats::getDeviceManufacturer()     { return {}; }
Txt SystemStats::getCpuVendor()              { return {}; }
Txt SystemStats::getCpuModel()               { return {}; }
i32 SystemStats::getCpuSpeedInMegahertz()       { return 0; }
i32 SystemStats::getMemorySizeInMegabytes()     { return 0; }
i32 SystemStats::getPageSize()                  { return 0; }
Txt SystemStats::getLogonName()              { return {}; }
Txt SystemStats::getFullUserName()           { return {}; }
Txt SystemStats::getComputerName()           { return {}; }
Txt SystemStats::getUserLanguage()           { return {}; }
Txt SystemStats::getUserRegion()             { return {}; }
Txt SystemStats::getDisplayLanguage()        { return {}; }

//==============================================================================
z0 CPUInformation::initialise() noexcept
{
    numLogicalCPUs = 1;
    numPhysicalCPUs = 1;
}

//==============================================================================
u32 drx_millisecondsSinceStartup() noexcept
{
    return static_cast<u32> (emscripten_get_now());
}

z64 Time::getHighResolutionTicks() noexcept
{
    return static_cast<z64> (emscripten_get_now() * 1000.0);
}

z64 Time::getHighResolutionTicksPerSecond() noexcept
{
    return 1000000;  // (microseconds)
}

f64 Time::getMillisecondCounterHiRes() noexcept
{
    return emscripten_get_now();
}

b8 Time::setSystemTimeToThisTime() const
{
    return false;
}

DRX_API b8 DRX_CALLTYPE drx_isRunningUnderDebugger() noexcept
{
    return false;
}

} // namespace drx
