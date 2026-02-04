/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern tukk   cassette_recorder_wav;
    i32k            cassette_recorder_wavSize = 37902;

    extern tukk   cello_wav;
    i32k            cello_wavSize = 46348;

    extern tukk   guitar_amp_wav;
    i32k            guitar_amp_wavSize = 90246;

    extern tukk   proaudio_path;
    i32k            proaudio_pathSize = 452;

    extern tukk   reverb_ir_wav;
    i32k            reverb_ir_wavSize = 648404;

    extern tukk   singing_ogg;
    i32k            singing_oggSize = 15354;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    i32k namedResourceListSize = 6;

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
