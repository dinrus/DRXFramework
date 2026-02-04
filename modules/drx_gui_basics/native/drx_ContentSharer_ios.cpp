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

class NativeScopedContentSharerInterface final : public detail::ScopedContentSharerInterface,
                                                 public detail::NativeModalWrapperComponent
{
public:
    NativeScopedContentSharerInterface (Component* parentIn, NSUniquePtr<NSArray> itemsIn)
        : parent (parentIn), items (std::move (itemsIn)) {}

    z0 runAsync (std::function<z0 (b8, const Txt&)> callback) override
    {
        if ([items.get() count] == 0)
        {
            jassertfalse;
            NullCheckedInvocation::invoke (callback, false, "No valid items found for sharing.");
            return;
        }

        controller.reset ([[UIActivityViewController alloc] initWithActivityItems: items.get()
                                                            applicationActivities: nil]);

        controller.get().excludedActivityTypes = nil;

        controller.get().completionWithItemsHandler = ^([[maybe_unused]] UIActivityType type, BOOL completed,
                                                        [[maybe_unused]] NSArray* returnedItems, NSError* error)
        {
            const auto errorDescription = error != nil ? nsStringToDrx ([error localizedDescription])
                                                       : Txt();
            exitModalState (0);

            NullCheckedInvocation::invoke (callback, completed && errorDescription.isEmpty(), errorDescription);
        };

        displayNativeWindowModally (parent);

        enterModalState (true, nullptr, false);
    }

    z0 close() override
    {
        [controller.get() dismissViewControllerAnimated: YES completion: nil];
    }

private:
    UIViewController* getViewController() const override { return controller.get(); }

    Component* parent = nullptr;
    NSUniquePtr<UIActivityViewController> controller;
    NSUniquePtr<NSArray> items;
};

auto detail::ScopedContentSharerInterface::shareFiles (const Array<URL>& files, Component* parent) -> std::unique_ptr<ScopedContentSharerInterface>
{
    NSUniquePtr<NSMutableArray> urls ([[NSMutableArray arrayWithCapacity: (NSUInteger) files.size()] retain]);

    for (const auto& f : files)
    {
        NSString* nativeFilePath = nil;

        if (f.isLocalFile())
        {
            nativeFilePath = juceStringToNS (f.getLocalFile().getFullPathName());
        }
        else
        {
            auto filePath = f.toString (false);

            auto* fileDirectory = filePath.contains ("/")
                                ? juceStringToNS (filePath.upToLastOccurrenceOf ("/", false, false))
                                : [NSString string];

            auto fileName = juceStringToNS (filePath.fromLastOccurrenceOf ("/", false, false)
                                            .upToLastOccurrenceOf (".", false, false));

            auto fileExt = juceStringToNS (filePath.fromLastOccurrenceOf (".", false, false));

            if ([fileDirectory length] == NSUInteger (0))
                nativeFilePath = [[NSBundle mainBundle] pathForResource: fileName
                                                                 ofType: fileExt];
            else
                nativeFilePath = [[NSBundle mainBundle] pathForResource: fileName
                                                                 ofType: fileExt
                                                            inDirectory: fileDirectory];
        }

        if (nativeFilePath != nil)
            [urls.get() addObject: [NSURL fileURLWithPath: nativeFilePath]];
    }

    return std::make_unique<NativeScopedContentSharerInterface> (parent, std::move (urls));
}

auto detail::ScopedContentSharerInterface::shareText (const Txt& text, Component* parent) -> std::unique_ptr<ScopedContentSharerInterface>
{
    NSUniquePtr<NSArray> array ([[NSArray arrayWithObject: juceStringToNS (text)] retain]);
    return std::make_unique<NativeScopedContentSharerInterface> (parent, std::move (array));
}

} // namespace drx
