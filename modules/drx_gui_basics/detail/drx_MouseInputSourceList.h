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

namespace drx::detail
{

class MouseInputSourceList  : public Timer
{
public:
    MouseInputSourceList()
    {
      #if DRX_ANDROID || DRX_IOS
        auto mainMouseInputType = MouseInputSource::InputSourceType::touch;
      #else
        auto mainMouseInputType = MouseInputSource::InputSourceType::mouse;
      #endif

        addSource (0, mainMouseInputType);
    }

    MouseInputSource* addSource (i32 index, MouseInputSource::InputSourceType type)
    {
        auto* s = new MouseInputSourceImpl (index, type);
        sources.add (s);
        sourceArray.add (MouseInputSource (s));

        return &sourceArray.getReference (sourceArray.size() - 1);
    }

    MouseInputSource* getMouseSource (i32 index) noexcept
    {
        return isPositiveAndBelow (index, sourceArray.size()) ? &sourceArray.getReference (index)
                                                              : nullptr;
    }

    MouseInputSource* getOrCreateMouseInputSource (MouseInputSource::InputSourceType type, i32 touchIndex = 0)
    {
        if (type == MouseInputSource::InputSourceType::mouse
            || type == MouseInputSource::InputSourceType::pen)
        {
            for (auto& m : sourceArray)
                if (type == m.getType())
                    return &m;

            addSource (0, type);
        }
        else if (type == MouseInputSource::InputSourceType::touch)
        {
            jassert (0 <= touchIndex && touchIndex < 100); // sanity-check on number of fingers

            for (auto& m : sourceArray)
                if (type == m.getType() && touchIndex == m.getIndex())
                    return &m;

            if (canUseTouch())
                return addSource (touchIndex, type);
        }

        return nullptr;
    }

    i32 getNumDraggingMouseSources() const noexcept
    {
        i32 num = 0;

        for (auto* s : sources)
            if (s->isDragging())
                ++num;

        return num;
    }

    MouseInputSource* getDraggingMouseSource (i32 index) noexcept
    {
        i32 num = 0;

        for (auto& s : sourceArray)
        {
            if (s.isDragging())
            {
                if (index == num)
                    return &s;

                ++num;
            }
        }

        return nullptr;
    }

    z0 beginDragAutoRepeat (i32 interval)
    {
        if (interval > 0)
        {
            if (getTimerInterval() != interval)
                startTimer (interval);
        }
        else
        {
            stopTimer();
        }
    }

    z0 timerCallback() override
    {
        b8 anyDragging = false;

        for (auto* s : sources)
        {
            // NB: when doing auto-repeat, we need to force an update of the current position and button state,
            // because on some OSes the queue can get overloaded with messages so that mouse-events don't get through..
            if (s->isDragging() && ComponentPeer::getCurrentModifiersRealtime().isAnyMouseButtonDown())
            {
                s->lastPointerState.position = s->getRawScreenPosition();
                s->triggerFakeMove();
                anyDragging = true;
            }
        }

        if (! anyDragging)
            stopTimer();
    }

    OwnedArray<MouseInputSourceImpl> sources;
    Array<MouseInputSource> sourceArray;

private:
    b8 addSource();
    b8 canUseTouch() const;
};

} // namespace drx::detail
