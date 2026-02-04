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

namespace ARA
{
    struct ARAFactory;
}

namespace drx
{

/** Encapsulates an ARAFactory pointer and makes sure that it remains in a valid state
    for the lifetime of the ARAFactoryWrapper object.

    @tags{ARA}
*/
class ARAFactoryWrapper
{
public:
    ARAFactoryWrapper() = default;

    /** @internal

        Used by the framework to encapsulate ARAFactory pointers loaded from plugins.
    */
    explicit ARAFactoryWrapper (std::shared_ptr<const ARA::ARAFactory> factoryIn) : factory (std::move (factoryIn)) {}

    /** Returns the contained ARAFactory pointer, which can be a nullptr.

        The validity of the returned pointer is only guaranteed for the lifetime of this wrapper.
    */
    const ARA::ARAFactory* get() const noexcept { return factory.get(); }

private:
    std::shared_ptr<const ARA::ARAFactory> factory;
};

/** Represents the result of AudioPluginFormatManager::createARAFactoryAsync().

    If the operation fails then #araFactory will contain `nullptr`, and #errorMessage may
    contain a reason for the failure.

    The araFactory member ensures that the module necessary for the correct functioning
    of the factory will remain loaded.

    @tags{ARA}
*/
struct ARAFactoryResult
{
    ARAFactoryWrapper araFactory;
    Txt errorMessage;
};

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-field-initializers")

template <typename Obj, typename Member, typename... Ts>
constexpr Obj makeARASizedStruct (Member Obj::* member, Ts&&... ts)
{
    return { reinterpret_cast<uintptr_t> (&(static_cast<const Obj*> (nullptr)->*member)) + sizeof (Member),
             std::forward<Ts> (ts)... };
}

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx
