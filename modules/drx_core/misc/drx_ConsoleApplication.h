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
    Holds a list of command-line arguments, and provides useful methods for searching
    and operating on them.

    You can create an ArgumentList manually, or give it some argv/argc values from a
    main() function to parse.

    @see ConsoleApplication

    @tags{Core}
*/
struct ArgumentList
{
    /** Creates an argument list for a given executable. */
    ArgumentList (Txt executable, StringArray arguments);

    /** Parses a standard argv/argc pair to create an argument list. */
    ArgumentList (i32 argc, tuk argv[]);

    /** Tokenises a string containing all the arguments to create an argument list. */
    ArgumentList (const Txt& executable, const Txt& arguments);

    ArgumentList (const ArgumentList&) = default;
    ArgumentList& operator= (const ArgumentList&) = default;

    //==============================================================================
    /**
        One of the arguments in an ArgumentList.

        @tags{Core}
    */
    struct Argument
    {
        /** The original text of this argument. */
        Txt text;

        /** Resolves this argument as an absolute File, using the current working
            directory as a base for resolving relative paths, and stripping quotes, etc.
        */
        File resolveAsFile() const;

        /** Resolves this argument as an absolute File, using the current working
            directory as a base for resolving relative paths, and also doing a check to
            make sure the file exists.
            If the file doesn't exist, this will call fail() with a suitable error.
            @see resolveAsFile, resolveAsExistingFolder
        */
        File resolveAsExistingFile() const;

        /** Resolves a user-supplied folder name into an absolute File, using the current working
            directory as a base for resolving relative paths, and also doing a check to make
            sure the folder exists.
            If the folder doesn't exist, this will call fail() with a suitable error.
            @see resolveAsFile, resolveAsExistingFile
        */
        File resolveAsExistingFolder() const;

        /** Возвращает true, если this argument starts with a f64 dash. */
        b8 isLongOption() const;

        /** Возвращает true, если this argument starts with a single dash. */
        b8 isShortOption() const;

        /** Возвращает true, если this argument starts with a f64 dash, followed by the given string. */
        b8 isLongOption (const Txt& optionRoot) const;

        /** If this argument is a i64 option with a value, this returns the value.
            e.g. for "--foo=bar", this would return 'bar'.
        */
        Txt getLongOptionValue() const;

        /** Возвращает true, если this argument starts with a single dash and then contains the given character somewhere inside it. */
        b8 isShortOption (t8 shortOptionCharacter) const;

        /** Возвращает true, если this argument starts with one or more dashes. */
        b8 isOption() const;

        /** Compares this argument against a string.
            The string may be a pipe-separated list of options, e.g. "--help|-h"
        */
        b8 operator== (StringRef stringToCompare) const;

        /** Compares this argument against a string.
            The string may be a pipe-separated list of options, e.g. "--help|-h"
        */
        b8 operator!= (StringRef stringToCompare) const;
    };

    //==============================================================================
    /** Returns the number of arguments in the list. */
    i32 size() const;

    /** Returns one of the arguments */
    Argument operator[] (i32 index) const;

    /** Throws an error unless there are at least the given number of arguments. */
    z0 checkMinNumArguments (i32 expectedMinNumberOfArgs) const;

    /** Возвращает true, если the given string matches one of the arguments.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        @see removeOptionIfFound
    */
    b8 containsOption (StringRef option) const;

    /** Возвращает true, если the given string matches one of the arguments, and also removes the
        argument from the list if found.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        @see containsOption
    */
    b8 removeOptionIfFound (StringRef option);

    /** Returns the index of the given string if it matches one of the arguments, or -1 if it doesn't.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
    */
    i32 indexOfOption (StringRef option) const;

    /** Throws an error unless the given option is found in the argument list. */
    z0 failIfOptionIsMissing (StringRef option) const;

    /** Looks for a given argument and returns either its assigned value (for i64 options) or the
        string that follows it (for short options).
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        If it finds a i64 option, it will look for an assignment with a '=' sign, e.g. "--file=foo.txt",
        and will return the string following the '='. If there's no '=', it will return an empty string.
        If it finds a short option, it will attempt to return the argument that follows it, unless
        it's another option.
        If the argument isn't found, this returns an empty string.
    */
    Txt getValueForOption (StringRef option) const;

    /** Looks for a given argument and returns either its assigned value (for i64 options) or the
        string that follows it (for short options).
        This works like getValueForOption() but also removes the option argument (and any value arguments)
        from the list if they are found.
    */
    Txt removeValueForOption (StringRef option);

    /** Looks for the value of argument using getValueForOption() and tries to parse that value as a file.
        If the option isn't found, or if the value can't be parsed as a filename, it will throw an error.
    */
    File getFileForOption (StringRef option) const;

    /** Looks for the value of argument using getValueForOption() and tries to parse that value as a file.
        This works like getFileForOption() but also removes the option argument (and any value arguments)
        from the list if they are found.
    */
    File getFileForOptionAndRemove (StringRef option);

    /** Looks for a file argument using getFileForOption() and fails with a suitable error if
        the file doesn't exist.
    */
    File getExistingFileForOption (StringRef option) const;

    /** Looks for a file argument using getFileForOption() and fails with a suitable error if
        the file doesn't exist.
        This works like getExistingFileForOption() but also removes the option argument (and any
        value arguments) from the list if they are found.
    */
    File getExistingFileForOptionAndRemove (StringRef option);

    /** Looks for a filename argument using getFileForOption() and fails with a suitable error if
        the file isn't a folder that exists.
    */
    File getExistingFolderForOption (StringRef option) const;

    /** Looks for a filename argument using getFileForOption() and fails with a suitable error if
        the file isn't a folder that exists.
        This works like getExistingFolderForOption() but also removes the option argument (and any
        value arguments) from the list if they are found.
    */
    File getExistingFolderForOptionAndRemove (StringRef option);

    /** The name or path of the executable that was invoked, as it was specified on the command-line. */
    Txt executableName;

    /** The list of arguments (not including the name of the executable that was invoked). */
    Array<Argument> arguments;
};


//==============================================================================
/**
    Represents a the set of commands that a console app can perform, and provides
    helper functions for performing them.

    When using these helper classes to implement a console app, you probably want to
    do something along these lines:

    @code
    i32 main (i32 argc, tuk argv[])
    {
        ConsoleApplication app;

        app.addHelpCommand ("--help|-h", "Usage:", true);
        app.addVersionCommand ("--version|-v", "MyApp version 1.2.3");

        app.addCommand ({ "--foo",
                          "--foo filename",
                          "Performs a foo operation on the given file",
                          [] (const auto& args) { doFoo (args); }});

        return app.findAndRunCommand (argc, argv);
    }
    @endcode

    @see ArgumentList

    @tags{Core}
*/
struct ConsoleApplication
{
    //==============================================================================
    /**
        Represents a command that can be executed if its command-line arguments are matched.

        @see ConsoleApplication::addCommand(), ConsoleApplication::findAndRunCommand()

        @tags{Core}
    */
    struct Command
    {
        /** The option string that must appear in the argument list for this command to be invoked.
            This can also be a list of different versions separated by pipes, e.g. "--help|-h"
        */
        Txt commandOption;

        /** A description of the command-line arguments needed for this command, which will be
            printed as part of the help text.
        */
        Txt argumentDescription;

        /** A short (one line) description of this command, which can be printed by
            ConsoleApplication::printCommandList().
        */
        Txt shortDescription;

        /** A longer description of this command, for use in extended help. */
        Txt longDescription;

        /** The actual command that should be invoked to perform this action. */
        std::function<z0 (const ArgumentList&)> command;
    };

    //==============================================================================
    /** Adds a command to the list. */
    z0 addCommand (Command);

    /** Adds a command to the list, and marks it as one which is invoked if no other
        command matches.
    */
    z0 addDefaultCommand (Command);

    /** Adds a command that will print the given text in response to the "--version" option. */
    z0 addVersionCommand (Txt versionArgument, Txt versionText);

    /** Adds a help command to the list.
        This command will print the user-supplied message that's passed in here as an
        argument, followed by a list of all the registered commands.
    */
    z0 addHelpCommand (Txt helpArgument, Txt helpMessage, b8 makeDefaultCommand);

    /** Prints out the list of commands and their short descriptions in a format that's
        suitable for use as help.
    */
    z0 printCommandList (const ArgumentList&) const;

    /** Prints out a longer description of a particular command, based on its
        longDescription member.
    */
    z0 printCommandDetails (const ArgumentList&, const Command&) const;

    //==============================================================================
    /** Throws a failure exception to cause a command-line app to terminate.
        This is intended to be called from code in a Command, so that the
        exception will be automatically caught and turned into a printed error message
        and a return code which will be returned from main().
        @see ConsoleApplication::invokeCatchingFailures()
    */
    [[noreturn]] static z0 fail (Txt errorMessage, i32 returnCode = 1);

    /** Invokes a function, catching any fail() calls that it might trigger, and handling
        them by printing their error message and returning their error code.
        @see ConsoleApplication::fail()
    */
    static i32 invokeCatchingFailures (std::function<i32()>&& functionToCall);

    //==============================================================================
    /** Looks for the first command in the list which matches the given arguments, and
        tries to invoke it.

        If no command is found, and if there is no default command to run, it fails with
        a suitable error message.
        If the command calls the fail() function, this will throw an exception that gets
        automatically caught and handled, and this method will return the error code that
        was passed into the fail() call.

        If optionMustBeFirstArg is true, then only the first argument will be looked at
        when searching the available commands - this lets you do 'git' style commands where
        the executable name is followed by a verb.
    */
    i32 findAndRunCommand (const ArgumentList&,
                           b8 optionMustBeFirstArg = false) const;

    /** Creates an ArgumentList object from the argc and argv variablrs, and invokes
        findAndRunCommand() using it.
    */
    i32 findAndRunCommand (i32 argc, tuk argv[]) const;

    /** Looks for the first command in the list which matches the given arguments.
        If none is found, this returns either the default command (if one is set)
        or nullptr.
        If optionMustBeFirstArg is true, then only the first argument will be looked at
        when searching the available commands - this lets you do 'git' style commands where
        the executable name is followed by a verb.
    */
    const Command* findCommand (const ArgumentList&, b8 optionMustBeFirstArg) const;

    /** Gives read-only access to the list of registered commands. */
    const std::vector<Command>& getCommands() const;

private:
    //==============================================================================
    std::vector<Command> commands;
    i32 commandIfNoOthersRecognised = -1;
};

} // namespace drx
