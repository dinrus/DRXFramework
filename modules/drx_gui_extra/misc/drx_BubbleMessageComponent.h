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
    A speech-bubble component that displays a short message.

    This can be used to show a message with the tail of the speech bubble
    pointing to a particular component or location on the screen.

    @see BubbleComponent

    @tags{GUI}
*/
class DRX_API  BubbleMessageComponent  : public BubbleComponent,
                                          private Timer
{
public:
    //==============================================================================
    /** Creates a bubble component.

        After creating one a BubbleComponent, do the following:
        - add it to an appropriate parent component, or put it on the
          desktop with Component::addToDesktop (0).
        - use the showAt() method to show a message.
        - it will make itself invisible after it times-out (and can optionally
          also delete itself), or you can reuse it somewhere else by calling
          showAt() again.
    */
    BubbleMessageComponent (i32 fadeOutLengthMs = 150);

    /** Destructor. */
    ~BubbleMessageComponent() override;

    //==============================================================================
    /** Shows a message bubble at a particular position.

        This shows the bubble with its stem pointing to the given location
        (coordinates being relative to its parent component).

        @param position                         the coords of the object to point to
        @param message                          the text to display
        @param numMillisecondsBeforeRemoving    how i64 to leave it on the screen before removing itself
                                                from its parent component. If this is 0 or less, it
                                                will stay there until manually removed.
        @param removeWhenMouseClicked           if this is true, the bubble will disappear as soon as a
                                                mouse button is pressed (anywhere on the screen)
        @param deleteSelfAfterUse               if true, then the component will delete itself after
                                                it becomes invisible
    */
    z0 showAt (const Rectangle<i32>& position,
                 const AttributedString& message,
                 i32 numMillisecondsBeforeRemoving,
                 b8 removeWhenMouseClicked = true,
                 b8 deleteSelfAfterUse = false);

    /** Shows a message bubble next to a particular component.

        This shows the bubble with its stem pointing at the given component.

        @param component                        the component that you want to point at
        @param message                          the text to display
        @param numMillisecondsBeforeRemoving    how i64 to leave it on the screen before removing itself
                                                from its parent component. If this is 0 or less, it
                                                will stay there until manually removed.
        @param removeWhenMouseClicked           if this is true, the bubble will disappear as soon as a
                                                mouse button is pressed (anywhere on the screen)
        @param deleteSelfAfterUse               if true, then the component will delete itself after
                                                it becomes invisible
    */
    z0 showAt (Component* component,
                 const AttributedString& message,
                 i32 numMillisecondsBeforeRemoving,
                 b8 removeWhenMouseClicked = true,
                 b8 deleteSelfAfterUse = false);


    //==============================================================================
    /** @internal */
    z0 getContentSize (i32& w, i32& h) override;
    /** @internal */
    z0 paintContent (Graphics& g, i32 w, i32 h) override;
    /** @internal */
    z0 timerCallback() override;

private:
    //==============================================================================
    i32 fadeOutLength, mouseClickCounter;
    TextLayout textLayout;
    z64 expiryTime;
    b8 deleteAfterUse;

    z0 createLayout (const AttributedString&);
    z0 init (i32, b8, b8);
    z0 hide (b8 fadeOut);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BubbleMessageComponent)
};

} // namespace drx
