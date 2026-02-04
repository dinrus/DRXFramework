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
/** Represents the current executable's process.

    This contains methods for controlling the current application at the
    process-level.

    @see Thread, DRXApplicationBase

    @tags{Core}
*/
class DRX_API  Process
{
public:
    //==============================================================================
    enum ProcessPriority
    {
        LowPriority         = 0,
        NormalPriority      = 1,
        HighPriority        = 2,
        RealtimePriority    = 3
    };

    /** Changes the current process's priority.

        @param priority     the process priority, where
                            0=low, 1=normal, 2=high, 3=realtime
    */
    static z0 DRX_CALLTYPE setPriority (ProcessPriority priority);

    /** Kills the current process immediately.

        This is an emergency process terminator that kills the application
        immediately - it's intended only for use only when something goes
        horribly wrong.

        @see DRXApplicationBase::quit
    */
    static z0 DRX_CALLTYPE terminate();

    //==============================================================================
    /** Возвращает true, если this application process is the one that the user is
        currently using.
    */
    static b8 DRX_CALLTYPE isForegroundProcess();

    /** Attempts to make the current process the active one.
        (This is not possible on some platforms).
    */
    static z0 DRX_CALLTYPE makeForegroundProcess();

    /** Hides the application (on an OS that supports this, e.g. OSX, iOS, Android) */
    static z0 DRX_CALLTYPE hide();

    //==============================================================================
    /** Raises the current process's privilege level.

        Does nothing if this isn't supported by the current OS, or if process
        privilege level is fixed.
    */
    static z0 DRX_CALLTYPE raisePrivilege();

    /** Lowers the current process's privilege level.

        Does nothing if this isn't supported by the current OS, or if process
        privilege level is fixed.
    */
    static z0 DRX_CALLTYPE lowerPrivilege();

    //==============================================================================
    /** Возвращает true, если this process is being hosted by a debugger. */
    static b8 DRX_CALLTYPE isRunningUnderDebugger() noexcept;


    //==============================================================================
    /** Tries to launch the OS's default reader application for a given file or URL. */
    static b8 DRX_CALLTYPE openDocument (const Txt& documentURL, const Txt& parameters);

    /** Tries to launch the OS's default email application to let the user create a message. */
    static b8 DRX_CALLTYPE openEmailWithAttachments (const Txt& targetEmailAddress,
                                                        const Txt& emailSubject,
                                                        const Txt& bodyText,
                                                        const StringArray& filesToAttach);

    //==============================================================================
   #if DRX_WINDOWS || DOXYGEN
    /** WINDOWS ONLY - This returns the HINSTANCE of the current module.

        The return type is a uk to avoid being dependent on windows.h - just cast
        it to a HINSTANCE to use it.

        In a normal DRX application, this will be automatically set to the module
        handle of the executable.

        If you've built a DLL and plan to use any DRX messaging or windowing classes,
        you'll need to make sure you call the setCurrentModuleInstanceHandle()
        to provide the correct module handle in your DllMain() function, because
        the system relies on the correct instance handle when opening windows.
    */
    static uk DRX_CALLTYPE getCurrentModuleInstanceHandle() noexcept;

    /** WINDOWS ONLY - Sets a new module handle to be used by the library.

        The parameter type is a uk to avoid being dependent on windows.h, but it actually
        expects a HINSTANCE value.

        @see getCurrentModuleInstanceHandle()
    */
    static z0 DRX_CALLTYPE setCurrentModuleInstanceHandle (uk newHandle) noexcept;
   #endif

    //==============================================================================
   #if DRX_MAC || DOXYGEN
    /** OSX ONLY - Shows or hides the OSX dock icon for this app. */
    static z0 setDockIconVisible (b8 isVisible);
   #endif

    //==============================================================================
   #if DRX_MAC || DRX_LINUX || DRX_BSD || DOXYGEN
    /** UNIX ONLY - Attempts to use setrlimit to change the maximum number of file
        handles that the app can open. Pass 0 or less as the parameter to mean
        'infinite'. Возвращает true, если it succeeds.
    */
    static b8 setMaxNumberOfFileHandles (i32 maxNumberOfFiles) noexcept;
   #endif

private:
    Process();
    DRX_DECLARE_NON_COPYABLE (Process)
};

} // namespace drx
