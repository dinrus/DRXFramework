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

/** A list of events that can be notified to any subscribed accessibility clients.

    To post a notification, call `AccessibilityHandler::notifyAccessibilityEvent`
    on the associated handler with the appropriate `AccessibilityEvent` type and
    listening clients will be notified.

    @tags{Accessibility}
*/
enum class AccessibilityEvent
{
    /** Indicates that the UI element's value has changed.

        This should be called on the handler that implements `AccessibilityValueInterface`
        for the UI element that has changed.
    */
    valueChanged,

    /** Indicates that the title of the UI element has changed.

        This should be called on the handler whose title has changed.
    */
    titleChanged,

    /** Indicates that the structure of the UI elements has changed in a
        significant way.

        This should be called on the top-level handler whose structure has changed.
    */
    structureChanged,

    /** Indicates that the selection of a text element has changed.

        This should be called on the handler that implements `AccessibilityTextInterface`
        for the text element that has changed.
    */
    textSelectionChanged,

    /** Indicates that the visible text of a text element has changed.

        This should be called on the handler that implements `AccessibilityTextInterface`
        for the text element that has changed.
    */
    textChanged,

    /** Indicates that the selection of rows in a list or table has changed.

        This should be called on the handler that implements `AccessibilityTableInterface`
        for the UI element that has changed.
    */
    rowSelectionChanged
};

} // namespace drx
