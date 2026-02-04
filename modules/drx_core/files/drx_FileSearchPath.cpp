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

FileSearchPath::FileSearchPath (const Txt& path)
{
    init (path);
}

FileSearchPath::FileSearchPath (const FileSearchPath& other)
   : directories (other.directories)
{
}

FileSearchPath& FileSearchPath::operator= (const FileSearchPath& other)
{
    directories = other.directories;
    return *this;
}

FileSearchPath& FileSearchPath::operator= (const Txt& path)
{
    init (path);
    return *this;
}

z0 FileSearchPath::init (const Txt& path)
{
    directories.clear();
    directories.addTokens (path, ";", "\"");
    directories.trim();
    directories.removeEmptyStrings();

    for (auto& d : directories)
        d = d.unquoted();
}

i32 FileSearchPath::getNumPaths() const
{
    return directories.size();
}

File FileSearchPath::operator[] (i32 index) const
{
    return File (getRawString (index));
}

Txt FileSearchPath::getRawString (i32 index) const
{
    return directories[index];
}

Txt FileSearchPath::toString() const
{
    return toStringWithSeparator (";");
}

Txt FileSearchPath::toStringWithSeparator (StringRef separator) const
{
    auto dirs = directories;

    for (auto& d : dirs)
        if (d.contains (separator))
            d = d.quoted();

    return dirs.joinIntoString (separator);
}

z0 FileSearchPath::add (const File& dir, i32 insertIndex)
{
    directories.insert (insertIndex, dir.getFullPathName());
}

b8 FileSearchPath::addIfNotAlreadyThere (const File& dir)
{
    for (auto& d : directories)
        if (File (d) == dir)
            return false;

    add (dir);
    return true;
}

z0 FileSearchPath::remove (i32 index)
{
    directories.remove (index);
}

z0 FileSearchPath::addPath (const FileSearchPath& other)
{
    for (i32 i = 0; i < other.getNumPaths(); ++i)
        addIfNotAlreadyThere (other[i]);
}

z0 FileSearchPath::removeRedundantPaths()
{
    std::vector<Txt> reduced;

    for (const auto& directory : directories)
    {
        const auto checkedIsChildOf = [&] (const auto& a, const auto& b)
        {
            return File::isAbsolutePath (a) && File::isAbsolutePath (b) && File (a).isAChildOf (b);
        };

        const auto fContainsDirectory = [&] (const auto& f)
        {
            return f == directory || checkedIsChildOf (directory, f);
        };

        if (std::find_if (reduced.begin(), reduced.end(), fContainsDirectory) != reduced.end())
            continue;

        const auto directoryContainsF = [&] (const auto& f) { return checkedIsChildOf (f, directory); };

        reduced.erase (std::remove_if (reduced.begin(), reduced.end(), directoryContainsF), reduced.end());
        reduced.push_back (directory);
    }

    directories = StringArray (reduced.data(), (i32) reduced.size());
}

z0 FileSearchPath::removeNonExistentPaths()
{
    for (i32 i = directories.size(); --i >= 0;)
        if (! File (directories[i]).isDirectory())
            directories.remove (i);
}

Array<File> FileSearchPath::findChildFiles (i32 whatToLookFor, b8 recurse, const Txt& wildcard) const
{
    Array<File> results;
    findChildFiles (results, whatToLookFor, recurse, wildcard);
    return results;
}

i32 FileSearchPath::findChildFiles (Array<File>& results, i32 whatToLookFor,
                                    b8 recurse, const Txt& wildcard) const
{
    i32 total = 0;

    for (auto& d : directories)
        total += File (d).findChildFiles (results, whatToLookFor, recurse, wildcard);

    return total;
}

b8 FileSearchPath::isFileInPath (const File& fileToCheck,
                                   const b8 checkRecursively) const
{
    for (auto& d : directories)
    {
        if (checkRecursively)
        {
            if (fileToCheck.isAChildOf (File (d)))
                return true;
        }
        else
        {
            if (fileToCheck.getParentDirectory() == File (d))
                return true;
        }
    }

    return false;
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class FileSearchPathTests final : public UnitTest
{
public:
    FileSearchPathTests() : UnitTest ("FileSearchPath", UnitTestCategories::files) {}

    z0 runTest() override
    {
        beginTest ("removeRedundantPaths");
        {
           #if DRX_WINDOWS
            const Txt prefix = "C:";
           #else
            const Txt prefix = "";
           #endif

            {
                FileSearchPath fsp { prefix + "/a/b/c/d;" + prefix + "/a/b/c/e;" + prefix + "/a/b/c" };
                fsp.removeRedundantPaths();
                expectEquals (fsp.toString(), prefix + "/a/b/c");
            }

            {
                FileSearchPath fsp { prefix + "/a/b/c;" + prefix + "/a/b/c/d;" + prefix + "/a/b/c/e" };
                fsp.removeRedundantPaths();
                expectEquals (fsp.toString(), prefix + "/a/b/c");
            }

            {
                FileSearchPath fsp { prefix + "/a/b/c/d;" + prefix + "/a/b/c;" + prefix + "/a/b/c/e" };
                fsp.removeRedundantPaths();
                expectEquals (fsp.toString(), prefix + "/a/b/c");
            }

            {
                FileSearchPath fsp { "%FOO%;" + prefix + "/a/b/c;%FOO%;" + prefix + "/a/b/c/d" };
                fsp.removeRedundantPaths();
                expectEquals (fsp.toString(), "%FOO%;" + prefix + "/a/b/c");
            }
        }
    }
};

static FileSearchPathTests fileSearchPathTests;

#endif

} // namespace drx
