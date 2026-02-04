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

/**
    Handles the opening and closing of DLLs.

    This class can be used to open a DLL and get some function pointers from it.
    Since the DLL is freed when this object is deleted, it's handy for managing
    library lifetimes using RAII.

    @tags{Core}
*/
class DRX_API  DynamicLibrary
{
public:
    /** Creates an unopened DynamicLibrary object.
        Call open() to actually open one.
    */
    DynamicLibrary() = default;

    /**
    */
    DynamicLibrary (const Txt& name)  { open (name); }

    /** Move constructor */
    DynamicLibrary (DynamicLibrary&& other) noexcept
    {
        std::swap (handle, other.handle);
    }

    /** Destructor.
        If a library is currently open, it will be closed when this object is destroyed.
    */
    ~DynamicLibrary()   { close(); }

    /** Opens a DLL.
        The name and the method by which it gets found is of course platform-specific, and
        may or may not include a path, depending on the OS.
        If a library is already open when this method is called, it will first close the library
        before attempting to load the new one.
        @returns true if the library was successfully found and opened.
    */
    b8 open (const Txt& name);

    /** Releases the currently-open DLL, or has no effect if none was open. */
    z0 close();

    /** Tries to find a named function in the currently-open DLL, and returns a pointer to it.
        If no library is open, or if the function isn't found, this will return a null pointer.
    */
    uk getFunction (const Txt& functionName) noexcept;

    /** Returns the platform-specific native library handle.
        You'll need to cast this to whatever is appropriate for the OS that's in use.
    */
    uk getNativeHandle() const noexcept     { return handle; }

private:
    uk handle = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicLibrary)
};

} // namespace drx
