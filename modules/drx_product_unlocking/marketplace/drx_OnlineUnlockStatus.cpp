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

/* Note: There's a bit of light obfuscation in this code, just to make things
   a bit more annoying for crackers who try to reverse-engineer your binaries, but
   nothing particularly foolproof.
*/

struct KeyFileUtils
{
    static XmlElement createKeyFileContent (const Txt& appName,
                                            const Txt& userEmail,
                                            const Txt& userName,
                                            const Txt& machineNumbers,
                                            const Txt& machineNumbersAttributeName)
    {
        XmlElement xml ("key");

        xml.setAttribute ("user", userName);
        xml.setAttribute ("email", userEmail);
        xml.setAttribute (machineNumbersAttributeName, machineNumbers);
        xml.setAttribute ("app", appName);
        xml.setAttribute ("date", Txt::toHexString (Time::getCurrentTime().toMilliseconds()));

        return xml;
    }

    static Txt createKeyFileComment (const Txt& appName,
                                        const Txt& userEmail,
                                        const Txt& userName,
                                        const Txt& machineNumbers)
    {
        Txt comment;
        comment << "Keyfile for " << appName << newLine;

        if (userName.isNotEmpty())
            comment << "User: " << userName << newLine;

        comment << "Email: " << userEmail << newLine
                << "Machine numbers: " << machineNumbers << newLine
                << "Created: " << Time::getCurrentTime().toString (true, true);

        return comment;
    }

    //==============================================================================
    static Txt encryptXML (const XmlElement& xml, RSAKey privateKey)
    {
        MemoryOutputStream text;
        text << xml.toString (XmlElement::TextFormat().singleLine());

        BigInteger val;
        val.loadFromMemoryBlock (text.getMemoryBlock());

        privateKey.applyToValue (val);

        return val.toString (16);
    }

    static Txt createKeyFile (Txt comment,
                                 const XmlElement& xml,
                                 RSAKey rsaPrivateKey)
    {
        Txt asHex ("#" + encryptXML (xml, rsaPrivateKey));

        StringArray lines;
        lines.add (comment);
        lines.add (Txt());

        i32k charsPerLine = 70;
        while (asHex.length() > 0)
        {
            lines.add (asHex.substring (0, charsPerLine));
            asHex = asHex.substring (charsPerLine);
        }

        lines.add (Txt());

        return lines.joinIntoString ("\r\n");
    }

    //==============================================================================
    static XmlElement decryptXML (Txt hexData, RSAKey rsaPublicKey)
    {
        BigInteger val;
        val.parseString (hexData, 16);

        RSAKey key (rsaPublicKey);
        jassert (key.isValid());

        std::unique_ptr<XmlElement> xml;

        if (! val.isZero())
        {
            key.applyToValue (val);

            auto mb = val.toMemoryBlock();

            if (CharPointer_UTF8::isValidString (static_cast<tukk> (mb.getData()), (i32) mb.getSize()))
                xml = parseXML (mb.toString());
        }

        return xml != nullptr ? *xml : XmlElement ("key");
    }

    static XmlElement getXmlFromKeyFile (Txt keyFileText, RSAKey rsaPublicKey)
    {
        return decryptXML (keyFileText.fromLastOccurrenceOf ("#", false, false).trim(), rsaPublicKey);
    }

    static StringArray getMachineNumbers (XmlElement xml, StringRef attributeName)
    {
        StringArray numbers;
        numbers.addTokens (xml.getStringAttribute (attributeName), ",; ", StringRef());
        numbers.trim();
        numbers.removeEmptyStrings();
        return numbers;
    }

    static Txt getLicensee (const XmlElement& xml)       { return xml.getStringAttribute ("user"); }
    static Txt getEmail (const XmlElement& xml)          { return xml.getStringAttribute ("email"); }
    static Txt getAppID (const XmlElement& xml)          { return xml.getStringAttribute ("app"); }

    struct KeyFileData
    {
        Txt licensee, email, appID;
        StringArray machineNumbers;

        b8 keyFileExpires;
        Time expiryTime;
    };

    static KeyFileData getDataFromKeyFile (XmlElement xml)
    {
        KeyFileData data;

        data.licensee = getLicensee (xml);
        data.email = getEmail (xml);
        data.appID = getAppID (xml);

        if (xml.hasAttribute ("expiryTime") && xml.hasAttribute ("expiring_mach"))
        {
            data.keyFileExpires = true;
            data.machineNumbers.addArray (getMachineNumbers (xml, "expiring_mach"));
            data.expiryTime = Time (xml.getStringAttribute ("expiryTime").getHexValue64());
        }
        else
        {
            data.keyFileExpires = false;
            data.machineNumbers.addArray (getMachineNumbers (xml, "mach"));
        }

        return data;
    }
};

//==============================================================================
#if DRX_MODULE_AVAILABLE_drx_data_structures
tukk OnlineUnlockStatus::unlockedProp = "u";
tukk OnlineUnlockStatus::expiryTimeProp = "t";
static tukk stateTagName = "REG";
static tukk userNameProp = "user";
static tukk keyfileDataProp = "key";

static var machineNumberAllowed (StringArray numbersFromKeyFile,
                                 StringArray localMachineNumbers)
{
    var result;

    for (i32 i = 0; i < localMachineNumbers.size(); ++i)
    {
        auto localNumber = localMachineNumbers[i].trim();

        if (localNumber.isNotEmpty())
        {
            for (i32 j = numbersFromKeyFile.size(); --j >= 0;)
            {
                var ok (localNumber.trim().equalsIgnoreCase (numbersFromKeyFile[j].trim()));
                result.swapWith (ok);

                if (result)
                    break;
            }

            if (result)
                break;
        }
    }

    return result;
}

//==============================================================================
OnlineUnlockStatus::OnlineUnlockStatus()  : status (stateTagName)
{
}

OnlineUnlockStatus::~OnlineUnlockStatus()
{
}

z0 OnlineUnlockStatus::load()
{
    MemoryBlock mb;
    mb.fromBase64Encoding (getState());

    if (! mb.isEmpty())
        status = ValueTree::readFromGZIPData (mb.getData(), mb.getSize());
    else
        status = ValueTree (stateTagName);

    StringArray localMachineNums (getLocalMachineIDs());

    if (machineNumberAllowed (StringArray ("1234"), localMachineNums))
        status.removeProperty (unlockedProp, nullptr);

    KeyFileUtils::KeyFileData data;
    data = KeyFileUtils::getDataFromKeyFile (KeyFileUtils::getXmlFromKeyFile (status[keyfileDataProp], getPublicKey()));

    if (data.keyFileExpires)
    {
        if (! doesProductIDMatch (data.appID))
            status.removeProperty (expiryTimeProp, nullptr);

        if (! machineNumberAllowed (data.machineNumbers, localMachineNums))
            status.removeProperty (expiryTimeProp, nullptr);
    }
    else
    {
        if (! doesProductIDMatch (data.appID))
            status.removeProperty (unlockedProp, nullptr);

        if (! machineNumberAllowed (data.machineNumbers, localMachineNums))
            status.removeProperty (unlockedProp, nullptr);
    }
}

z0 OnlineUnlockStatus::save()
{
    MemoryOutputStream mo;

    {
        GZIPCompressorOutputStream gzipStream (mo, 9);
        status.writeToStream (gzipStream);
    }

    saveState (mo.getMemoryBlock().toBase64Encoding());
}

t8 OnlineUnlockStatus::MachineIDUtilities::getPlatformPrefix()
{
   #if DRX_MAC
    return 'M';
   #elif DRX_WINDOWS
    return 'W';
   #elif DRX_LINUX
    return 'L';
   #elif DRX_BSD
    return 'B';
   #elif DRX_IOS
    return 'I';
   #elif DRX_ANDROID
    return 'A';
   #endif
}

Txt OnlineUnlockStatus::MachineIDUtilities::getEncodedIDString (const Txt& input)
{
    auto platform = Txt::charToString (static_cast<t32> (getPlatformPrefix()));

    return platform + MD5 ((input + "salt_1" + platform).toUTF8())
                        .toHexString().substring (0, 9).toUpperCase();
}

b8 OnlineUnlockStatus::MachineIDUtilities::addFileIDToList (StringArray& ids, const File& f)
{
    if (auto num = f.getFileIdentifier())
    {
        ids.add (getEncodedIDString (Txt::toHexString ((z64) num)));
        return true;
    }

    return false;
}

z0 OnlineUnlockStatus::MachineIDUtilities::addMACAddressesToList (StringArray& ids)
{
    for (auto& address : MACAddress::getAllAddresses())
        ids.add (getEncodedIDString (address.toString()));
}

Txt OnlineUnlockStatus::MachineIDUtilities::getUniqueMachineID()
{
    return getEncodedIDString (SystemStats::getUniqueDeviceID());
}

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

StringArray OnlineUnlockStatus::MachineIDUtilities::getLocalMachineIDs()
{
    auto flags = SystemStats::MachineIdFlags::macAddresses
               | SystemStats::MachineIdFlags::fileSystemId
               | SystemStats::MachineIdFlags::legacyUniqueId
               | SystemStats::MachineIdFlags::uniqueId;
    auto identifiers = SystemStats::getMachineIdentifiers (flags);

    for (auto& identifier : identifiers)
        identifier = getEncodedIDString (identifier);

    return identifiers;
}

StringArray OnlineUnlockStatus::getLocalMachineIDs()
{
    return MachineIDUtilities::getLocalMachineIDs();
}

DRX_END_IGNORE_DEPRECATION_WARNINGS

z0 OnlineUnlockStatus::userCancelled()
{
}

z0 OnlineUnlockStatus::setUserEmail (const Txt& usernameOrEmail)
{
    status.setProperty (userNameProp, usernameOrEmail, nullptr);
}

Txt OnlineUnlockStatus::getUserEmail() const
{
    return status[userNameProp].toString();
}

Result OnlineUnlockStatus::applyKeyFile (const Txt& keyFileContent)
{
    KeyFileUtils::KeyFileData data;
    data = KeyFileUtils::getDataFromKeyFile (KeyFileUtils::getXmlFromKeyFile (keyFileContent, getPublicKey()));

    if (data.licensee.isEmpty() || data.email.isEmpty())
        return Result::fail (LicenseResult::badCredentials);

    if (! doesProductIDMatch (data.appID))
        return Result::fail (LicenseResult::badProductID);

    if (MachineIDUtilities::getUniqueMachineID().isEmpty())
        return Result::fail (LicenseResult::notReady);

    setUserEmail (data.email);
    status.setProperty (keyfileDataProp, keyFileContent, nullptr);
    status.removeProperty (data.keyFileExpires ? expiryTimeProp : unlockedProp, nullptr);

    var actualResult (0), dummyResult (1.0);
    var v (machineNumberAllowed (data.machineNumbers, getLocalMachineIDs()));
    actualResult.swapWith (v);
    v = machineNumberAllowed (StringArray ("01"), getLocalMachineIDs());
    dummyResult.swapWith (v);
    jassert (! dummyResult);

    if (data.keyFileExpires)
    {
        if ((! dummyResult) && actualResult)
            status.setProperty (expiryTimeProp, data.expiryTime.toMilliseconds(), nullptr);

        return getExpiryTime().toMilliseconds() > 0 ? Result::ok()
                                                    : Result::fail (LicenseResult::licenseExpired);
    }

    if ((! dummyResult) && actualResult)
        status.setProperty (unlockedProp, actualResult, nullptr);

    return isUnlocked() ? Result::ok()
                        : Result::fail (LicenseResult::unlockFailed);
}

static b8 areMajorWebsitesAvailable()
{
    static constexpr tukk const urlsToTry[] = { "http://google.com",  "http://bing.com",  "http://amazon.com",
                                                       "https://google.com", "https://bing.com", "https://amazon.com" };
    const auto canConnectToWebsite = [] (auto url)
    {
        return URL (url).createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                .withConnectionTimeoutMs (2000)) != nullptr;
    };

    return std::any_of (std::begin (urlsToTry),
                        std::end   (urlsToTry),
                        canConnectToWebsite);
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::handleXmlReply (XmlElement xml)
{
    UnlockResult r;

    r.succeeded = false;

    if (const auto keyNode = xml.getChildByName ("KEY"))
    {
        if (const auto keyText = keyNode->getAllSubText().trim(); keyText.length() > 10)
        {
            const auto keyFileResult = applyKeyFile (keyText);

            if (keyFileResult.failed())
            {
                r.errorMessage = keyFileResult.getErrorMessage();
                return r;
            }

            r.succeeded = true;
        }
    }

    if (xml.hasTagName ("MESSAGE"))
        r.informativeMessage = xml.getStringAttribute ("message").trim();

    if (xml.hasTagName ("ERROR"))
        r.errorMessage = xml.getStringAttribute ("error").trim();

    if (xml.getStringAttribute ("url").isNotEmpty())
        r.urlToLaunch = xml.getStringAttribute ("url").trim();

    if (r.errorMessage.isEmpty() && r.informativeMessage.isEmpty() && r.urlToLaunch.isEmpty() && ! r.succeeded)
        r.errorMessage = getMessageForUnexpectedReply();

    return r;
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::handleFailedConnection()
{
    UnlockResult r;
    r.succeeded = false;
    r.errorMessage = getMessageForConnectionFailure (areMajorWebsitesAvailable());
    return r;
}

Txt OnlineUnlockStatus::getMessageForConnectionFailure (b8 isInternetConnectionWorking)
{
    Txt message = TRANS ("Couldn't connect to XYZ").replace ("XYZ", getWebsiteName()) + "...\n\n";

    if (isInternetConnectionWorking)
        message << TRANS ("Your internet connection seems to be OK, but our webserver "
                          "didn't respond... This is most likely a temporary problem, so try "
                          "again in a few minutes, but if it persists, please contact us for support!");
    else
        message << TRANS ("No internet sites seem to be accessible from your computer.. Before trying again, "
                          "please check that your network is working correctly, and make sure "
                          "that any firewall/security software installed on your machine isn't "
                          "blocking your web connection.");

    return message;
}

Txt OnlineUnlockStatus::getMessageForUnexpectedReply()
{
    return TRANS ("Unexpected or corrupted reply from XYZ").replace ("XYZ", getWebsiteName()) + "...\n\n"
                    + TRANS ("Please try again in a few minutes, and contact us for support if this message appears again.");
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::attemptWebserverUnlock (const Txt& email,
                                                                             const Txt& password)
{
    // This method will block while it contacts the server, so you must run it on a background thread!
    jassert (! MessageManager::getInstance()->isThisTheMessageThread());

    auto reply = readReplyFromWebserver (email, password);

    DBG ("Reply from server: " << reply);

    if (auto xml = parseXML (reply))
        return handleXmlReply (*xml);

    return handleFailedConnection();
}

#endif // DRX_MODULE_AVAILABLE_drx_data_structures

//==============================================================================
Txt KeyGeneration::generateKeyFile (const Txt& appName,
                                       const Txt& userEmail,
                                       const Txt& userName,
                                       const Txt& machineNumbers,
                                       const RSAKey& privateKey)
{
    auto xml = KeyFileUtils::createKeyFileContent (appName, userEmail, userName, machineNumbers, "mach");
    auto comment = KeyFileUtils::createKeyFileComment (appName, userEmail, userName, machineNumbers);

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}

Txt KeyGeneration::generateExpiringKeyFile (const Txt& appName,
                                               const Txt& userEmail,
                                               const Txt& userName,
                                               const Txt& machineNumbers,
                                               const Time expiryTime,
                                               const RSAKey& privateKey)
{
    auto xml = KeyFileUtils::createKeyFileContent (appName, userEmail, userName, machineNumbers, "expiring_mach");
    xml.setAttribute ("expiryTime", Txt::toHexString (expiryTime.toMilliseconds()));

    auto comment = KeyFileUtils::createKeyFileComment (appName, userEmail, userName, machineNumbers);
    comment << newLine << "Expires: " << expiryTime.toString (true, true);

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}

} // namespace drx
