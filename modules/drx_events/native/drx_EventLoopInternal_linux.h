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

struct LinuxEventLoopInternal
{
    /**
        @internal

        Receives notifications when a fd callback is added or removed.

        This is useful for VST3 plugins that host other VST3 plugins. In this scenario, the 'inner'
        plugin may want to register additional file descriptors on top of those registered by the
        'outer' plugin. In order for this to work, the outer plugin must in turn pass this request
        on to the host, to indicate that the outer plugin wants to receive FD callbacks for both the
        inner and outer plugin.
    */
    struct Listener
    {
        virtual ~Listener() = default;
        virtual z0 fdCallbacksChanged() = 0;
    };

    /** @internal */
    static z0 registerLinuxEventLoopListener (Listener&);
    /** @internal */
    static z0 deregisterLinuxEventLoopListener (Listener&);
    /** @internal */
    static z0 invokeEventLoopCallbackForFd (i32);
    /** @internal */
    static std::vector<i32> getRegisteredFds();
};

} // namespace drx
