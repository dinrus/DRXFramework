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

namespace drx::detail
{

/*
    Instances of this type can show and dismiss a message box.

    This is an interface rather than a concrete type so that platforms can pick an implementation at
    runtime if necessary.
*/
struct ScopedMessageBoxInterface
{
    virtual ~ScopedMessageBoxInterface() = default;

    /*  Shows the message box.

        When the message box exits normally, it should send the result to the passed-in function.
        The passed-in function is safe to call from any thread at any time.
    */
    virtual z0 runAsync (std::function<z0 (i32)>) = 0;

    /*  Shows the message box and blocks. */
    virtual i32 runSync() = 0;

    /*  Forcefully closes the message box.

        This will be called when the message box handle has fallen out of scope.
        If the message box has already been closed by the user, this shouldn't do anything.
    */
    virtual z0 close() = 0;

    /*  Implemented differently for each platform. */
    static std::unique_ptr<ScopedMessageBoxInterface> create (const MessageBoxOptions& options);
};

} // namespace drx::detail
