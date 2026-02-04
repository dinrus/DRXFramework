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

namespace drx::LinuxEventLoop
{

    /** Registers a callback that will be called when a file descriptor is ready for I/O.

        This will add the given file descriptor to the internal set of file descriptors
        that will be passed to the poll() call. When this file descriptor has data to read
        the readCallback will be called.

        @param fd            the file descriptor to be monitored
        @param readCallback  a callback that will be called when the file descriptor has
                             data to read. The file descriptor will be passed as an argument
        @param eventMask     a bit mask specifying the events you are interested in for the
                             file descriptor. The possible values for this are defined in
                             <poll.h>
    */
    z0 registerFdCallback (i32 fd, std::function<z0 (i32)> readCallback, short eventMask = 1 /*POLLIN*/);

    /** Unregisters a previously registered file descriptor.

        @see registerFdCallback
    */
    z0 unregisterFdCallback (i32 fd);

} // namespace drx::LinuxEventLoop
