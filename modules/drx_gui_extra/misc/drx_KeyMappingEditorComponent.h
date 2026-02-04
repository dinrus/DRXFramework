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
    A component to allow editing of the keymaps stored by a KeyPressMappingSet
    object.

    @see KeyPressMappingSet

    @tags{GUI}
*/
class DRX_API  KeyMappingEditorComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a KeyMappingEditorComponent.

        @param mappingSet   this is the set of mappings to display and edit. Make sure the
                            mappings object is not deleted before this component!
        @param showResetToDefaultButton     if true, then at the bottom of the list, the
                                            component will include a 'reset to defaults' button.
    */
    KeyMappingEditorComponent (KeyPressMappingSet& mappingSet,
                               b8 showResetToDefaultButton);

    /** Destructor. */
    ~KeyMappingEditorComponent() override;

    //==============================================================================
    /** Sets up the colours to use for parts of the component.

        @param mainBackground       colour to use for most of the background
        @param textColor           colour to use for the text
    */
    z0 setColors (Color mainBackground,
                     Color textColor);

    /** Returns the KeyPressMappingSet that this component is acting upon. */
    KeyPressMappingSet& getMappings() const noexcept                { return mappings; }

    /** Returns the ApplicationCommandManager that this component is connected to. */
    ApplicationCommandManager& getCommandManager() const noexcept   { return mappings.getCommandManager(); }


    //==============================================================================
    /** Can be overridden if some commands need to be excluded from the list.

        By default this will use the KeyPressMappingSet's shouldCommandBeVisibleInEditor()
        method to decide what to return, but you can override it to handle special cases.
    */
    virtual b8 shouldCommandBeIncluded (CommandID commandID);

    /** Can be overridden to indicate that some commands are shown as read-only.

        By default this will use the KeyPressMappingSet's shouldCommandBeReadOnlyInEditor()
        method to decide what to return, but you can override it to handle special cases.
    */
    virtual b8 isCommandReadOnly (CommandID commandID);

    /** This can be overridden to let you change the format of the string used
        to describe a keypress.

        This is handy if you're using non-standard KeyPress objects, e.g. for custom
        keys that are triggered by something else externally. If you override the
        method, be sure to let the base class's method handle keys you're not
        interested in.
    */
    virtual Txt getDescriptionForKeyPress (const KeyPress& key);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId  = 0x100ad00,    /**< The background colour to fill the editor background. */
        textColorId        = 0x100ad01,    /**< The colour for the text. */
    };

    //==============================================================================
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    z0 resized() override;

private:
    //==============================================================================
    KeyPressMappingSet& mappings;
    TreeView tree;
    TextButton resetButton;

    class TopLevelItem;
    class ChangeKeyButton;
    class MappingItem;
    class CategoryItem;
    class ItemComponent;
    std::unique_ptr<TopLevelItem> treeItem;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingEditorComponent)
};

} // namespace drx
