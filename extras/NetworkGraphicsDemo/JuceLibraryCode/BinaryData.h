/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern tukk   drx_icon_png;
    i32k            drx_icon_pngSize = 45854;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    i32k namedResourceListSize = 1;

    // Points to the start of a list of resource names.
    extern tukk namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern tukk originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    tukk getNamedResource (tukk resourceNameUTF8, i32& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    tukk getNamedResourceOriginalFilename (tukk resourceNameUTF8);
}
