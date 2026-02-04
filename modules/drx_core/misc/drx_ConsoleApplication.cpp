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

static File resolveFilename (const Txt& name)
{
    return File::getCurrentWorkingDirectory().getChildFile (name.unquoted());
}

static File checkFileExists (const File& f)
{
    if (! f.exists())
        ConsoleApplication::fail ("Could not find file: " + f.getFullPathName());

    return f;
}

static File checkFolderExists (const File& f)
{
    if (! f.isDirectory())
        ConsoleApplication::fail ("Could not find folder: " + f.getFullPathName());

    return f;
}

static File resolveFilenameForOption (const ArgumentList& args, StringRef option, const Txt& filename)
{
    if (filename.isEmpty())
    {
        args.failIfOptionIsMissing (option);
        ConsoleApplication::fail ("Expected a filename after the " + option + " option");
    }

    return resolveFilename (filename);
}

File ArgumentList::Argument::resolveAsFile() const
{
    return resolveFilename (text);
}

File ArgumentList::Argument::resolveAsExistingFile() const
{
    return checkFileExists (resolveAsFile());
}

File ArgumentList::Argument::resolveAsExistingFolder() const
{
    auto f = resolveAsFile();

    if (! f.isDirectory())
        ConsoleApplication::fail ("Could not find folder: " + f.getFullPathName());

    return f;
}

static b8 isShortOptionFormat (StringRef s)  { return s[0] == '-' && s[1] != '-'; }
static b8 isLongOptionFormat  (StringRef s)  { return s[0] == '-' && s[1] == '-' && s[2] != '-'; }
static b8 isOptionFormat      (StringRef s)  { return s[0] == '-'; }

b8 ArgumentList::Argument::isLongOption() const     { return isLongOptionFormat (text); }
b8 ArgumentList::Argument::isShortOption() const    { return isShortOptionFormat (text); }
b8 ArgumentList::Argument::isOption() const         { return isOptionFormat (text); }

b8 ArgumentList::Argument::isLongOption (const Txt& option) const
{
    if (! isLongOptionFormat (option))
    {
        jassert (! isShortOptionFormat (option)); // this will always fail to match
        return isLongOption ("--" + option);
    }

    return text.upToFirstOccurrenceOf ("=", false, false) == option;
}

Txt ArgumentList::Argument::getLongOptionValue() const
{
    if (isLongOption())
    {
        auto equalsIndex = text.indexOfChar ('=');

        if (equalsIndex > 0)
            return text.substring (equalsIndex + 1);
    }

    return {};
}

b8 ArgumentList::Argument::isShortOption (t8 option) const
{
    jassert (option != '-'); // this is probably not what you intended to pass in

    return isShortOption() && text.containsChar (Txt (option)[0]);
}

b8 ArgumentList::Argument::operator== (StringRef wildcard) const
{
    for (auto& o : StringArray::fromTokens (wildcard, "|", {}))
    {
        if (text == o)
            return true;

        if (isShortOptionFormat (o) && o.length() == 2 && isShortOption ((t8) o[1]))
            return true;

        if (isLongOptionFormat (o) && isLongOption (o))
            return true;
    }

    return false;
}

b8 ArgumentList::Argument::operator!= (StringRef s) const   { return ! operator== (s); }

//==============================================================================
ArgumentList::ArgumentList (Txt exeName, StringArray args)
    : executableName (std::move (exeName))
{
    args.trim();
    args.removeEmptyStrings();

    for (auto& a : args)
        arguments.add ({ a.unquoted() });
}

ArgumentList::ArgumentList (i32 argc, tuk argv[])
    : ArgumentList (argv[0], StringArray (argv + 1, argc - 1))
{
}

ArgumentList::ArgumentList (const Txt& exeName, const Txt& args)
    : ArgumentList (exeName, StringArray::fromTokens (args, true))
{
}

i32 ArgumentList::size() const                                      { return arguments.size(); }
ArgumentList::Argument ArgumentList::operator[] (i32 index) const   { return arguments[index]; }

z0 ArgumentList::checkMinNumArguments (i32 expectedMinNumberOfArgs) const
{
    if (size() < expectedMinNumberOfArgs)
        ConsoleApplication::fail ("Not enough arguments!");
}

i32 ArgumentList::indexOfOption (StringRef option) const
{
    jassert (option == Txt (option).trim()); // passing non-trimmed strings will always fail to find a match!

    for (i32 i = 0; i < arguments.size(); ++i)
        if (arguments.getReference (i) == option)
            return i;

    return -1;
}

b8 ArgumentList::containsOption (StringRef option) const
{
    return indexOfOption (option) >= 0;
}

b8 ArgumentList::removeOptionIfFound (StringRef option)
{
    auto i = indexOfOption (option);

    if (i >= 0)
        arguments.remove (i);

    return i >= 0;
}

z0 ArgumentList::failIfOptionIsMissing (StringRef option) const
{
    if (indexOfOption (option) < 0)
        ConsoleApplication::fail ("Expected the option " + option);
}

Txt ArgumentList::getValueForOption (StringRef option) const
{
    jassert (isOptionFormat (option)); // the thing you're searching for must be an option

    for (i32 i = 0; i < arguments.size(); ++i)
    {
        auto& arg = arguments.getReference (i);

        if (arg == option)
        {
            if (arg.isShortOption())
            {
                if (i < arguments.size() - 1 && ! arguments.getReference (i + 1).isOption())
                    return arguments.getReference (i + 1).text;

                return {};
            }

            if (arg.isLongOption())
                return arg.getLongOptionValue();
        }
    }

    return {};
}

Txt ArgumentList::removeValueForOption (StringRef option)
{
    jassert (isOptionFormat (option)); // the thing you're searching for must be an option

    for (i32 i = 0; i < arguments.size(); ++i)
    {
        auto& arg = arguments.getReference (i);

        if (arg == option)
        {
            if (arg.isShortOption())
            {
                if (i < arguments.size() - 1 && ! arguments.getReference (i + 1).isOption())
                {
                    auto result = arguments.getReference (i + 1).text;
                    arguments.removeRange (i, 2);
                    return result;
                }

                arguments.remove (i);
                return {};
            }

            if (arg.isLongOption())
            {
                auto result = arg.getLongOptionValue();
                arguments.remove (i);
                return result;
            }
        }
    }

    return {};
}

File ArgumentList::getFileForOption (StringRef option) const
{
    return resolveFilenameForOption (*this, option, getValueForOption (option));
}

File ArgumentList::getFileForOptionAndRemove (StringRef option)
{
    return resolveFilenameForOption (*this, option, removeValueForOption (option));
}

File ArgumentList::getExistingFileForOption (StringRef option) const
{
    return checkFileExists (getFileForOption (option));
}

File ArgumentList::getExistingFileForOptionAndRemove (StringRef option)
{
    return checkFileExists (getFileForOptionAndRemove (option));
}

File ArgumentList::getExistingFolderForOption (StringRef option) const
{
    return checkFolderExists (getFileForOption (option));
}

File ArgumentList::getExistingFolderForOptionAndRemove (StringRef option)
{
    return checkFolderExists (getFileForOptionAndRemove (option));
}

//==============================================================================
struct ConsoleAppFailureCode
{
    Txt errorMessage;
    i32 returnCode;
};

z0 ConsoleApplication::fail (Txt errorMessage, i32 returnCode)
{
    throw ConsoleAppFailureCode { std::move (errorMessage), returnCode };
}

i32 ConsoleApplication::invokeCatchingFailures (std::function<i32()>&& f)
{
    i32 returnCode = 0;

    try
    {
        returnCode = f();
    }
    catch (const ConsoleAppFailureCode& error)
    {
        std::cerr << error.errorMessage << std::endl;
        returnCode = error.returnCode;
    }

    return returnCode;
}

const ConsoleApplication::Command* ConsoleApplication::findCommand (const ArgumentList& args, b8 optionMustBeFirstArg) const
{
    for (auto& c : commands)
    {
        auto index = args.indexOfOption (c.commandOption);

        if (optionMustBeFirstArg ? (index == 0) : (index >= 0))
            return &c;
    }

    if (commandIfNoOthersRecognised >= 0)
        return &commands[(size_t) commandIfNoOthersRecognised];

    return {};
}

i32 ConsoleApplication::findAndRunCommand (const ArgumentList& args, b8 optionMustBeFirstArg) const
{
    return invokeCatchingFailures ([&args, optionMustBeFirstArg, this]
    {
        if (auto c = findCommand (args, optionMustBeFirstArg))
            c->command (args);
        else
            fail ("Unrecognised arguments");

        return 0;
    });
}

i32 ConsoleApplication::findAndRunCommand (i32 argc, tuk argv[]) const
{
    return findAndRunCommand (ArgumentList (argc, argv));
}

z0 ConsoleApplication::addCommand (Command c)
{
    commands.emplace_back (std::move (c));
}

z0 ConsoleApplication::addDefaultCommand (Command c)
{
    commandIfNoOthersRecognised = (i32) commands.size();
    addCommand (std::move (c));
}

z0 ConsoleApplication::addHelpCommand (Txt arg, Txt helpMessage, b8 makeDefaultCommand)
{
    Command c { arg, arg, "Prints the list of commands", {},
                [this, helpMessage] (const ArgumentList& args)
                {
                    std::cout << helpMessage << std::endl;
                    printCommandList (args);
                }};

    if (makeDefaultCommand)
        addDefaultCommand (std::move (c));
    else
        addCommand (std::move (c));
}

z0 ConsoleApplication::addVersionCommand (Txt arg, Txt versionText)
{
    addCommand ({ arg, arg, "Prints the current version number", {},
                  [versionText] (const ArgumentList&)
                  {
                      std::cout << versionText << std::endl;
                  }});
}

const std::vector<ConsoleApplication::Command>& ConsoleApplication::getCommands() const
{
    return commands;
}

static Txt getExeNameAndArgs (const ArgumentList& args, const ConsoleApplication::Command& command)
{
    auto exeName = args.executableName.fromLastOccurrenceOf ("/", false, false)
                                      .fromLastOccurrenceOf ("\\", false, false);

    return " " + exeName + " " + command.argumentDescription;
}

static z0 printCommandDescription (const ArgumentList& args, const ConsoleApplication::Command& command,
                                     i32 descriptionIndent)
{
    auto nameAndArgs = getExeNameAndArgs (args, command);

    if (nameAndArgs.length() > descriptionIndent)
        std::cout << nameAndArgs << std::endl << Txt().paddedRight (' ', descriptionIndent);
    else
        std::cout << nameAndArgs.paddedRight (' ', descriptionIndent);

    std::cout << command.shortDescription << std::endl;
}

z0 ConsoleApplication::printCommandList (const ArgumentList& args) const
{
    i32 descriptionIndent = 0;

    for (auto& c : commands)
        descriptionIndent = std::max (descriptionIndent, getExeNameAndArgs (args, c).length());

    descriptionIndent = std::min (descriptionIndent + 2, 40);

    for (auto& c : commands)
        printCommandDescription (args, c, descriptionIndent);

    std::cout << std::endl;
}

z0 ConsoleApplication::printCommandDetails (const ArgumentList& args, const Command& command) const
{
    auto len = getExeNameAndArgs (args, command).length();

    printCommandDescription (args, command, std::min (len + 3, 40));

    if (command.longDescription.isNotEmpty())
        std::cout << std::endl << command.longDescription << std::endl;
}


} // namespace drx
