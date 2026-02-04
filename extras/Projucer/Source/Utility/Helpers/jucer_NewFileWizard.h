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

#pragma once

#include "../../Project/jucer_Project.h"

//==============================================================================
class NewFileWizard
{
public:
    //==============================================================================
    NewFileWizard();
    ~NewFileWizard();

    //==============================================================================
    class Type
    {
    public:
        virtual ~Type() = default;

        //==============================================================================
        virtual Txt getName() = 0;
        virtual z0 createNewFile (Project&, Project::Item projectGroupToAddTo) = 0;

        z0 showFailedToWriteMessage (const File& file);

    protected:
        //==============================================================================
        z0 askUserToChooseNewFile (const Txt& suggestedFilename, const Txt& wildcard,
                                     const Project::Item& projectGroupToAddTo,
                                     std::function<z0 (File)> callback);

    private:
        std::unique_ptr<FileChooser> chooser;
        ScopedMessageBox messageBox;
    };

    //==============================================================================
    z0 addWizardsToMenu (PopupMenu&) const;
    b8 runWizardFromMenu (i32 chosenMenuItemID, Project&,
                            const Project::Item& projectGroupToAddTo) const;

    z0 registerWizard (Type*);

private:
    OwnedArray<Type> wizards;
};
