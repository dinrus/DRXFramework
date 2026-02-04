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
/** Manages a list of ActionListeners, and can send them messages.

    To quickly add methods to your class that can add/remove action
    listeners and broadcast to them, you can derive from this.

    @see ActionListener, ChangeListener

    @tags{Events}
*/
class DRX_API  ActionBroadcaster
{
public:
    //==============================================================================
    /** Creates an ActionBroadcaster. */
    ActionBroadcaster();

    /** Destructor. */
    virtual ~ActionBroadcaster();

    //==============================================================================
    /** Adds a listener to the list.
        Trying to add a listener that's already on the list will have no effect.
    */
    z0 addActionListener (ActionListener* listener);

    /** Removes a listener from the list.
        If the listener isn't on the list, this won't have any effect.
    */
    z0 removeActionListener (ActionListener* listener);

    /** Removes all listeners from the list. */
    z0 removeAllActionListeners();

    //==============================================================================
    /** Broadcasts a message to all the registered listeners.
        @see ActionListener::actionListenerCallback
    */
    z0 sendActionMessage (const Txt& message) const;


private:
    //==============================================================================
    class ActionMessage;
    friend class ActionMessage;

    SortedSet<ActionListener*> actionListeners;
    CriticalSection actionListenerLock;

    DRX_DECLARE_WEAK_REFERENCEABLE (ActionBroadcaster)
    DRX_DECLARE_NON_COPYABLE (ActionBroadcaster)
};

} // namespace drx
