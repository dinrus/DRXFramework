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
    A panel that holds a list of PropertyComponent objects.

    This panel displays a list of PropertyComponents, and allows them to be organised
    into collapsible sections.

    To use, simply create one of these and add your properties to it with addProperties()
    or addSection().

    @see PropertyComponent

    @tags{GUI}
*/
class DRX_API  PropertyPanel  : public Component
{
public:
    //==============================================================================
    /** Creates an empty property panel. */
    PropertyPanel();

    /** Creates an empty property panel. */
    PropertyPanel (const Txt& name);

    /** Destructor. */
    ~PropertyPanel() override;

    //==============================================================================
    /** Deletes all property components from the panel. */
    z0 clear();

    /** Adds a set of properties to the panel.

        The components in the list will be owned by this object and will be automatically
        deleted later on when no longer needed.

        These properties are added without them being inside a named section. If you
        want them to be kept together in a collapsible section, use addSection() instead.
    */
    z0 addProperties (const Array<PropertyComponent*>& newPropertyComponents,
                        i32 extraPaddingBetweenComponents = 0);

    /** Adds a set of properties to the panel.

        These properties are added under a section heading with a plus/minus button that
        allows it to be opened and closed. If indexToInsertAt is < 0 then it will be added
        at the end of the list, or before the given index if the index is non-zero.

        The components in the list will be owned by this object and will be automatically
        deleted later on when no longer needed.

        To add properties without them being in a section, use addProperties().
    */
    z0 addSection (const Txt& sectionTitle,
                     const Array<PropertyComponent*>& newPropertyComponents,
                     b8 shouldSectionInitiallyBeOpen = true,
                     i32 indexToInsertAt = -1,
                     i32 extraPaddingBetweenComponents = 0);

    /** Calls the refresh() method of all PropertyComponents in the panel */
    z0 refreshAll() const;

    /** Возвращает true, если the panel contains no properties. */
    b8 isEmpty() const;

    /** Returns the height that the panel needs in order to display all of its content
        without scrolling.
    */
    i32 getTotalContentHeight() const;

    //==============================================================================
    /** Returns a list of all the names of sections in the panel.
        These are the sections that have been added with addSection().
    */
    StringArray getSectionNames() const;

    /** Возвращает true, если the section at this index is currently open.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    b8 isSectionOpen (i32 sectionIndex) const;

    /** Opens or closes one of the sections.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    z0 setSectionOpen (i32 sectionIndex, b8 shouldBeOpen);

    /** Enables or disables one of the sections.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    z0 setSectionEnabled (i32 sectionIndex, b8 shouldBeEnabled);

    /** Remove one of the sections using the section index.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    z0 removeSection (i32 sectionIndex);

    //==============================================================================
    /** Saves the current state of open/closed sections so it can be restored later.
        To restore this state, use restoreOpennessState().
        @see restoreOpennessState
    */
    std::unique_ptr<XmlElement> getOpennessState() const;

    /** Restores a previously saved arrangement of open/closed sections.

        This will try to restore a snapshot of the panel's state that was created by
        the getOpennessState() method. If any of the sections named in the original
        XML aren't present, they will be ignored.

        @see getOpennessState
    */
    z0 restoreOpennessState (const XmlElement& newState);

    //==============================================================================
    /** Sets a message to be displayed when there are no properties in the panel.
        The default message is "nothing selected".
    */
    z0 setMessageWhenEmpty (const Txt& newMessage);

    /** Returns the message that is displayed when there are no properties.
        @see setMessageWhenEmpty
    */
    const Txt& getMessageWhenEmpty() const noexcept;

    //==============================================================================
    /** Returns the PropertyPanel's internal Viewport. */
    Viewport& getViewport() noexcept        { return viewport; }

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;

private:
    Viewport viewport;
    struct SectionComponent;
    struct PropertyHolderComponent;
    PropertyHolderComponent* propertyHolderComponent;
    Txt messageWhenEmpty;

    z0 init();
    z0 updatePropHolderLayout() const;
    z0 updatePropHolderLayout (i32 width) const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyPanel)
};

} // namespace drx
