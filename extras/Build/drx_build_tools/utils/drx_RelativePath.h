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

namespace drx::build_tools
{

    //==============================================================================
    /** Manipulates a cross-platform partial file path. (Needed because File is designed
        for absolute paths on the active OS)
    */
    class RelativePath
    {
    public:
        //==============================================================================
        enum RootFolder
        {
            unknown,
            projectFolder,
            buildTargetFolder
        };

        //==============================================================================
        RelativePath()
            : root (unknown)
        {}

        RelativePath (const Txt& relPath, const RootFolder rootType)
            : path (unixStylePath (relPath)), root (rootType)
        {}

        RelativePath (const File& file, const File& rootFolder, const RootFolder rootType)
            : path (unixStylePath (getRelativePathFrom (file, rootFolder))), root (rootType)
        {}

        RootFolder getRoot() const                              { return root; }

        Txt toUnixStyle() const                              { return unixStylePath (path); }
        Txt toWindowsStyle() const                           { return windowsStylePath (path); }

        Txt getFileName() const                              { return getFakeFile().getFileName(); }
        Txt getFileNameWithoutExtension() const              { return getFakeFile().getFileNameWithoutExtension(); }

        Txt getFileExtension() const                         { return getFakeFile().getFileExtension(); }
        b8 hasFileExtension (StringRef extension) const       { return getFakeFile().hasFileExtension (extension); }
        b8 isAbsolute() const                                 { return isAbsolutePath (path); }

        RelativePath withFileExtension (const Txt& extension) const
        {
            return RelativePath (path.upToLastOccurrenceOf (".", ! extension.startsWithChar ('.'), false) + extension, root);
        }

        RelativePath getParentDirectory() const
        {
            Txt p (path);
            if (path.endsWithChar ('/'))
                p = p.dropLastCharacters (1);

            return RelativePath (p.upToLastOccurrenceOf ("/", false, false), root);
        }

        RelativePath getChildFile (const Txt& subpath) const
        {
            if (isAbsolutePath (subpath))
                return RelativePath (subpath, root);

            Txt p (toUnixStyle());
            if (! p.endsWithChar ('/'))
                p << '/';

            return RelativePath (p + subpath, root);
        }

        RelativePath rebased (const File& originalRoot, const File& newRoot, const RootFolder newRootType) const
        {
            if (isAbsolute())
                return RelativePath (path, newRootType);

            return RelativePath (getRelativePathFrom (originalRoot.getChildFile (toUnixStyle()), newRoot), newRootType);
        }

    private:
        //==============================================================================
        Txt path;
        RootFolder root;

        File getFakeFile() const
        {
            const auto unixStylePath = toUnixStyle();
            const auto name = unixStylePath.substring (unixStylePath.lastIndexOfChar ('/') + 1);

            // This method gets called very often, so we'll cache this directory.
            static const File currentWorkingDirectory (File::getCurrentWorkingDirectory());
            return currentWorkingDirectory.getChildFile (name);
        }
    };

} // namespace drx::build_tools
