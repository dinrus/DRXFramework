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

class CodeDocumentLine;


//==============================================================================
/**
    A class for storing and manipulating a source code file.

    When using a CodeEditorComponent, it takes one of these as its source object.

    The CodeDocument stores its content as an array of lines, which makes it
    quick to insert and delete.

    @see CodeEditorComponent

    @tags{GUI}
*/
class DRX_API  CodeDocument
{
public:
    /** Creates a new, empty document. */
    CodeDocument();

    /** Destructor. */
    ~CodeDocument();

    //==============================================================================
    /** A position in a code document.

        Using this class you can find a position in a code document and quickly get its
        character position, line, and index. By calling setPositionMaintained (true), the
        position is automatically updated when text is inserted or deleted in the document,
        so that it maintains its original place in the text.
    */
    class DRX_API  Position
    {
    public:
        /** Creates an uninitialised position.
            Don't attempt to call any methods on this until you've given it an owner document
            to refer to!
        */
        Position() noexcept;

        /** Creates a position based on a line and index in a document.

            Note that this index is NOT the column number, it's the number of characters from the
            start of the line. The "column" number isn't quite the same, because if the line
            contains any tab characters, the relationship of the index to its visual column depends on
            the number of spaces per tab being used!

            Lines are numbered from zero, and if the line or index are beyond the bounds of the document,
            they will be adjusted to keep them within its limits.
        */
        Position (const CodeDocument& ownerDocument,
                  i32 line, i32 indexInLine) noexcept;

        /** Creates a position based on a character index in a document.
            This position is placed at the specified number of characters from the start of the
            document. The line and column are auto-calculated.

            If the position is beyond the range of the document, it'll be adjusted to keep it
            inside.
        */
        Position (const CodeDocument& ownerDocument,
                  i32 charactersFromStartOfDocument) noexcept;

        /** Creates a copy of another position.

            This will copy the position, but the new object will not be set to maintain its position,
            even if the source object was set to do so.
        */
        Position (const Position&) noexcept;

        /** Destructor. */
        ~Position();

        Position& operator= (const Position&);

        b8 operator== (const Position&) const noexcept;
        b8 operator!= (const Position&) const noexcept;

        /** Points this object at a new position within the document.

            If the position is beyond the range of the document, it'll be adjusted to keep it
            inside.
            @see getPosition, setLineAndIndex
        */
        z0 setPosition (i32 charactersFromStartOfDocument);

        /** Returns the position as the number of characters from the start of the document.
            @see setPosition, getLineNumber, getIndexInLine
        */
        i32 getPosition() const noexcept            { return characterPos; }

        /** Moves the position to a new line and index within the line.

            Note that the index is NOT the column at which the position appears in an editor.
            If the line contains any tab characters, the relationship of the index to its
            visual position depends on the number of spaces per tab being used!

            Lines are numbered from zero, and if the line or index are beyond the bounds of the document,
            they will be adjusted to keep them within its limits.
        */
        z0 setLineAndIndex (i32 newLineNumber, i32 newIndexInLine);

        /** Returns the line number of this position.
            The first line in the document is numbered zero, not one!
        */
        i32 getLineNumber() const noexcept          { return line; }

        /** Returns the number of characters from the start of the line.

            Note that this value is NOT the column at which the position appears in an editor.
            If the line contains any tab characters, the relationship of the index to its
            visual position depends on the number of spaces per tab being used!
        */
        i32 getIndexInLine() const noexcept         { return indexInLine; }

        /** Allows the position to be automatically updated when the document changes.

            If this is set to true, the position will register with its document so that
            when the document has text inserted or deleted, this position will be automatically
            moved to keep it at the same position in the text.
        */
        z0 setPositionMaintained (b8 isMaintained);

        //==============================================================================
        /** Moves the position forwards or backwards by the specified number of characters.
            @see movedBy
        */
        z0 moveBy (i32 characterDelta);

        /** Returns a position which is the same as this one, moved by the specified number of
            characters.
            @see moveBy
        */
        Position movedBy (i32 characterDelta) const;

        /** Returns a position which is the same as this one, moved up or down by the specified
            number of lines.
            @see movedBy
        */
        Position movedByLines (i32 deltaLines) const;

        /** Returns the character in the document at this position.
            @see getLineText
        */
        t32 getCharacter() const;

        /** Returns the line from the document that this position is within.
            @see getCharacter, getLineNumber
        */
        Txt getLineText() const;

    private:
        CodeDocument* owner = nullptr;
        i32 characterPos = 0, line = 0, indexInLine = 0;
        b8 positionMaintained = false;

        friend class CodeDocument;
    };

    //==============================================================================
    /** Returns the full text of the document. */
    Txt getAllContent() const;

    /** Returns a section of the document's text. */
    Txt getTextBetween (const Position& start, const Position& end) const;

    /** Returns a line from the document. */
    Txt getLine (i32 lineIndex) const noexcept;

    /** Returns the number of characters in the document. */
    i32 getNumCharacters() const noexcept;

    /** Returns the number of lines in the document. */
    i32 getNumLines() const noexcept                    { return lines.size(); }

    /** Returns the number of characters in the longest line of the document. */
    i32 getMaximumLineLength() noexcept;

    /** Deletes a section of the text.
        This operation is undoable.
    */
    z0 deleteSection (const Position& startPosition, const Position& endPosition);

    /** Deletes a section of the text.
        This operation is undoable.
    */
    z0 deleteSection (i32 startIndex, i32 endIndex);

    /** Inserts some text into the document at a given position.
        This operation is undoable.
    */
    z0 insertText (const Position& position, const Txt& text);

    /** Inserts some text into the document at a given position.
        This operation is undoable.
    */
    z0 insertText (i32 insertIndex, const Txt& text);

    /** Replaces a section of the text with a new string.
        This operation is undoable.
    */
    z0 replaceSection (i32 startIndex, i32 endIndex, const Txt& newText);

    /** Clears the document and replaces it with some new text.

        This operation is undoable - if you're trying to completely reset the document, you
        might want to also call clearUndoHistory() and setSavePoint() after using this method.
    */
    z0 replaceAllContent (const Txt& newContent);

    /** Analyses the changes between the current content and some new text, and applies
        those changes.
    */
    z0 applyChanges (const Txt& newContent);

    /** Replaces the editor's contents with the contents of a stream.
        This will also reset the undo history and save point marker.
    */
    b8 loadFromStream (InputStream& stream);

    /** Writes the editor's current contents to a stream. */
    b8 writeToStream (OutputStream& stream);

    //==============================================================================
    /** Returns the preferred new-line characters for the document.
        This will be either "\\n", "\\r\\n", or (rarely) "\\r".
        @see setNewLineCharacters
    */
    Txt getNewLineCharacters() const noexcept          { return newLineChars; }

    /** Sets the new-line characters that the document should use.
        The string must be either "\\n", "\\r\\n", or (rarely) "\\r".
        @see getNewLineCharacters
    */
    z0 setNewLineCharacters (const Txt& newLineCharacters) noexcept;

    //==============================================================================
    /** Begins a new undo transaction.

        The document itself will not call this internally, so relies on whatever is using the
        document to periodically call this to break up the undo sequence into sensible chunks.
        @see UndoManager::beginNewTransaction
    */
    z0 newTransaction();

    /** Undo the last operation.
        @see UndoManager::undo
    */
    z0 undo();

    /** Redo the last operation.
        @see UndoManager::redo
    */
    z0 redo();

    /** Clears the undo history.
        @see UndoManager::clearUndoHistory
    */
    z0 clearUndoHistory();

    /** Returns the document's UndoManager */
    UndoManager& getUndoManager() noexcept              { return undoManager; }

    //==============================================================================
    /** Makes a note that the document's current state matches the one that is saved.

        After this has been called, hasChangedSinceSavePoint() will return false until
        the document has been altered, and then it'll start returning true. If the document is
        altered, but then undone until it gets back to this state, hasChangedSinceSavePoint()
        will again return false.

        @see hasChangedSinceSavePoint
    */
    z0 setSavePoint() noexcept;

    /** Возвращает true, если the state of the document differs from the state it was in when
        setSavePoint() was last called.

        @see setSavePoint
    */
    b8 hasChangedSinceSavePoint() const noexcept;

    //==============================================================================
    /** Searches for a word-break. */
    Position findWordBreakAfter (const Position& position) const noexcept;
    /** Searches for a word-break. */
    Position findWordBreakBefore (const Position& position) const noexcept;
    /** Finds the token that contains the given position. */
    z0 findTokenContaining (const Position& pos, Position& start, Position& end) const noexcept;
    /** Finds the line that contains the given position. */
    z0 findLineContaining  (const Position& pos, Position& start, Position& end) const noexcept;

    //==============================================================================
    /** An object that receives callbacks from the CodeDocument when its text changes.
        @see CodeDocument::addListener, CodeDocument::removeListener
    */
    class DRX_API  Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;

        /** Called by a CodeDocument when text is added. */
        virtual z0 codeDocumentTextInserted (const Txt& newText, i32 insertIndex) = 0;

        /** Called by a CodeDocument when text is deleted. */
        virtual z0 codeDocumentTextDeleted (i32 startIndex, i32 endIndex) = 0;
    };

    /** Registers a listener object to receive callbacks when the document changes.
        If the listener is already registered, this method has no effect.
        @see removeListener
    */
    z0 addListener (Listener* listener);

    /** Deregisters a listener.
        @see addListener
    */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** Iterates the text in a CodeDocument.

        This class lets you read characters from a CodeDocument. It's designed to be used
        by a CodeTokeniser object.

        @see CodeDocument
    */
    class DRX_API  Iterator
    {
    public:
        /** Creates an uninitialised iterator.
            Don't attempt to call any methods on this until you've given it an
            owner document to refer to!
         */
        Iterator() noexcept;

        Iterator (const CodeDocument& document) noexcept;
        Iterator (CodeDocument::Position) noexcept;
        ~Iterator() noexcept;

        Iterator (const Iterator&) = default;
        Iterator& operator= (const Iterator&) = default;

        /** Reads the next character and returns it. Returns 0 if you try to
            read past the document's end.
            @see peekNextChar, previousChar
        */
        t32 nextChar() noexcept;

        /** Reads the next character without moving the current position. */
        t32 peekNextChar() const noexcept;

        /** Reads the previous character and returns it. Returns 0 if you try to
            read past the document's start.
            @see isSOF, peekPreviousChar, nextChar
         */
        t32 previousChar() noexcept;

        /** Reads the next character without moving the current position. */
        t32 peekPreviousChar() const noexcept;

        /** Advances the position by one character. */
        z0 skip() noexcept;

        /** Returns the position as the number of characters from the start of the document. */
        i32 getPosition() const noexcept        { return position; }

        /** Skips over any whitespace characters until the next character is non-whitespace. */
        z0 skipWhitespace() noexcept;

        /** Skips forward until the next character will be the first character on the next line */
        z0 skipToEndOfLine() noexcept;

        /** Skips backward until the next character will be the first character on this line */
        z0 skipToStartOfLine() noexcept;

        /** Returns the line number of the next character. */
        i32 getLine() const noexcept            { return line; }

        /** Возвращает true, если the iterator has reached the end of the document. */
        b8 isEOF() const noexcept;

        /** Возвращает true, если the iterator is at the start of the document. */
        b8 isSOF() const noexcept;

        /** Convert this iterator to a CodeDocument::Position. */
        CodeDocument::Position toPosition() const;

    private:
        b8 reinitialiseCharPtr() const;

        const CodeDocument* document;
        mutable Txt::CharPointerType charPointer { nullptr };
        i32 line = 0, position = 0;
    };

private:
    //==============================================================================
    struct InsertAction;
    struct DeleteAction;
    friend class Iterator;
    friend class Position;

    OwnedArray<CodeDocumentLine> lines;
    Array<Position*> positionsToMaintain;
    UndoManager undoManager;
    i32 currentActionIndex = 0, indexOfSavedState = -1;
    i32 maximumLineLength = -1;
    ListenerList<Listener> listeners;
    Txt newLineChars { "\r\n" };

    z0 insert (const Txt& text, i32 insertPos, b8 undoable);
    z0 remove (i32 startPos, i32 endPos, b8 undoable);
    z0 checkLastLineStatus();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeDocument)
};

} // namespace drx
