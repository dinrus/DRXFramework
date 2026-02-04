/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation.

  ==============================================================================
*/

#pragma once

#include <drx_audio_processors/drx_audio_processors.h>

//==============================================================================
/**
*/
class %%aradocumentcontroller_class_name%%  : public drx::ARADocumentControllerSpecialisation
{
public:
    //==============================================================================
    using ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation;

protected:
    //==============================================================================
    // Override document controller customization methods here

    drx::ARAPlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

    b8 doRestoreObjectsFromStream (drx::ARAInputStream& input, const drx::ARARestoreObjectsFilter* filter) noexcept override;
    b8 doStoreObjectsToStream (drx::ARAOutputStream& output, const drx::ARAStoreObjectsFilter* filter) noexcept override;

private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%aradocumentcontroller_class_name%%)
};
