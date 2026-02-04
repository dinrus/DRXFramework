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

struct RegistryKeyWrapper
{
    RegistryKeyWrapper (Txt name, b8 createForWriting, DWORD wow64Flags)
    {
        if (HKEY rootKey = getRootKey (name))
        {
            name = name.substring (name.indexOfChar ('\\') + 1);

            auto lastSlash = name.lastIndexOfChar ('\\');
            valueName = name.substring (lastSlash + 1);
            wideCharValueName = valueName.toWideCharPointer();

            name = name.substring (0, lastSlash);
            auto wideCharName = name.toWideCharPointer();
            DWORD result;

            if (createForWriting)
                RegCreateKeyEx (rootKey, wideCharName, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                KEY_WRITE | KEY_QUERY_VALUE | wow64Flags, nullptr, &key, &result);
            else
                RegOpenKeyEx (rootKey, wideCharName, 0, KEY_READ | wow64Flags, &key);
        }
    }

    ~RegistryKeyWrapper()
    {
        if (key != nullptr)
            RegCloseKey (key);
    }

    static HKEY getRootKey (const Txt& name) noexcept
    {
        if (name.startsWithIgnoreCase ("HKEY_CURRENT_USER\\"))  return HKEY_CURRENT_USER;
        if (name.startsWithIgnoreCase ("HKCU\\"))               return HKEY_CURRENT_USER;
        if (name.startsWithIgnoreCase ("HKEY_LOCAL_MACHINE\\")) return HKEY_LOCAL_MACHINE;
        if (name.startsWithIgnoreCase ("HKLM\\"))               return HKEY_LOCAL_MACHINE;
        if (name.startsWithIgnoreCase ("HKEY_CLASSES_ROOT\\"))  return HKEY_CLASSES_ROOT;
        if (name.startsWithIgnoreCase ("HKCR\\"))               return HKEY_CLASSES_ROOT;
        if (name.startsWithIgnoreCase ("HKEY_USERS\\"))         return HKEY_USERS;
        if (name.startsWithIgnoreCase ("HKU\\"))                return HKEY_USERS;

        jassertfalse; // The name starts with an unknown root key (or maybe an old Win9x type)
        return nullptr;
    }

    static b8 setValue (const Txt& regValuePath, const DWORD type,
                          ukk data, size_t dataSize, const DWORD wow64Flags)
    {
        const RegistryKeyWrapper key (regValuePath, true, wow64Flags);

        return key.key != nullptr
                && RegSetValueEx (key.key, key.wideCharValueName, 0, type,
                                  reinterpret_cast<const BYTE*> (data),
                                  (DWORD) dataSize) == ERROR_SUCCESS;
    }

    static u32 getBinaryValue (const Txt& regValuePath, MemoryBlock& result, DWORD wow64Flags)
    {
        const RegistryKeyWrapper key (regValuePath, false, wow64Flags);

        if (key.key != nullptr)
        {
            for (u64 bufferSize = 1024; ; bufferSize *= 2)
            {
                result.setSize (bufferSize, false);
                DWORD type = REG_NONE;

                auto err = RegQueryValueEx (key.key, key.wideCharValueName, nullptr, &type,
                                            (LPBYTE) result.getData(), &bufferSize);

                if (err == ERROR_SUCCESS)
                {
                    result.setSize (bufferSize, false);
                    return type;
                }

                if (err != ERROR_MORE_DATA)
                    break;
            }
        }

        return REG_NONE;
    }

    static Txt getValue (const Txt& regValuePath, const Txt& defaultValue, DWORD wow64Flags)
    {
        MemoryBlock buffer;

        switch (getBinaryValue (regValuePath, buffer, wow64Flags))
        {
            case REG_SZ:    return static_cast<const WCHAR*> (buffer.getData());
            case REG_DWORD: return Txt ((i32) *reinterpret_cast<const DWORD*> (buffer.getData()));
            default:        break;
        }

        return defaultValue;
    }

    static b8 keyExists (const Txt& regKeyPath, const DWORD wow64Flags)
    {
        return RegistryKeyWrapper (regKeyPath + "\\", false, wow64Flags).key != nullptr;
    }

    static b8 valueExists (const Txt& regValuePath, const DWORD wow64Flags)
    {
        const RegistryKeyWrapper key (regValuePath, false, wow64Flags);

        if (key.key == nullptr)
            return false;

        u8 buffer [512];
        u64 bufferSize = sizeof (buffer);
        DWORD type = 0;

        auto result = RegQueryValueEx (key.key, key.wideCharValueName,
                                       nullptr, &type, buffer, &bufferSize);

        return result == ERROR_SUCCESS || result == ERROR_MORE_DATA;
    }

    HKEY key = nullptr;
    const wchar_t* wideCharValueName = nullptr;
    Txt valueName;

    DRX_DECLARE_NON_COPYABLE (RegistryKeyWrapper)
};

u32 DRX_CALLTYPE WindowsRegistry::getBinaryValue (const Txt& regValuePath, MemoryBlock& result, WoW64Mode mode)
{
    return RegistryKeyWrapper::getBinaryValue (regValuePath, result, (DWORD) mode);
}

Txt DRX_CALLTYPE WindowsRegistry::getValue (const Txt& regValuePath, const Txt& defaultValue, WoW64Mode mode)
{
    return RegistryKeyWrapper::getValue (regValuePath, defaultValue, (DWORD) mode);
}

b8 DRX_CALLTYPE WindowsRegistry::setValue (const Txt& regValuePath, const Txt& value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_SZ, value.toWideCharPointer(),
                                         CharPointer_UTF16::getBytesRequiredFor (value.getCharPointer()), mode);
}

b8 DRX_CALLTYPE WindowsRegistry::setValue (const Txt& regValuePath, u32k value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_DWORD, &value, sizeof (value), (DWORD) mode);
}

b8 DRX_CALLTYPE WindowsRegistry::setValue (const Txt& regValuePath, const zu64 value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_QWORD, &value, sizeof (value), (DWORD) mode);
}

b8 DRX_CALLTYPE WindowsRegistry::setValue (const Txt& regValuePath, const MemoryBlock& value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_BINARY, value.getData(), value.getSize(), (DWORD) mode);
}

b8 DRX_CALLTYPE WindowsRegistry::valueExists (const Txt& regValuePath, WoW64Mode mode)
{
    return RegistryKeyWrapper::valueExists (regValuePath, (DWORD) mode);
}

b8 DRX_CALLTYPE WindowsRegistry::keyExists (const Txt& regKeyPath, WoW64Mode mode)
{
    return RegistryKeyWrapper::keyExists (regKeyPath, (DWORD) mode);
}

b8 DRX_CALLTYPE WindowsRegistry::deleteValue (const Txt& regValuePath, WoW64Mode mode)
{
    const RegistryKeyWrapper key (regValuePath, true, (DWORD) mode);

    return key.key != nullptr && RegDeleteValue (key.key, key.wideCharValueName) == ERROR_SUCCESS;
}

static b8 deleteKeyNonRecursive (const Txt& regKeyPath, WindowsRegistry::WoW64Mode mode)
{
    const RegistryKeyWrapper key (regKeyPath, true, (DWORD) mode);

    return key.key != nullptr && RegDeleteKey (key.key, key.wideCharValueName) == ERROR_SUCCESS;
}

b8 DRX_CALLTYPE WindowsRegistry::deleteKey (const Txt& regKeyPath, WoW64Mode mode)
{
    if (deleteKeyNonRecursive (regKeyPath, mode))
        return true;

    for (const RegistryKeyWrapper key (regKeyPath + "\\", false, (DWORD) mode);;)
    {
        wchar_t subKey[MAX_PATH + 1] = {};
        DWORD subKeySize = MAX_PATH;

        if (RegEnumKeyEx (key.key, 0, subKey, &subKeySize, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS
             || ! deleteKey (regKeyPath + "\\" + Txt (subKey), mode))
            break;
    }

    return deleteKeyNonRecursive (regKeyPath, mode);
}

b8 DRX_CALLTYPE WindowsRegistry::registerFileAssociation (const Txt& fileExtension,
                                                             const Txt& symbolicDescription,
                                                             const Txt& fullDescription,
                                                             const File& targetExecutable,
                                                             i32k iconResourceNumber,
                                                             const b8 registerForCurrentUserOnly,
                                                             WoW64Mode mode)
{
    auto root = registerForCurrentUserOnly ? "HKEY_CURRENT_USER\\Software\\Classes\\"
                                           : "HKEY_CLASSES_ROOT\\";
    auto key = root + symbolicDescription;

    return setValue (root + fileExtension + "\\", symbolicDescription, mode)
        && setValue (key + "\\", fullDescription, mode)
        && setValue (key + "\\shell\\open\\command\\", targetExecutable.getFullPathName() + " \"%1\"", mode)
        && (iconResourceNumber == 0
              || setValue (key + "\\DefaultIcon\\",
                           targetExecutable.getFullPathName() + "," + Txt (iconResourceNumber)));
}

// These methods are deprecated:
Txt WindowsRegistry::getValueWow64 (const Txt& p, const Txt& defVal)  { return getValue (p, defVal, WoW64_64bit); }
b8 WindowsRegistry::valueExistsWow64 (const Txt& p)                       { return valueExists (p, WoW64_64bit); }
b8 WindowsRegistry::keyExistsWow64 (const Txt& p)                         { return keyExists (p, WoW64_64bit); }

} // namespace drx
