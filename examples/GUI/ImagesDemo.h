/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             ImagesDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Displays image files.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ImagesDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class ImagesDemo final : public Component,
                         public FileBrowserListener
{
public:
    ImagesDemo()
    {
        setOpaque (true);

        fileTree.setTitle ("Files");
        fileTree.addListener (this);
        fileTree.setColor (TreeView::backgroundColorId, Colors::grey);
        addAndMakeVisible (fileTree);

        addAndMakeVisible (resizerBar);

        addAndMakeVisible (imagePreview);

        // we have to set up our StretchableLayoutManager so it know the limits and preferred sizes of it's contents
        stretchableManager.setItemLayout (0,            // for the fileTree
                                          -0.1, -0.9,   // must be between 50 pixels and 90% of the available space
                                          -0.3);        // and its preferred size is 30% of the total available space

        stretchableManager.setItemLayout (1,            // for the resize bar
                                          5, 5, 5);     // hard limit to 5 pixels

        stretchableManager.setItemLayout (2,            // for the imagePreview
                                          -0.1, -0.9,   // size must be between 50 pixels and 90% of the available space
                                          -0.7);        // and its preferred size is 70% of the total available space

        setSize (500, 500);

        RuntimePermissions::request (RuntimePermissions::readMediaImages, [self = SafePointer { this }] (b8 granted)
        {
            if (self == nullptr)
                return;

            if (! granted)
            {
                AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                                  "Permissions warning",
                                                  "External storage access permission not granted, some files"
                                                  " may be inaccessible.");
                return;
            }

            self->imageList.setDirectory (File::getSpecialLocation (File::userPicturesDirectory), true, true);
            self->directoryThread.startThread (Thread::Priority::background);
        });
    }

    ~ImagesDemo() override
    {
        fileTree.removeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::white);
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (4);

        // make a list of two of our child components that we want to reposition
        Component* comps[] = { &fileTree, &resizerBar, &imagePreview };

        // this will position the 3 components, one above the other, to fit
        // vertically into the rectangle provided.
        stretchableManager.layOutComponents (comps, 3,
                                             r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                             true, true);
    }

private:
    WildcardFileFilter imagesWildcardFilter  { "*.jpeg;*.jpg;*.png;*.gif", "*", "Image File Filter"};
    TimeSliceThread directoryThread          { "Image File Scanner Thread" };
    DirectoryContentsList imageList          { &imagesWildcardFilter, directoryThread };
    FileTreeComponent fileTree               { imageList };

    ImageComponent imagePreview;

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar { &stretchableManager, 1, false };

    z0 selectionChanged() override
    {
        // we're only really interested in when the selection changes, regardless of if it was
        // clicked or not so we'll only override this method
        auto selectedFile = fileTree.getSelectedFile();

        if (selectedFile.existsAsFile())
            imagePreview.setImage (ImageCache::getFromFile (selectedFile));

        // the image cache is a handy way to load images from files or directly from memory and
        // will keep them hanging around for a few seconds in case they are requested elsewhere
    }

    z0 fileClicked (const File&, const MouseEvent&) override {}
    z0 fileDoubleClicked (const File&)              override {}
    z0 browserRootChanged (const File&)             override {}

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagesDemo)
};
