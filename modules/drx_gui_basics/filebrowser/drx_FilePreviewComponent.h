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
    Base class for components that live inside a file chooser dialog box and
    show previews of the files that get selected.

    One of these allows special extra information to be displayed for files
    in a dialog box as the user selects them. Each time the current file or
    directory is changed, the selectedFileChanged() method will be called
    to allow it to update itself appropriately.

    @see FileChooser, ImagePreviewComponent

    @tags{GUI}
*/
class DRX_API  FilePreviewComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a FilePreviewComponent. */
    FilePreviewComponent();

    /** Destructor. */
    ~FilePreviewComponent() override;

    /** Called to indicate that the user's currently selected file has changed.

        @param newSelectedFile  the newly selected file or directory, which may be
                                a default File() object if none is selected.
    */
    virtual z0 selectedFileChanged (const File& newSelectedFile) = 0;


private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePreviewComponent)
};

} // namespace drx
