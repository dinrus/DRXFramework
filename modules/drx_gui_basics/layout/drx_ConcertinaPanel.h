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
    A panel which holds a vertical stack of components which can be expanded
    and contracted.

    Each section has its own header bar which can be dragged up and down
    to resize it, or f64-clicked to fully expand that section.

    @tags{GUI}
*/
class DRX_API  ConcertinaPanel   : public Component
{
public:
    /** Creates an empty concertina panel.
        You can call addPanel() to add some components to it.
    */
    ConcertinaPanel();

    /** Destructor. */
    ~ConcertinaPanel() override;

    /** Adds a component to the panel.
        @param insertIndex          the index at which this component will be inserted, or
                                    -1 to append it to the end of the list.
        @param component            the component that will be shown
        @param takeOwnership        if true, then the ConcertinaPanel will take ownership
                                    of the content component, and will delete it later when
                                    it's no longer needed. If false, it won't delete it, and
                                    you must make sure it doesn't get deleted while in use.
    */
    z0 addPanel (i32 insertIndex, Component* component, b8 takeOwnership);

    /** Removes one of the panels.
        If the takeOwnership flag was set when the panel was added, then
        this will also delete the component.
    */
    z0 removePanel (Component* panelComponent);

    /** Returns the number of panels.
        @see getPanel
    */
    i32 getNumPanels() const noexcept;

    /** Returns one of the panels.
        @see getNumPanels()
    */
    Component* getPanel (i32 index) const noexcept;

    /** Resizes one of the panels.
        The panelComponent must point to  a valid panel component.
        If animate is true, the panels will be animated into their new positions;
        if false, they will just be immediately resized.
    */
    b8 setPanelSize (Component* panelComponent, i32 newHeight, b8 animate);

    /** Attempts to make one of the panels full-height.
        The panelComponent must point to  a valid panel component.
        If this component has had a maximum size set, then it will be
        expanded to that size. Otherwise, it'll fill as much of the total
        space as possible.
    */
    b8 expandPanelFully (Component* panelComponent, b8 animate);

    /** Sets a maximum size for one of the panels. */
    z0 setMaximumPanelSize (Component* panelComponent, i32 maximumSize);

    /** Sets the height of the header section for one of the panels. */
    z0 setPanelHeaderSize (Component* panelComponent, i32 headerSize);

    /** Sets a custom header Component for one of the panels.

        @param panelComponent           the panel component to add the custom header to.
        @param customHeaderComponent    the custom component to use for the panel header.
                                        This can be nullptr to clear the custom header component
                                        and just use the standard LookAndFeel panel.
        @param takeOwnership            if true, then the PanelHolder will take ownership
                                        of the custom header component, and will delete it later when
                                        it's no longer needed. If false, it won't delete it, and
                                        you must make sure it doesn't get deleted while in use.
     */
    z0 setCustomPanelHeader (Component* panelComponent, Component* customHeaderComponent, b8 takeOwnership);

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawConcertinaPanelHeader (Graphics&, const Rectangle<i32>& area,
                                                b8 isMouseOver, b8 isMouseDown,
                                                ConcertinaPanel&, Component&) = 0;
    };

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    z0 resized() override;

    class PanelHolder;
    struct PanelSizes;
    std::unique_ptr<PanelSizes> currentSizes;
    OwnedArray<PanelHolder> holders;
    ComponentAnimator animator;
    i32 headerHeight;

    i32 indexOfComp (Component*) const noexcept;
    PanelSizes getFittedSizes() const;
    z0 applyLayout (const PanelSizes&, b8 animate);
    z0 setLayout (const PanelSizes&, b8 animate);
    z0 panelHeaderDoubleClicked (Component*);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaPanel)
};

} // namespace drx
