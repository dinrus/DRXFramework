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
    A base class for components that display a list of the files in a directory.

    @see DirectoryContentsList

    @tags{GUI}
*/
class DRX_API  DirectoryContentsDisplayComponent
{
public:
    //==============================================================================
    /** Creates a DirectoryContentsDisplayComponent for a given list of files. */
    DirectoryContentsDisplayComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    virtual ~DirectoryContentsDisplayComponent();

    //==============================================================================
    /** The list that this component is displaying */
    DirectoryContentsList& directoryContentsList;

    //==============================================================================
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    virtual i32 getNumSelectedFiles() const = 0;

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    virtual File getSelectedFile (i32 index) const = 0;

    /** Deselects any selected files. */
    virtual z0 deselectAllFiles() = 0;

    /** Scrolls this view to the top. */
    virtual z0 scrollToTop() = 0;

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    virtual z0 setSelectedFile (const File&) = 0;

    //==============================================================================
    /** Adds a listener to be told when files are selected or clicked.
        @see removeListener
    */
    z0 addListener (FileBrowserListener* listener);

    /** Removes a listener.
        @see addListener
    */
    z0 removeListener (FileBrowserListener* listener);


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the list.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        highlightColorId          = 0x1000540, /**< The colour to use to fill a highlighted row of the list. */
        textColorId               = 0x1000541, /**< The colour for the text. */
        highlightedTextColorId    = 0x1000542  /**< The colour with which to draw the text in highlighted sections. */
    };

    //==============================================================================
    /** @internal */
    z0 sendSelectionChangeMessage();
    /** @internal */
    z0 sendDoubleClickMessage (const File&);
    /** @internal */
    z0 sendMouseClickMessage (const File&, const MouseEvent&);

protected:
    //==============================================================================
    ListenerList<FileBrowserListener> listeners;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectoryContentsDisplayComponent)
};

} // namespace drx
