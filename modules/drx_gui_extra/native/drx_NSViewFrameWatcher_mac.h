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

#if DRX_MAC

namespace drx
{

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
const auto nsViewFrameChangedSelector = @selector (frameChanged:);
DRX_END_IGNORE_WARNINGS_GCC_LIKE

struct NSViewCallbackInterface
{
    virtual ~NSViewCallbackInterface() = default;
    virtual z0 frameChanged() = 0;
};

//==============================================================================
struct NSViewFrameChangeCallbackClass   : public ObjCClass<NSObject>
{
    NSViewFrameChangeCallbackClass()
        : ObjCClass ("DRX_NSViewCallback_")
    {
        addIvar<NSViewCallbackInterface*> ("target");

        addMethod (nsViewFrameChangedSelector, frameChanged);

        registerClass();
    }

    static z0 setTarget (id self, NSViewCallbackInterface* c)
    {
        object_setInstanceVariable (self, "target", c);
    }

private:
    static z0 frameChanged (id self, SEL, NSNotification*)
    {
        if (auto* target = getIvar<NSViewCallbackInterface*> (self, "target"))
            target->frameChanged();
    }

    DRX_DECLARE_NON_COPYABLE (NSViewFrameChangeCallbackClass)
};

//==============================================================================
class NSViewFrameWatcher : private NSViewCallbackInterface
{
public:
    NSViewFrameWatcher (NSView* viewToWatch, std::function<z0()> viewResizedIn)
        : viewResized (std::move (viewResizedIn)), callback (makeCallbackForView (viewToWatch))
    {
    }

    ~NSViewFrameWatcher() override
    {
        [[NSNotificationCenter defaultCenter] removeObserver: callback];
        [callback release];
        callback = nil;
    }

    DRX_DECLARE_NON_COPYABLE (NSViewFrameWatcher)
    DRX_DECLARE_NON_MOVEABLE (NSViewFrameWatcher)

private:
    id makeCallbackForView (NSView* view)
    {
        static NSViewFrameChangeCallbackClass cls;
        auto* result = [cls.createInstance() init];
        NSViewFrameChangeCallbackClass::setTarget (result, this);

        [[NSNotificationCenter defaultCenter]  addObserver: result
                                                  selector: nsViewFrameChangedSelector
                                                      name: NSViewFrameDidChangeNotification
                                                    object: view];

        return result;
    }

    z0 frameChanged() override { viewResized(); }

    std::function<z0()> viewResized;
    id callback;
};

} // namespace drx

#endif
