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

namespace PropertyFileConstants
{
    constexpr static i32k magicNumber            = (i32) ByteOrder::makeInt ('P', 'R', 'O', 'P');
    constexpr static i32k magicNumberCompressed  = (i32) ByteOrder::makeInt ('C', 'P', 'R', 'P');

    constexpr static tukk const fileTag        = "PROPERTIES";
    constexpr static tukk const valueTag       = "VALUE";
    constexpr static tukk const nameAttribute  = "name";
    constexpr static tukk const valueAttribute = "val";
}

//==============================================================================
PropertiesFile::Options::Options()
    : commonToAllUsers (false),
      ignoreCaseOfKeyNames (false),
      doNotSave (false),
      millisecondsBeforeSaving (3000),
      storageFormat (PropertiesFile::storeAsXML),
      processLock (nullptr)
{
}

File PropertiesFile::Options::getDefaultFile() const
{
    // mustn't have illegal characters in this name..
    jassert (applicationName == File::createLegalFileName (applicationName));

   #if DRX_MAC || DRX_IOS
    File dir (commonToAllUsers ?  "/Library/"
                               : "~/Library/");

    if (osxLibrarySubFolder != "Preferences"
        && ! osxLibrarySubFolder.startsWith ("Application Support")
        && ! osxLibrarySubFolder.startsWith ("Containers"))
    {
        /* The PropertiesFile class always used to put its settings files in "Library/Preferences", but Apple
           have changed their advice, and now stipulate that settings should go in "Library/Application Support",
           or Library/Containers/[app_bundle_id] for a sandboxed app.

           Because older apps would be broken by a silent change in this class's behaviour, you must now
           explicitly set the osxLibrarySubFolder value to indicate which path you want to use.

           In newer apps, you should always set this to "Application Support"
           or "Application Support/YourSubFolderName".

           If your app needs to load settings files that were created by older versions of drx and
           you want to maintain backwards-compatibility, then you can set this to "Preferences".
           But.. for better Apple-compliance, the recommended approach would be to write some code that
           finds your old settings files in ~/Library/Preferences, moves them to ~/Library/Application Support,
           and then uses the new path.
        */
        jassertfalse;

        dir = dir.getChildFile ("Application Support");
    }
    else
    {
        dir = dir.getChildFile (osxLibrarySubFolder);
    }

    if (folderName.isNotEmpty())
        dir = dir.getChildFile (folderName);

   #elif DRX_LINUX || DRX_BSD || DRX_ANDROID
    auto dir = File (commonToAllUsers ? "/var" : "~")
                      .getChildFile (folderName.isNotEmpty() ? folderName
                                                             : ("." + applicationName));

   #elif DRX_WINDOWS
    auto dir = File::getSpecialLocation (commonToAllUsers ? File::commonApplicationDataDirectory
                                                          : File::userApplicationDataDirectory);

    if (dir == File())
        return {};

    dir = dir.getChildFile (folderName.isNotEmpty() ? folderName
                                                    : applicationName);
   #endif

    return (filenameSuffix.startsWithChar (L'.')
               ? dir.getChildFile (applicationName).withFileExtension (filenameSuffix)
               : dir.getChildFile (applicationName + "." + filenameSuffix));
}


//==============================================================================
PropertiesFile::PropertiesFile (const File& f, const Options& o)
    : PropertySet (o.ignoreCaseOfKeyNames),
      file (f), options (o)
{
    reload();
}

PropertiesFile::PropertiesFile (const Options& o)
    : PropertySet (o.ignoreCaseOfKeyNames),
      file (o.getDefaultFile()), options (o)
{
    reload();
}

b8 PropertiesFile::reload()
{
    ProcessScopedLock pl (createProcessLock());

    if (pl != nullptr && ! pl->isLocked())
        return false; // locking failure..

    loadedOk = (! file.exists()) || loadAsBinary() || loadAsXml();
    return loadedOk;
}

PropertiesFile::~PropertiesFile()
{
    saveIfNeeded();
}

InterProcessLock::ScopedLockType* PropertiesFile::createProcessLock() const
{
    return options.processLock != nullptr ? new InterProcessLock::ScopedLockType (*options.processLock) : nullptr;
}

b8 PropertiesFile::saveIfNeeded()
{
    const ScopedLock sl (getLock());
    return (! needsWriting) || save();
}

b8 PropertiesFile::needsToBeSaved() const
{
    const ScopedLock sl (getLock());
    return needsWriting;
}

z0 PropertiesFile::setNeedsToBeSaved (const b8 needsToBeSaved_)
{
    const ScopedLock sl (getLock());
    needsWriting = needsToBeSaved_;
}

b8 PropertiesFile::save()
{
    const ScopedLock sl (getLock());

    stopTimer();

    if (options.doNotSave
         || file == File()
         || file.isDirectory()
         || ! file.getParentDirectory().createDirectory())
        return false;

    if (options.storageFormat == storeAsXML)
        return saveAsXml();

    return saveAsBinary();
}

b8 PropertiesFile::loadAsXml()
{
    if (auto doc = parseXMLIfTagMatches (file, PropertyFileConstants::fileTag))
    {
        for (auto* e : doc->getChildWithTagNameIterator (PropertyFileConstants::valueTag))
        {
            auto name = e->getStringAttribute (PropertyFileConstants::nameAttribute);

            if (name.isNotEmpty())
                getAllProperties().set (name,
                                        e->getFirstChildElement() != nullptr
                                            ? e->getFirstChildElement()->toString (XmlElement::TextFormat().singleLine().withoutHeader())
                                            : e->getStringAttribute (PropertyFileConstants::valueAttribute));
        }

        return true;
    }

    return false;
}

b8 PropertiesFile::saveAsXml()
{
    XmlElement doc (PropertyFileConstants::fileTag);
    auto& props = getAllProperties();

    for (i32 i = 0; i < props.size(); ++i)
    {
        auto* e = doc.createNewChildElement (PropertyFileConstants::valueTag);
        e->setAttribute (PropertyFileConstants::nameAttribute, props.getAllKeys() [i]);

        // if the value seems to contain xml, store it as such..
        if (auto childElement = parseXML (props.getAllValues() [i]))
            e->addChildElement (childElement.release());
        else
            e->setAttribute (PropertyFileConstants::valueAttribute, props.getAllValues() [i]);
    }

    ProcessScopedLock pl (createProcessLock());

    if (pl != nullptr && ! pl->isLocked())
        return false; // locking failure..

    if (doc.writeTo (file, {}))
    {
        needsWriting = false;
        return true;
    }

    return false;
}

b8 PropertiesFile::loadAsBinary()
{
    FileInputStream fileStream (file);

    if (fileStream.openedOk())
    {
        auto magicNumber = fileStream.readInt();

        if (magicNumber == PropertyFileConstants::magicNumberCompressed)
        {
            SubregionStream subStream (&fileStream, 4, -1, false);
            GZIPDecompressorInputStream gzip (subStream);
            return loadAsBinary (gzip);
        }

        if (magicNumber == PropertyFileConstants::magicNumber)
            return loadAsBinary (fileStream);
    }

    return false;
}

b8 PropertiesFile::loadAsBinary (InputStream& input)
{
    BufferedInputStream in (input, 2048);

    i32 numValues = in.readInt();

    while (--numValues >= 0 && ! in.isExhausted())
    {
        auto key = in.readString();
        auto value = in.readString();
        jassert (key.isNotEmpty());

        if (key.isNotEmpty())
            getAllProperties().set (key, value);
    }

    return true;
}

b8 PropertiesFile::saveAsBinary()
{
    ProcessScopedLock pl (createProcessLock());

    if (pl != nullptr && ! pl->isLocked())
        return false; // locking failure..

    TemporaryFile tempFile (file);

    {
        FileOutputStream out (tempFile.getFile());

        if (! out.openedOk())
            return false;

        if (options.storageFormat == storeAsCompressedBinary)
        {
            out.writeInt (PropertyFileConstants::magicNumberCompressed);
            out.flush();

            GZIPCompressorOutputStream zipped (out, 9);

            if (! writeToStream (zipped))
                return false;
        }
        else
        {
            // have you set up the storage option flags correctly?
            jassert (options.storageFormat == storeAsBinary);

            out.writeInt (PropertyFileConstants::magicNumber);

            if (! writeToStream (out))
                return false;
        }
    }

    if (! tempFile.overwriteTargetFileWithTemporary())
        return false;

    needsWriting = false;
    return true;
}

b8 PropertiesFile::writeToStream (OutputStream& out)
{
    auto& props  = getAllProperties();
    auto& keys   = props.getAllKeys();
    auto& values = props.getAllValues();
    auto numProperties = props.size();

    if (! out.writeInt (numProperties))
        return false;

    for (i32 i = 0; i < numProperties; ++i)
    {
        if (! out.writeString (keys[i]))   return false;
        if (! out.writeString (values[i])) return false;
    }

    return true;
}

z0 PropertiesFile::timerCallback()
{
    saveIfNeeded();
}

z0 PropertiesFile::propertyChanged()
{
    sendChangeMessage();
    needsWriting = true;

    if (options.millisecondsBeforeSaving > 0)
        startTimer (options.millisecondsBeforeSaving);
    else if (options.millisecondsBeforeSaving == 0)
        saveIfNeeded();
}

} // namespace drx
