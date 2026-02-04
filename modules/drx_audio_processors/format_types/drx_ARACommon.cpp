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

#if (DRX_PLUGINHOST_ARA && (DRX_PLUGINHOST_VST3 || DRX_PLUGINHOST_AU) && (DRX_MAC || DRX_WINDOWS || DRX_LINUX))

namespace drx
{

static z0 dummyARAInterfaceAssert (ARA::ARAAssertCategory, ukk, tukk)
{}

static ARA::ARAInterfaceConfiguration createInterfaceConfig (const ARA::ARAFactory* araFactory)
{
    static auto* assertFunction = &dummyARAInterfaceAssert;

   #if ARA_VALIDATE_API_CALLS
    assertFunction = &::ARA::ARAInterfaceAssert;
    static std::once_flag flag;
    std::call_once (flag, [] { ARA::ARASetExternalAssertReference (&assertFunction); });
   #endif

    return makeARASizedStruct (&ARA::ARAInterfaceConfiguration::assertFunctionAddress,
                               jmin (araFactory->highestSupportedApiGeneration, (ARA::ARAAPIGeneration) ARA::kARAAPIGeneration_2_X_Draft),
                               &assertFunction);
}

/*  If the provided ARAFactory is not yet in use it constructs a new shared_ptr that will call the
    provided onDelete function inside the custom deleter of this new shared_ptr instance.

    The onDelete function is responsible for releasing the resources that guarantee the validity of
    the wrapped ARAFactory*.

    If however the ARAFactory is already in use the function will just return a copy of the already
    existing shared_ptr and call the onDelete function immediately. This is to ensure that the
    ARAFactory is only uninitialised when no plugin instance can be using it.

    On both platforms the onDelete function is used to release resources that ensure that the module
    providing the ARAFactory* remains loaded.
*/
static std::shared_ptr<const ARA::ARAFactory> getOrCreateARAFactory (const ARA::ARAFactory* ptr,
                                                                     std::function<z0()> onDelete)
{
    DRX_ASSERT_MESSAGE_THREAD

    static std::unordered_map<const ARA::ARAFactory*, std::weak_ptr<const ARA::ARAFactory>> cache;

    auto& cachePtr = cache[ptr];

    if (const auto obj = cachePtr.lock())
    {
        onDelete();
        return obj;
    }

    const auto interfaceConfig = createInterfaceConfig (ptr);
    ptr->initializeARAWithConfiguration (&interfaceConfig);
    const auto obj = std::shared_ptr<const ARA::ARAFactory> (ptr, [deleter = std::move (onDelete)] (const ARA::ARAFactory* factory)
                                                             {
                                                                 factory->uninitializeARA();
                                                                 deleter();
                                                             });
    cachePtr = obj;
    return obj;
}

}

#endif
