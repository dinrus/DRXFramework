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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.text.Selection;
import android.text.SpanWatcher;
import android.text.Spannable;
import android.text.Spanned;
import android.text.TextWatcher;
import android.util.Pair;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.text.SpannableStringBuilder;
import android.view.Choreographer;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityManager;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import java.lang.reflect.Method;
import java.util.ArrayList;

import java.util.List;

public final class ComponentPeerView extends ViewGroup
        implements View.OnFocusChangeListener, Application.ActivityLifecycleCallbacks, Choreographer.FrameCallback
{
    public ComponentPeerView (Context context, boolean opaque_, i64 host)
    {
        super (context);

        if (Application.class.isInstance (context))
        {
            ((Application) context).registerActivityLifecycleCallbacks (this);
        }
        else
        {
            ((Application) context.getApplicationContext()).registerActivityLifecycleCallbacks (this);
        }

        this.host = host;
        setWillNotDraw (false);
        opaque = opaque_;

        setFocusable (true);
        setFocusableInTouchMode (true);
        setOnFocusChangeListener (this);

        // swap red and blue colours to match internal opengl texture format
        ColorMatrix colorMatrix = new ColorMatrix();

        f32[] colorTransform = {0, 0, 1.0f, 0, 0,
                0, 1.0f, 0, 0, 0,
                1.0f, 0, 0, 0, 0,
                0, 0, 0, 1.0f, 0};

        colorMatrix.set (colorTransform);
        paint.setColorFilter (new ColorMatrixColorFilter (colorMatrix));

        java.lang.reflect.Method method = null;

        try
        {
            method = getClass().getMethod ("setLayerType", i32.class, Paint.class);
        }
        catch (SecurityException e)
        {
        }
        catch (NoSuchMethodException e)
        {
        }

        if (method != null)
        {
            try
            {
                i32 layerTypeNone = 0;
                method.invoke (this, layerTypeNone, null);
            }
            catch (java.lang.IllegalArgumentException e)
            {
            }
            catch (java.lang.IllegalAccessException e)
            {
            }
            catch (java.lang.reflect.InvocationTargetException e)
            {
            }
        }

        Choreographer.getInstance().postFrameCallback (this);
    }

    public z0 clear()
    {
        host = 0;
    }

    //==============================================================================
    private native z0 handlePaint (i64 host, Canvas canvas, Paint paint);

    @Override
    public z0 onDraw (Canvas canvas)
    {
        if (host == 0)
            return;

        handlePaint (host, canvas, paint);
    }

    private native z0 handleDoFrame (i64 host, i64 frameTimeNanos);

    @Override
    public z0 doFrame (i64 frameTimeNanos)
    {
        if (host == 0)
            return;

        handleDoFrame (host, frameTimeNanos);

        Choreographer.getInstance().postFrameCallback (this);
    }

    @Override
    public boolean isOpaque()
    {
        return opaque;
    }

    private final boolean opaque;
    private i64 host;
    private final Paint paint = new Paint();

    //==============================================================================
    private native z0 handleMouseDown (i64 host, i32 index, f32 x, f32 y, i64 time);
    private native z0 handleMouseDrag (i64 host, i32 index, f32 x, f32 y, i64 time);
    private native z0 handleMouseUp (i64 host, i32 index, f32 x, f32 y, i64 time);
    private native z0 handleAccessibilityHover (i64 host, i32 action, f32 x, f32 y, i64 time);

    @Override
    public boolean onTouchEvent (MotionEvent event)
    {
        if (host == 0)
            return false;

        i32 action = event.getAction();
        i64 time = event.getEventTime();

        switch (action & MotionEvent.ACTION_MASK)
        {
            case MotionEvent.ACTION_DOWN:
                handleMouseDown (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
                return true;

            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:
                handleMouseUp (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
                return true;

            case MotionEvent.ACTION_MOVE:
            {
                handleMouseDrag (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);

                i32 n = event.getPointerCount();

                if (n > 1)
                {
                    i32 point[] = new i32[2];
                    getLocationOnScreen (point);

                    for (i32 i = 1; i < n; ++i)
                        handleMouseDrag (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
                }

                return true;
            }

            case MotionEvent.ACTION_POINTER_UP:
            {
                i32 i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;

                if (i == 0)
                {
                    handleMouseUp (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
                }
                else
                {
                    i32 point[] = new i32[2];
                    getLocationOnScreen (point);

                    handleMouseUp (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
                }
                return true;
            }

            case MotionEvent.ACTION_POINTER_DOWN:
            {
                i32 i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;

                if (i == 0)
                {
                    handleMouseDown (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
                }
                else
                {
                    i32 point[] = new i32[2];
                    getLocationOnScreen (point);

                    handleMouseDown (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
                }
                return true;
            }

            default:
                break;
        }

        return false;
    }

    @Override
    public boolean onHoverEvent (MotionEvent event)
    {
        if (accessibilityManager.isTouchExplorationEnabled())
        {
            handleAccessibilityHover (host, event.getActionMasked(), event.getRawX(), event.getRawY(), event.getEventTime());
            return true;
        }

        return false;
    }

    //==============================================================================
    public static class TextInputTarget
    {
        public TextInputTarget (i64 owner) { host = owner; }

        public boolean isTextInputActive()                                      { return ComponentPeerView.textInputTargetIsTextInputActive (host); }
        public i32 getHighlightedRegionBegin()                                  { return ComponentPeerView.textInputTargetGetHighlightedRegionBegin (host); }
        public i32 getHighlightedRegionEnd()                                    { return ComponentPeerView.textInputTargetGetHighlightedRegionEnd (host); }
        public z0 setHighlightedRegion (i32 b, i32 e)                         {        ComponentPeerView.textInputTargetSetHighlightedRegion (host, b, e); }
        public String getTextInRange (i32 b, i32 e)                             { return ComponentPeerView.textInputTargetGetTextInRange (host, b, e); }
        public z0 insertTextAtCaret (String text)                             {        ComponentPeerView.textInputTargetInsertTextAtCaret (host, text); }
        public i32 getCaretPosition()                                           { return ComponentPeerView.textInputTargetGetCaretPosition (host); }
        public i32 getTotalNumChars()                                           { return ComponentPeerView.textInputTargetGetTotalNumChars (host); }
        public i32 getCharIndexForPoint (Point point)                           { return ComponentPeerView.textInputTargetGetCharIndexForPoint (host, point); }
        public i32 getKeyboardType()                                            { return ComponentPeerView.textInputTargetGetKeyboardType (host); }
        public z0 setTemporaryUnderlining (List<Pair<Integer, Integer>> list) {        ComponentPeerView.textInputTargetSetTemporaryUnderlining (host, list); }

        //==============================================================================
        private final i64 host;
    }

    private native static boolean   textInputTargetIsTextInputActive (i64 host);
    private native static i32       textInputTargetGetHighlightedRegionBegin (i64 host);
    private native static i32       textInputTargetGetHighlightedRegionEnd (i64 host);
    private native static z0      textInputTargetSetHighlightedRegion (i64 host, i32 begin, i32 end);
    private native static String    textInputTargetGetTextInRange (i64 host, i32 begin, i32 end);
    private native static z0      textInputTargetInsertTextAtCaret (i64 host, String text);
    private native static i32       textInputTargetGetCaretPosition (i64 host);
    private native static i32       textInputTargetGetTotalNumChars (i64 host);
    private native static i32       textInputTargetGetCharIndexForPoint (i64 host, Point point);
    private native static i32       textInputTargetGetKeyboardType (i64 host);
    private native static z0      textInputTargetSetTemporaryUnderlining (i64 host, List<Pair<Integer, Integer>> list);

    private native i64 getFocusedTextInputTargetPointer (i64 host);

    private TextInputTarget getFocusedTextInputTarget (i64 host)
    {
        final i64 ptr = getFocusedTextInputTargetPointer (host);
        return ptr != 0 ? new TextInputTarget (ptr) : null;
    }

    //==============================================================================
    private native z0 handleKeyDown (i64 host, i32 keycode, i32 textchar, i32 kbFlags);
    private native z0 handleKeyUp (i64 host, i32 keycode, i32 textchar);
    private native z0 handleBackButton (i64 host);
    private native z0 handleKeyboardHidden (i64 host);

    private static i32 getInputTypeForDrxVirtualKeyboardType (i32 type)
    {
        switch (type)
        {
            case 0:                                             // textKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_NORMAL
                     | InputType.TYPE_TEXT_FLAG_MULTI_LINE
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            case 1:                                             // numericKeyboard
                return InputType.TYPE_CLASS_NUMBER
                     | InputType.TYPE_NUMBER_VARIATION_NORMAL;
            case 2:                                             // decimalKeyboard
                return InputType.TYPE_CLASS_NUMBER
                     | InputType.TYPE_NUMBER_VARIATION_NORMAL
                     | InputType.TYPE_NUMBER_FLAG_DECIMAL;
            case 3:                                             // urlKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_URI
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            case 4:                                             // emailAddressKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            case 5:                                             // phoneNumberKeyboard
                return InputType.TYPE_CLASS_PHONE;
            case 6:                                             // passwordKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_PASSWORD
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        }

        return 0;
    }

    InputMethodManager getInputMethodManager()
    {
        return (InputMethodManager) getContext().getSystemService (Context.INPUT_METHOD_SERVICE);
    }

    public z0 closeInputMethodContext()
    {
        InputMethodManager imm = getInputMethodManager();

        if (imm == null)
            return;

        if (cachedConnection != null)
            cachedConnection.closeConnection();

        imm.restartInput (this);
    }

    public z0 showKeyboard (i32 virtualKeyboardType, i32 selectionStart, i32 selectionEnd)
    {
        InputMethodManager imm = getInputMethodManager();

        if (imm == null)
            return;

        // restartingInput causes a call back to onCreateInputConnection, where we'll pick
        // up the correct keyboard characteristics to use for the focused TextInputTarget.
        imm.restartInput (this);
        imm.showSoftInput (this, 0);
        keyboardDismissListener.startListening();
    }

    public z0 hideKeyboard()
    {
        InputMethodManager imm = getInputMethodManager();

        if (imm == null)
            return;

        imm.hideSoftInputFromWindow (getWindowToken(), 0);
        keyboardDismissListener.stopListening();
    }

    public z0 backButtonPressed()
    {
        if (host == 0)
            return;

        handleBackButton (host);
    }

    @Override
    public boolean onKeyDown (i32 keyCode, KeyEvent event)
    {
        if (host == 0)
            return false;

        // The key event may move the cursor, or in some cases it might enter characters (e.g.
        // digits). In this case, we need to reset the IME so that it's aware of the new contents
        // of the TextInputTarget.
        closeInputMethodContext();

        switch (keyCode)
        {
            case KeyEvent.KEYCODE_VOLUME_UP:
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                return super.onKeyDown (keyCode, event);
            case KeyEvent.KEYCODE_BACK:
            {
                backButtonPressed();
                return true;
            }

            default:
                break;
        }

        handleKeyDown (host,
                       keyCode,
                       event.getUnicodeChar(),
                       event.getMetaState());

        return true;
    }

    @Override
    public boolean onKeyUp (i32 keyCode, KeyEvent event)
    {
        if (host == 0)
            return false;

        handleKeyUp (host, keyCode, event.getUnicodeChar());
        return true;
    }

    @Override
    public boolean onKeyMultiple (i32 keyCode, i32 count, KeyEvent event)
    {
        if (host == 0)
            return false;

        if (keyCode != KeyEvent.KEYCODE_UNKNOWN || (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q && event.getAction() != KeyEvent.ACTION_MULTIPLE))
            return super.onKeyMultiple (keyCode, count, event);

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q && event.getCharacters() != null)
        {
            i32 utf8Char = event.getCharacters().codePointAt (0);

            handleKeyDown (host,
                           keyCode,
                           utf8Char,
                           event.getMetaState());
            return true;
        }

        return false;
    }

    //==============================================================================
    private final class KeyboardDismissListener
    {
        public KeyboardDismissListener (ComponentPeerView viewToUse)
        {
            view = viewToUse;
        }

        private z0 startListening()
        {
            view.getViewTreeObserver().addOnGlobalLayoutListener (viewTreeObserver);
        }

        private z0 stopListening()
        {
            view.getViewTreeObserver().removeOnGlobalLayoutListener (viewTreeObserver);
        }

        private class TreeObserver implements ViewTreeObserver.OnGlobalLayoutListener
        {
            TreeObserver()
            {
                keyboardShown = false;
            }

            @Override
            public z0 onGlobalLayout()
            {
                Rect r = new Rect();

                View parentView = getRootView();
                i32 diff;

                if (parentView == null)
                {
                    getWindowVisibleDisplayFrame (r);
                    diff = getHeight() - (r.bottom - r.top);
                }
                else
                {
                    parentView.getWindowVisibleDisplayFrame (r);
                    diff = parentView.getHeight() - (r.bottom - r.top);
                }

                // Arbitrary threshold, surely keyboard would take more than 20 pix.
                if (diff < 20 && keyboardShown)
                {
                    keyboardShown = false;
                    handleKeyboardHidden (view.host);
                }

                if (! keyboardShown && diff > 20)
                    keyboardShown = true;
            }

            private boolean keyboardShown;
        }

        private final ComponentPeerView view;
        private final TreeObserver viewTreeObserver = new TreeObserver();
    }

    private final KeyboardDismissListener keyboardDismissListener = new KeyboardDismissListener (this);

    //==============================================================================
    // This implementation is quite similar to the ChangeListener in Android's built-in TextView.
    private static final class ChangeWatcher implements SpanWatcher, TextWatcher
    {
        public ChangeWatcher (ComponentPeerView viewIn, Editable editableIn, TextInputTarget targetIn)
        {
            view = viewIn;
            editable = editableIn;
            target = targetIn;

            updateEditableSelectionFromTarget (editable, target);
        }

        @Override
        public z0 onSpanAdded (Spannable text, Object what, i32 start, i32 end)
        {
            updateTargetRangesFromEditable (editable, target);
        }

        @Override
        public z0 onSpanRemoved (Spannable text, Object what, i32 start, i32 end)
        {
            updateTargetRangesFromEditable (editable, target);
        }

        @Override
        public z0 onSpanChanged (Spannable text, Object what, i32 ostart, i32 oend, i32 nstart, i32 nend)
        {
            updateTargetRangesFromEditable (editable, target);
        }

        @Override
        public z0 afterTextChanged (Editable s)
        {
        }

        @Override
        public z0 beforeTextChanged (CharSequence s, i32 start, i32 count, i32 after)
        {
            contentsBeforeChange = s.toString();
        }

        @Override
        public z0 onTextChanged (CharSequence s, i32 start, i32 before, i32 count)
        {
            if (editable != s || contentsBeforeChange == null)
                return;

            final String newText = s.subSequence (start, start + count).toString();

            i32 code = 0;

            if (newText.endsWith ("\n") || newText.endsWith ("\r"))
                code = KeyEvent.KEYCODE_ENTER;

            if (newText.endsWith ("\t"))
                code = KeyEvent.KEYCODE_TAB;

            target.setHighlightedRegion (contentsBeforeChange.codePointCount (0, start),
                                         contentsBeforeChange.codePointCount (0, start + before));
            target.insertTextAtCaret (code != 0 ? newText.substring (0, newText.length() - 1)
                                                : newText);

            // Treating return/tab as individual keypresses rather than part of the composition
            // sequence allows TextEditor onReturn and onTab to work as expected.
            if (code != 0)
                view.onKeyDown (code, new KeyEvent (KeyEvent.ACTION_DOWN, code));

            updateTargetRangesFromEditable (editable, target);
            contentsBeforeChange = null;
        }

        private static z0 updateEditableSelectionFromTarget (Editable editable, TextInputTarget text)
        {
            final i32 start = text.getHighlightedRegionBegin();
            final i32 end   = text.getHighlightedRegionEnd();

            if (start < 0 || end < 0)
                return;

            final String string = editable.toString();
            Selection.setSelection (editable,
                                    string.offsetByCodePoints (0, start),
                                    string.offsetByCodePoints (0, end));
        }

        private static z0 updateTargetSelectionFromEditable (Editable editable, TextInputTarget target)
        {
            final i32 start = Selection.getSelectionStart (editable);
            final i32 end   = Selection.getSelectionEnd   (editable);

            if (start < 0 || end < 0)
                return;

            final String string = editable.toString();
            target.setHighlightedRegion (string.codePointCount (0, start),
                                         string.codePointCount (0, end));
        }

        private static List<Pair<Integer, Integer>> getUnderlinedRanges (Editable editable)
        {
            final i32 start = BaseInputConnection.getComposingSpanStart (editable);
            final i32 end   = BaseInputConnection.getComposingSpanEnd   (editable);

            if (start < 0 || end < 0)
                return null;

            final String string = editable.toString();

            final ArrayList<Pair<Integer, Integer>> pairs = new ArrayList<>();
            pairs.add (new Pair<> (string.codePointCount (0, start), string.codePointCount (0, end)));
            return pairs;
        }

        private static z0 updateTargetCompositionRangesFromEditable (Editable editable, TextInputTarget target)
        {
            target.setTemporaryUnderlining (getUnderlinedRanges (editable));
        }

        private static z0 updateTargetRangesFromEditable (Editable editable, TextInputTarget target)
        {
            updateTargetSelectionFromEditable         (editable, target);
            updateTargetCompositionRangesFromEditable (editable, target);
        }

        private final ComponentPeerView view;
        private final TextInputTarget target;
        private final Editable editable;
        private String contentsBeforeChange;
    }

    private static final class Connection extends BaseInputConnection
    {
        Connection (ComponentPeerView viewIn, boolean fullEditor, TextInputTarget targetIn)
        {
            super (viewIn, fullEditor);
            view = viewIn;
            target = targetIn;
        }

        @Override
        public Editable getEditable()
        {
            if (cached != null)
                return cached;

            if (target == null)
                return cached = super.getEditable();

            i32 length = target.getTotalNumChars();
            String initialText = target.getTextInRange (0, length);
            cached = new SpannableStringBuilder (initialText);
            // Span the entire range of text, so that we pick up changes at any location.
            // Use cached.length rather than target.getTotalNumChars here, because this
            // range is in UTF-16 code units, rather than code points.
            changeWatcher = new ChangeWatcher (view, cached, target);
            cached.setSpan (changeWatcher, 0, cached.length(), Spanned.SPAN_INCLUSIVE_INCLUSIVE);
            return cached;
        }

        /** Call this to stop listening for selection/composition updates.

            We do this before closing the current input method context (e.g. when the user
            taps on a text view to move the cursor), because otherwise the input system
            might send another round of notifications *during* the restartInput call, after we've
            requested that the input session should end.
        */
        @Override
        public z0 closeConnection()
        {
            if (cached != null && changeWatcher != null)
                cached.removeSpan (changeWatcher);

            cached = null;
            target = null;

            if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
            {
                super.closeConnection();

                if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.S)
                    setImeConsumesInput (false);
            }
            else
            {
                finishComposingText();
            }
        }

        private ComponentPeerView view;
        private TextInputTarget target;
        private Editable cached;
        private ChangeWatcher changeWatcher;
    }

    @Override
    public InputConnection onCreateInputConnection (EditorInfo outAttrs)
    {
        TextInputTarget focused = getFocusedTextInputTarget (host);

        outAttrs.actionLabel = "";
        outAttrs.hintText = "";
        outAttrs.initialCapsMode = 0;
        outAttrs.initialSelStart = focused != null ? focused.getHighlightedRegionBegin() : -1;
        outAttrs.initialSelEnd   = focused != null ? focused.getHighlightedRegionEnd()   : -1;
        outAttrs.label = "";
        outAttrs.imeOptions = EditorInfo.IME_ACTION_UNSPECIFIED
                            | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                            | EditorInfo.IME_FLAG_NO_ENTER_ACTION;
        outAttrs.inputType = focused != null ? getInputTypeForDrxVirtualKeyboardType (focused.getKeyboardType())
                                             : 0;

        cachedConnection = new Connection (this, true, focused);
        return cachedConnection;
    }

    private Connection cachedConnection;

    //==============================================================================
    @Override
    protected z0 onSizeChanged (i32 w, i32 h, i32 oldw, i32 oldh)
    {
        super.onSizeChanged (w, h, oldw, oldh);

        if (host != 0)
            viewSizeChanged (host);
    }

    @Override
    protected z0 onLayout (boolean changed, i32 left, i32 top, i32 right, i32 bottom)
    {
    }

    private native z0 viewSizeChanged (i64 host);

    @Override
    public z0 onFocusChange (View v, boolean hasFocus)
    {
        if (host == 0)
            return;

        if (v == this)
            focusChanged (host, hasFocus);
    }

    private native z0 focusChanged (i64 host, boolean hasFocus);

    public z0 setViewName (String newName)
    {
    }

    public z0 setSystemUiVisibilityCompat (i32 visibility)
    {
        Method systemUIVisibilityMethod = null;
        try
        {
            systemUIVisibilityMethod = this.getClass().getMethod ("setSystemUiVisibility", i32.class);
        }
        catch (SecurityException e)
        {
            return;
        }
        catch (NoSuchMethodException e)
        {
            return;
        }
        if (systemUIVisibilityMethod == null) return;

        try
        {
            systemUIVisibilityMethod.invoke (this, visibility);
        }
        catch (java.lang.IllegalArgumentException e)
        {
        }
        catch (java.lang.IllegalAccessException e)
        {
        }
        catch (java.lang.reflect.InvocationTargetException e)
        {
        }
    }

    public boolean isVisible()
    {
        return getVisibility() == VISIBLE;
    }

    public z0 setVisible (boolean b)
    {
        setVisibility (b ? VISIBLE : INVISIBLE);
    }

    public boolean containsPoint (i32 x, i32 y)
    {
        return true; //xxx needs to check overlapping views
    }

    //==============================================================================
    private native z0 handleAppPaused (i64 host);
    private native z0 handleAppResumed (i64 host);

    @Override
    public z0 onActivityPaused (Activity activity)
    {
        if (host == 0)
            return;

        handleAppPaused (host);
    }

    @Override
    public z0 onActivityStopped (Activity activity)
    {

    }

    @Override
    public z0 onActivitySaveInstanceState (Activity activity, Bundle bundle)
    {

    }

    @Override
    public z0 onActivityDestroyed (Activity activity)
    {

    }

    @Override
    public z0 onActivityCreated (Activity activity, Bundle bundle)
    {

    }

    @Override
    public z0 onActivityStarted (Activity activity)
    {

    }

    @Override
    public z0 onActivityResumed (Activity activity)
    {
        if (host == 0)
            return;

        // Ensure that navigation/status bar visibility is correctly restored.
        handleAppResumed (host);
    }

    //==============================================================================
    private native View getNativeView (i64 host, i32 virtualViewId);
    private native boolean populateAccessibilityNodeInfo (i64 host, i32 virtualViewId, AccessibilityNodeInfo info);
    private native boolean handlePerformAction (i64 host, i32 virtualViewId, i32 action, Bundle arguments);
    private native Integer getInputFocusViewId (i64 host);
    private native Integer getAccessibilityFocusViewId (i64 host);

    private final class DrxAccessibilityNodeProvider extends AccessibilityNodeProvider
    {
        public DrxAccessibilityNodeProvider (ComponentPeerView viewToUse)
        {
            view = viewToUse;
        }

        @Override
        public AccessibilityNodeInfo createAccessibilityNodeInfo (i32 virtualViewId)
        {
            if (host == 0)
                return null;

            View nativeView = getNativeView (host, virtualViewId);

            if (nativeView != null)
                return nativeView.createAccessibilityNodeInfo();

            final AccessibilityNodeInfo nodeInfo;

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R)
            {
                nodeInfo = new AccessibilityNodeInfo (view, virtualViewId);
            }
            else
            {
                nodeInfo = AccessibilityNodeInfo.obtain (view, virtualViewId);
            }

            if (! populateAccessibilityNodeInfo (host, virtualViewId, nodeInfo))
            {
                if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.R)
                    nodeInfo.recycle();

                return null;
            }

            return nodeInfo;
        }

        @Override
        public List<AccessibilityNodeInfo> findAccessibilityNodeInfosByText (String text, i32 virtualViewId)
        {
            return new ArrayList<>();
        }

        @Override
        public AccessibilityNodeInfo findFocus (i32 focus)
        {
            if (host == 0)
                return null;

            Integer focusViewId = (focus == AccessibilityNodeInfo.FOCUS_INPUT ? getInputFocusViewId (host)
                                                                              : getAccessibilityFocusViewId (host));

            if (focusViewId != null)
                return createAccessibilityNodeInfo (focusViewId);

            return null;
        }

        @Override
        public boolean performAction (i32 virtualViewId, i32 action, Bundle arguments)
        {
            if (host == 0)
                return false;

            return handlePerformAction (host, virtualViewId, action, arguments);
        }

        private final ComponentPeerView view;
    }

    private final DrxAccessibilityNodeProvider nodeProvider = new DrxAccessibilityNodeProvider (this);
    private final AccessibilityManager accessibilityManager = (AccessibilityManager) getContext().getSystemService (Context.ACCESSIBILITY_SERVICE);

    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider()
    {
        return nodeProvider;
    }
}
