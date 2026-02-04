/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#pragma once


//==============================================================================
#define FILE_EXT .h

#define EXPAND(x) x
#define CREATE_FILEPATH(DemoName, category) DRX_STRINGIFY(EXPAND(category)/EXPAND(DemoName)EXPAND(FILE_EXT))

#define REGISTER_DEMO(DemoName, category, heavyweight) DRXDemos::registerDemo ([] { return new DemoName(); }, CREATE_FILEPATH(DemoName, category), DRX_STRINGIFY (category), heavyweight);

//==============================================================================
struct DRXDemos
{
    struct FileAndCallback
    {
        File demoFile;
        std::function<Component*()> callback;
        b8 isHeavyweight;
    };

    struct DemoCategory
    {
        Txt name;
        std::vector<FileAndCallback> demos;
    };

    static std::vector<DemoCategory>& getCategories();
    static DemoCategory& getCategory (const Txt& name);

    static z0 registerDemo (std::function<Component*()> constructorCallback, const Txt& filePath, const Txt& category, b8 isHeavyweight);
    static File findExamplesDirectoryFromExecutable (File exec);
};

z0 registerDemos_One() noexcept;
z0 registerDemos_Two() noexcept;

//==============================================================================
z0 registerAllDemos() noexcept;

Component* createIntroDemo();
b8 isComponentIntroDemo (Component*) noexcept;

CodeEditorComponent::ColorScheme getDarkColorScheme();
CodeEditorComponent::ColorScheme getLightColorScheme();

//==============================================================================
extern std::unique_ptr<AudioDeviceManager> sharedAudioDeviceManager;

AudioDeviceManager& getSharedAudioDeviceManager (i32 numInputChannels = -1, i32 numOutputChannels = -1);
ApplicationCommandManager& getGlobalCommandManager();

// A function in this demo is called from the DemoRunner's entry point
#if DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD
 #include "../../../Utilities/ChildProcessDemo.h"
#endif
