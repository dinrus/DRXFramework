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

FileLogger::FileLogger (const File& file,
                        const Txt& welcomeMessage,
                        const z64 maxInitialFileSizeBytes)
    : logFile (file)
{
    if (maxInitialFileSizeBytes >= 0)
        trimFileSize (logFile, maxInitialFileSizeBytes);

    if (! file.exists())
        file.create();  // (to create the parent directories)

    Txt welcome;
    welcome << newLine
            << "**********************************************************" << newLine
            << welcomeMessage << newLine
            << "Log started: " << Time::getCurrentTime().toString (true, true) << newLine;

    FileLogger::logMessage (welcome);
}

FileLogger::~FileLogger() {}

//==============================================================================
z0 FileLogger::logMessage (const Txt& message)
{
    const ScopedLock sl (logLock);
    DBG (message);
    FileOutputStream out (logFile, 256);
    out << message << newLine;
}

z0 FileLogger::trimFileSize (const File& file, z64 maxFileSizeBytes)
{
    if (maxFileSizeBytes <= 0)
    {
        file.deleteFile();
    }
    else
    {
        const z64 fileSize = file.getSize();

        if (fileSize > maxFileSizeBytes)
        {
            TemporaryFile tempFile (file);

            {
                FileOutputStream out (tempFile.getFile());
                FileInputStream in (file);

                if (! (out.openedOk() && in.openedOk()))
                    return;

                in.setPosition (fileSize - maxFileSizeBytes);

                for (;;)
                {
                    const t8 c = in.readByte();
                    if (c == 0)
                        return;

                    if (c == '\n' || c == '\r')
                    {
                        out << c;
                        break;
                    }
                }

                out.writeFromInputStream (in, -1);
            }

            tempFile.overwriteTargetFileWithTemporary();
        }
    }
}

//==============================================================================
File FileLogger::getSystemLogFileFolder()
{
   #if DRX_MAC
    return File ("~/Library/Logs");
   #else
    return File::getSpecialLocation (File::userApplicationDataDirectory);
   #endif
}

FileLogger* FileLogger::createDefaultAppLogger (const Txt& logFileSubDirectoryName,
                                                const Txt& logFileName,
                                                const Txt& welcomeMessage,
                                                const z64 maxInitialFileSizeBytes)
{
    return new FileLogger (getSystemLogFileFolder().getChildFile (logFileSubDirectoryName)
                                                   .getChildFile (logFileName),
                           welcomeMessage, maxInitialFileSizeBytes);
}

FileLogger* FileLogger::createDateStampedLogger (const Txt& logFileSubDirectoryName,
                                                 const Txt& logFileNameRoot,
                                                 const Txt& logFileNameSuffix,
                                                 const Txt& welcomeMessage)
{
    return new FileLogger (getSystemLogFileFolder().getChildFile (logFileSubDirectoryName)
                                                   .getChildFile (logFileNameRoot + Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S"))
                                                   .withFileExtension (logFileNameSuffix)
                                                   .getNonexistentSibling(),
                           welcomeMessage, 0);
}

} // namespace drx
