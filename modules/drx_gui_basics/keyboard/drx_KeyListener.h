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
/**
    Receives callbacks when keys are pressed.

    You can add a key listener to a component to be informed when that component
    gets key events. See the Component::addKeyListener method for more details.

    @see KeyPress, Component::addKeyListener, KeyPressMappingSet

    @tags{GUI}
*/
class DRX_API  KeyListener
{
public:
    /** Destructor. */
    virtual ~KeyListener() = default;

    //==============================================================================
    /** Called to indicate that a key has been pressed.

        If your implementation returns true, then the key event is considered to have
        been consumed, and will not be passed on to any other components. If it returns
        false, then the key will be passed to other components that might want to use it.

        @param key                      the keystroke, including modifier keys
        @param originatingComponent     the component that received the key event
        @see keyStateChanged, Component::keyPressed
    */
    virtual b8 keyPressed (const KeyPress& key,
                             Component* originatingComponent) = 0;

    /** Called when any key is pressed or released.

        When this is called, classes that might be interested in
        the state of one or more keys can use KeyPress::isKeyCurrentlyDown() to
        check whether their key has changed.

        If your implementation returns true, then the key event is considered to have
        been consumed, and will not be passed on to any other components. If it returns
        false, then the key will be passed to other components that might want to use it.

        @param originatingComponent     the component that received the key event
        @param isKeyDown                true if a key is being pressed, false if one is being released
        @see KeyPress, Component::keyStateChanged
    */
    virtual b8 keyStateChanged (b8 isKeyDown, Component* originatingComponent);
};

} // namespace drx
