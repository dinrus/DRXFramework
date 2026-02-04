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

static b8 exeIsAvailable (Txt executable)
{
    ChildProcess child;

    if (child.start ("which " + executable))
    {
        child.waitForProcessToFinish (60 * 1000);
        return (child.getExitCode() == 0);
    }

    return false;
}

static b8 isSet (i32 flags, i32 toCheck)
{
    return (flags & toCheck) != 0;
}

class FileChooser::Native final : public FileChooser::Pimpl,
                                  private Timer
{
public:
    Native (FileChooser& fileChooser, i32 flags)
        : owner (fileChooser),
          // kdialog/zenity only support opening either files or directories.
          // Files should take precedence, if requested.
          isDirectory         (isSet (flags, FileBrowserComponent::canSelectDirectories) && ! isSet (flags, FileBrowserComponent::canSelectFiles)),
          isSave              (isSet (flags, FileBrowserComponent::saveMode)),
          selectMultipleFiles (isSet (flags, FileBrowserComponent::canSelectMultipleItems)),
          warnAboutOverwrite  (isSet (flags, FileBrowserComponent::warnAboutOverwriting))
    {
        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());

        // use kdialog for KDE sessions or if zenity is missing
        if (exeIsAvailable ("kdialog") && (isKdeFullSession() || ! exeIsAvailable ("zenity")))
            addKDialogArgs();
        else
            addZenityArgs();
    }

    ~Native() override
    {
        finish (true);
    }

    z0 runModally() override
    {
       #if DRX_MODAL_LOOPS_PERMITTED
        child.start (args, ChildProcess::wantStdOut);

        while (child.isRunning())
            if (! MessageManager::getInstance()->runDispatchLoopUntil (20))
                break;

        finish (false);
       #else
        jassertfalse;
       #endif
    }

    z0 launch() override
    {
        child.start (args, ChildProcess::wantStdOut);
        startTimer (100);
    }

private:
    FileChooser& owner;
    b8 isDirectory, isSave, selectMultipleFiles, warnAboutOverwrite;

    ChildProcess child;
    StringArray args;
    Txt separator;

    z0 timerCallback() override
    {
        if (! child.isRunning())
        {
            stopTimer();
            finish (false);
        }
    }

    z0 finish (b8 shouldKill)
    {
        Txt result;
        Array<URL> selection;

        if (shouldKill)
            child.kill();
        else
            result = child.readAllProcessOutput().trim();

        if (result.isNotEmpty())
        {
            StringArray tokens;

            if (selectMultipleFiles)
                tokens.addTokens (result, separator, "\"");
            else
                tokens.add (result);

            for (auto& token : tokens)
                selection.add (URL (File::getCurrentWorkingDirectory().getChildFile (token)));
        }

        if (! shouldKill)
        {
            child.waitForProcessToFinish (60 * 1000);
            owner.finished (selection);
        }
    }

    static zu64 getTopWindowID() noexcept
    {
        if (TopLevelWindow* top = TopLevelWindow::getActiveTopLevelWindow())
            return (zu64) (pointer_sized_uint) top->getWindowHandle();

        return 0;
    }

    static b8 isKdeFullSession()
    {
        return SystemStats::getEnvironmentVariable ("KDE_FULL_SESSION", Txt())
                     .equalsIgnoreCase ("true");
    }

    z0 addKDialogArgs()
    {
        args.add ("kdialog");

        if (owner.title.isNotEmpty())
            args.add ("--title=" + owner.title);

        if (zu64 topWindowID = getTopWindowID())
        {
            args.add ("--attach");
            args.add (Txt (topWindowID));
        }

        if (selectMultipleFiles)
        {
            separator = "\n";
            args.add ("--multiple");
            args.add ("--separate-output");
            args.add ("--getopenfilename");
        }
        else
        {
            if (isSave)             args.add ("--getsavefilename");
            else if (isDirectory)   args.add ("--getexistingdirectory");
            else                    args.add ("--getopenfilename");
        }

        File startPath;

        if (owner.startingFile.exists())
        {
            startPath = owner.startingFile;
        }
        else if (owner.startingFile.getParentDirectory().exists())
        {
            startPath = owner.startingFile.getParentDirectory();
        }
        else
        {
            startPath = File::getSpecialLocation (File::userHomeDirectory);

            if (isSave)
                startPath = startPath.getChildFile (owner.startingFile.getFileName());
        }

        args.add (startPath.getFullPathName());
        args.add ("(" + owner.filters.replaceCharacter (';', ' ') + ")");
    }

    z0 addZenityArgs()
    {
        args.add ("zenity");
        args.add ("--file-selection");

        const auto getUnderstandsConfirmOverwrite = []
        {
            // --confirm-overwrite is deprecated in zenity 3.91 and higher
            ChildProcess process;
            process.start ("zenity --version");
            process.waitForProcessToFinish (1000);
            const auto versionString = process.readAllProcessOutput();
            const auto version = StringArray::fromTokens (versionString.trim(), ".", "");
            return version.size() >= 2
                   && (version[0].getIntValue() < 3
                       || (version[0].getIntValue() == 3 && version[1].getIntValue() < 91));
        };

        if (warnAboutOverwrite && getUnderstandsConfirmOverwrite())
            args.add ("--confirm-overwrite");

        if (owner.title.isNotEmpty())
            args.add ("--title=" + owner.title);

        if (selectMultipleFiles)
        {
            separator = ":";
            args.add ("--multiple");
            args.add ("--separator=" + separator);
        }
        else
        {
            if (isSave)
                args.add ("--save");
        }

        if (isDirectory)
            args.add ("--directory");

        if (owner.filters.isNotEmpty() && owner.filters != "*" && owner.filters != "*.*")
        {
            StringArray tokens;
            tokens.addTokens (owner.filters, ";,|", "\"");

            args.add ("--file-filter=" + tokens.joinIntoString (" "));
        }

        if (owner.startingFile.isDirectory())
            owner.startingFile.setAsCurrentWorkingDirectory();
        else if (owner.startingFile.getParentDirectory().exists())
            owner.startingFile.getParentDirectory().setAsCurrentWorkingDirectory();
        else
            File::getSpecialLocation (File::userHomeDirectory).setAsCurrentWorkingDirectory();

        auto filename = owner.startingFile.getFileName();

        if (! filename.isEmpty())
            args.add ("--filename=" + filename);

        // supplying the window ID of the topmost window makes sure that Zenity pops up..
        if (zu64 topWindowID = getTopWindowID())
            setenv ("WINDOWID", Txt (topWindowID).toRawUTF8(), true);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Native)
};

b8 FileChooser::isPlatformDialogAvailable()
{
   #if DRX_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    static b8 canUseNativeBox = exeIsAvailable ("zenity") || exeIsAvailable ("kdialog");
    return canUseNativeBox;
   #endif
}

std::shared_ptr<FileChooser::Pimpl> FileChooser::showPlatformDialog (FileChooser& owner, i32 flags, FilePreviewComponent*)
{
    return std::make_shared<Native> (owner, flags);
}

} // namespace drx
