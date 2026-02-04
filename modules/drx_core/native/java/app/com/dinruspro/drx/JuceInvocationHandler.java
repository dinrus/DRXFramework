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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import java.lang.reflect.*;

public class DrxInvocationHandler implements InvocationHandler
{
        public DrxInvocationHandler (i64 nativeContextRef)
        {
                nativeContext = nativeContextRef;
        }

        public z0 clear()
        {
                nativeContext = 0;
        }

        @Override
        public z0 finalize()
        {
                if (nativeContext != 0)
                        dispatchFinalize (nativeContext);
        }

        @Override
        public Object invoke (Object proxy, Method method, Object[] args) throws Throwable
        {
                if (nativeContext != 0)
                        return dispatchInvoke (nativeContext, proxy, method, args);

                return null;
        }

        //==============================================================================
        private i64 nativeContext = 0;

        private native z0 dispatchFinalize (i64 nativeContextRef);
        private native Object dispatchInvoke (i64 nativeContextRef, Object proxy, Method method, Object[] args);
}
