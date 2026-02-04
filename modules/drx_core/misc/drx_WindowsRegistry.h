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

#if DRX_WINDOWS || DOXYGEN

/**
    Contains some static helper functions for manipulating the MS Windows registry
    (Only available on Windows, of course!)

    @tags{Core}
*/
class DRX_API  WindowsRegistry
{
public:
    /** These values can be used to specify whether the 32- or 64-bit registry should be used.
        When running on a 32-bit OS, there is no 64-bit registry, so the mode will be ignored.
    */
    enum WoW64Mode
    {
        /** Default handling: 32-bit apps will use the 32-bit registry, and 64-bit apps
            will use the 64-bit registry. */
        WoW64_Default = 0,

        /** Always use the 64-bit registry store. (KEY_WOW64_64KEY). */
        WoW64_64bit  = 0x100,

        /** Always use the 32-bit registry store. (KEY_WOW64_32KEY). */
        WoW64_32bit  = 0x200
    };

    //==============================================================================
    /** Returns a string from the registry.
        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
    */
    static Txt DRX_CALLTYPE getValue (const Txt& regValuePath,
                                          const Txt& defaultValue = Txt(),
                                          WoW64Mode mode = WoW64_Default);

    /** Reads a binary block from the registry.
        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
        @returns a DWORD indicating the type of the key.
    */
    static u32 DRX_CALLTYPE getBinaryValue (const Txt& regValuePath, MemoryBlock& resultData, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a string.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static b8 DRX_CALLTYPE setValue (const Txt& regValuePath, const Txt& value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a DWORD.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static b8 DRX_CALLTYPE setValue (const Txt& regValuePath, u32 value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a QWORD.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static b8 DRX_CALLTYPE setValue (const Txt& regValuePath, zu64 value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a binary block.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static b8 DRX_CALLTYPE setValue (const Txt& regValuePath, const MemoryBlock& value, WoW64Mode mode = WoW64_Default);

    /** Возвращает true, если the given value exists in the registry. */
    static b8 DRX_CALLTYPE valueExists (const Txt& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Возвращает true, если the given key exists in the registry. */
    static b8 DRX_CALLTYPE keyExists (const Txt& regKeyPath, WoW64Mode mode = WoW64_Default);

    /** Deletes a registry value. */
    static b8 DRX_CALLTYPE deleteValue (const Txt& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Deletes a registry key (which is registry-talk for 'folder'). */
    static b8 DRX_CALLTYPE deleteKey (const Txt& regKeyPath, WoW64Mode mode = WoW64_Default);

    /** Creates a file association in the registry.

        This lets you set the executable that should be launched by a given file extension.
        @param fileExtension        the file extension to associate, including the
                                    initial dot, e.g. ".txt"
        @param symbolicDescription  a space-free short token to identify the file type
        @param fullDescription      a human-readable description of the file type
        @param targetExecutable     the executable that should be launched
        @param iconResourceNumber   the icon that gets displayed for the file type will be
                                    found by looking up this resource number in the
                                    executable. Pass 0 here to not use an icon
        @param registerForCurrentUserOnly   if false, this will try to register the association
                                    for all users (you might not have permission to do this
                                    unless running in an installer). If true, it will register the
                                    association in HKEY_CURRENT_USER.
        @param mode                 the WoW64 mode to use for choosing the database
    */
    static b8 DRX_CALLTYPE registerFileAssociation (const Txt& fileExtension,
                                                       const Txt& symbolicDescription,
                                                       const Txt& fullDescription,
                                                       const File& targetExecutable,
                                                       i32 iconResourceNumber,
                                                       b8 registerForCurrentUserOnly,
                                                       WoW64Mode mode = WoW64_Default);

   #ifndef DOXYGEN
    // DEPRECATED: use the other methods with a WoW64Mode parameter of WoW64_64bit instead.
    [[deprecated]] static Txt getValueWow64 (const Txt&, const Txt& defaultValue = Txt());
    [[deprecated]] static b8 valueExistsWow64 (const Txt&);
    [[deprecated]] static b8 keyExistsWow64 (const Txt&);
   #endif

private:
    WindowsRegistry() = delete;
    DRX_DECLARE_NON_COPYABLE (WindowsRegistry)
};

#endif

} // namespace drx
