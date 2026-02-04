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

//==============================================================================
/** Used to build a tree of elements representing an XML document.

    An XML document can be parsed into a tree of XmlElements, each of which
    represents an XML tag structure, and which may itself contain other
    nested elements.

    An XmlElement can also be converted back into a text document, and has
    lots of useful methods for manipulating its attributes and sub-elements,
    so XmlElements can actually be used as a handy general-purpose data
    structure.

    Here's an example of parsing some elements: @code
    // check we're looking at the right kind of document..
    if (myElement->hasTagName ("ANIMALS"))
    {
        // now we'll iterate its sub-elements looking for 'giraffe' elements..
        for (auto* e : myElement->getChildIterator())
        {
            if (e->hasTagName ("GIRAFFE"))
            {
                // found a giraffe, so use some of its attributes..

                Txt giraffeName  = e->getStringAttribute ("name");
                i32 giraffeAge      = e->getIntAttribute ("age");
                b8 isFriendly     = e->getBoolAttribute ("friendly");
            }
        }
    }
    @endcode

    And here's an example of how to create an XML document from scratch: @code
    // create an outer node called "ANIMALS"
    XmlElement animalsList ("ANIMALS");

    for (i32 i = 0; i < numAnimals; ++i)
    {
        // create an inner element..
        XmlElement* giraffe = new XmlElement ("GIRAFFE");

        giraffe->setAttribute ("name", "nigel");
        giraffe->setAttribute ("age", 10);
        giraffe->setAttribute ("friendly", true);

        // ..and add our new element to the parent node
        animalsList.addChildElement (giraffe);
    }

    // now we can turn the whole thing into textual XML
    auto xmlString = animalsList.toString();
    @endcode

    @see parseXML, parseXMLIfTagMatches, XmlDocument

    @tags{Core}
*/
class DRX_API  XmlElement
{
public:
    //==============================================================================
    /** Creates an XmlElement with this tag name. */
    explicit XmlElement (const Txt& tagName);

    /** Creates an XmlElement with this tag name. */
    explicit XmlElement (tukk tagName);

    /** Creates an XmlElement with this tag name. */
    explicit XmlElement (const Identifier& tagName);

    /** Creates an XmlElement with this tag name. */
    explicit XmlElement (StringRef tagName);

    /** Creates an XmlElement with this tag name. */
    XmlElement (Txt::CharPointerType tagNameBegin, Txt::CharPointerType tagNameEnd);

    /** Creates a (deep) copy of another element. */
    XmlElement (const XmlElement&);

    /** Creates a (deep) copy of another element. */
    XmlElement& operator= (const XmlElement&);

    /** Move assignment operator */
    XmlElement& operator= (XmlElement&&) noexcept;

    /** Move constructor */
    XmlElement (XmlElement&&) noexcept;

    /** Deleting an XmlElement will also delete all of its child elements. */
    ~XmlElement() noexcept;

    //==============================================================================
    /** Compares two XmlElements to see if they contain the same text and attributes.

        The elements are only considered equivalent if they contain the same attributes
        with the same values, and have the same sub-nodes.

        @param other                    the other element to compare to
        @param ignoreOrderOfAttributes  if true, this means that two elements with the
                                        same attributes in a different order will be
                                        considered the same; if false, the attributes must
                                        be in the same order as well
    */
    b8 isEquivalentTo (const XmlElement* other,
                         b8 ignoreOrderOfAttributes) const noexcept;

    //==============================================================================
    /** A struct containing options for formatting the text when representing an
        XML element as a string.
    */
    struct DRX_API TextFormat
    {
        /** Default constructor. */
        TextFormat();

        Txt dtd;                        /**< If supplied, this DTD will be added to the document. */
        Txt customHeader;               /**< If supplied, this header will be used (and customEncoding & addDefaultHeader will be ignored). */
        Txt customEncoding;             /**< If not empty and addDefaultHeader is true, this will be set as the encoding. Otherwise, a default of "UTF-8" will be used */
        b8 addDefaultHeader = true;      /**< If true, a default header will be generated; otherwise just bare XML will be emitted. */
        i32 lineWrapLength = 60;           /**< A maximum line length before wrapping is done. (If newLineChars is nullptr, this is ignored) */
        tukk newLineChars = "\r\n"; /**< Allows the newline characters to be set. If you set this to nullptr, then the whole XML document will be placed on a single line. */

        [[nodiscard]] TextFormat singleLine() const;     /**< returns a copy of this format with newLineChars set to nullptr. */
        [[nodiscard]] TextFormat withoutHeader() const;  /**< returns a copy of this format with the addDefaultHeader flag set to false. */
    };

    /** Returns a text version of this XML element.
        If your intention is to write the XML to a file or stream, it's probably more efficient to
        use writeTo() instead of creating an intermediate string.
        @see writeTo
    */
    Txt toString (const TextFormat& format = {}) const;

    /** Writes the document to a stream as UTF-8.
        @see writeTo, toString
    */
    z0 writeTo (OutputStream& output, const TextFormat& format = {}) const;

    /** Writes the document to a file as UTF-8.
        @see writeTo, toString
    */
    b8 writeTo (const File& destinationFile, const TextFormat& format = {}) const;

    //==============================================================================
    /** Returns this element's tag type name.
        E.g. for an element such as \<MOOSE legs="4" antlers="2">, this would return "MOOSE".
        @see hasTagName
    */
    const Txt& getTagName() const noexcept            { return tagName; }

    /** Returns the namespace portion of the tag-name, or an empty string if none is specified. */
    Txt getNamespace() const;

    /** Returns the part of the tag-name that follows any namespace declaration. */
    Txt getTagNameWithoutNamespace() const;

    /** Tests whether this element has a particular tag name.
        @param possibleTagName  the tag name you're comparing it with
        @see getTagName
    */
    b8 hasTagName (StringRef possibleTagName) const noexcept;

    /** Tests whether this element has a particular tag name, ignoring any XML namespace prefix.
        So a test for e.g. "xyz" will return true for "xyz" and also "foo:xyz", "bar::xyz", etc.
        @see getTagName
    */
    b8 hasTagNameIgnoringNamespace (StringRef possibleTagName) const;

    /** Changes this elements tag name.
        @see getTagName
     */
    z0 setTagName (StringRef newTagName);

    //==============================================================================
    /** Returns the number of XML attributes this element contains.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, this would
        return 2.
    */
    i32 getNumAttributes() const noexcept;

    /** Returns the name of one of the elements attributes.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, then
        getAttributeName (1) would return "antlers".

        @see getAttributeValue, getStringAttribute
    */
    const Txt& getAttributeName (i32 attributeIndex) const noexcept;

    /** Returns the value of one of the elements attributes.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, then
        getAttributeName (1) would return "2".

        @see getAttributeName, getStringAttribute
    */
    const Txt& getAttributeValue (i32 attributeIndex) const noexcept;

    //==============================================================================
    // Attribute-handling methods..

    /** Checks whether the element contains an attribute with a certain name. */
    b8 hasAttribute (StringRef attributeName) const noexcept;

    /** Returns the value of a named attribute.
        @param attributeName        the name of the attribute to look up
    */
    const Txt& getStringAttribute (StringRef attributeName) const noexcept;

    /** Returns the value of a named attribute.
        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
    */
    Txt getStringAttribute (StringRef attributeName, const Txt& defaultReturnValue) const;

    /** Compares the value of a named attribute with a value passed-in.

        @param attributeName            the name of the attribute to look up
        @param stringToCompareAgainst   the value to compare it with
        @param ignoreCase               whether the comparison should be case-insensitive
        @returns    true if the value of the attribute is the same as the string passed-in;
                    false if it's different (or if no such attribute exists)
    */
    b8 compareAttribute (StringRef attributeName,
                           StringRef stringToCompareAgainst,
                           b8 ignoreCase = false) const noexcept;

    /** Returns the value of a named attribute as an integer.

        This will try to find the attribute and convert it to an integer (using
        the Txt::getIntValue() method).

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
        @see setAttribute
    */
    i32 getIntAttribute (StringRef attributeName, i32 defaultReturnValue = 0) const;

    /** Returns the value of a named attribute as floating-point.

        This will try to find the attribute and convert it to a f64 (using
        the Txt::getDoubleValue() method).

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
        @see setAttribute
    */
    f64 getDoubleAttribute (StringRef attributeName, f64 defaultReturnValue = 0.0) const;

    /** Returns the value of a named attribute as a boolean.

        This will try to find the attribute and interpret it as a boolean. To do this,
        it'll return true if the value is "1", "true", "y", etc, or false for other
        values.

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
    */
    b8 getBoolAttribute (StringRef attributeName, b8 defaultReturnValue = false) const;

    /** Adds a named attribute to the element.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
        @see removeAttribute
    */
    z0 setAttribute (const Identifier& attributeName, const Txt& newValue);

    /** Adds a named attribute to the element, setting it to an integer value.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
    */
    z0 setAttribute (const Identifier& attributeName, i32 newValue);

    /** Adds a named attribute to the element, setting it to a floating-point value.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
    */
    z0 setAttribute (const Identifier& attributeName, f64 newValue);

    /** Removes a named attribute from the element.

        @param attributeName    the name of the attribute to remove
        @see removeAllAttributes
    */
    z0 removeAttribute (const Identifier& attributeName) noexcept;

    /** Removes all attributes from this element. */
    z0 removeAllAttributes() noexcept;

    //==============================================================================
    // Child element methods..

    /** Returns the first of this element's sub-elements.
        see getNextElement() for an example of how to iterate the sub-elements.

        @see getChildIterator
    */
    XmlElement* getFirstChildElement() const noexcept       { return firstChildElement; }

    /** Returns the next of this element's siblings.

        This can be used for iterating an element's sub-elements, e.g.
        @code
        XmlElement* child = myXmlDocument->getFirstChildElement();

        while (child != nullptr)
        {
            ...do stuff with this child..

            child = child->getNextElement();
        }
        @endcode

        Note that when iterating the child elements, some of them might be
        text elements as well as XML tags - use isTextElement() to work this
        out.

        Also, it's much easier and neater to use this method indirectly via the
        getChildIterator() method.

        @returns    the sibling element that follows this one, or a nullptr if
                    this is the last element in its parent

        @see getNextElement, isTextElement, getChildIterator
    */
    inline XmlElement* getNextElement() const noexcept          { return nextListItem; }

    /** Returns the next of this element's siblings which has the specified tag
        name.

        This is like getNextElement(), but will scan through the list until it
        finds an element with the given tag name.

        @see getNextElement, getChildIterator
    */
    XmlElement* getNextElementWithTagName (StringRef requiredTagName) const;

    /** Returns the number of sub-elements in this element.
        @see getChildElement
    */
    i32 getNumChildElements() const noexcept;

    /** Returns the sub-element at a certain index.

        It's not very efficient to iterate the sub-elements by index - see
        getNextElement() for an example of how best to iterate.

        @returns the n'th child of this element, or nullptr if the index is out-of-range
        @see getNextElement, isTextElement, getChildByName
    */
    XmlElement* getChildElement (i32 index) const noexcept;

    /** Returns the first sub-element with a given tag-name.

        @param tagNameToLookFor     the tag name of the element you want to find
        @returns the first element with this tag name, or nullptr if none is found
        @see getNextElement, isTextElement, getChildElement, getChildByAttribute
    */
    XmlElement* getChildByName (StringRef tagNameToLookFor) const noexcept;

    /** Returns the first sub-element which has an attribute that matches the given value.

        @param attributeName     the name of the attribute to check
        @param attributeValue    the target value of the attribute
        @returns the first element with this attribute value, or nullptr if none is found
        @see getChildByName
    */
    XmlElement* getChildByAttribute (StringRef attributeName,
                                     StringRef attributeValue) const noexcept;

    //==============================================================================
    /** Appends an element to this element's list of children.

        Child elements are deleted automatically when their parent is deleted, so
        make sure the object that you pass in will not be deleted by anything else,
        and make sure it's not already the child of another element.

        Note that due to the XmlElement using a singly-linked-list, prependChildElement()
        is an O(1) operation, but addChildElement() is an O(N) operation - so if
        you're adding large number of elements, you may prefer to do so in reverse order!

        @see getFirstChildElement, getNextElement, getNumChildElements,
             getChildElement, removeChildElement
    */
    z0 addChildElement (XmlElement* newChildElement) noexcept;

    /** Inserts an element into this element's list of children.

        Child elements are deleted automatically when their parent is deleted, so
        make sure the object that you pass in will not be deleted by anything else,
        and make sure it's not already the child of another element.

        @param newChildElement  the element to add
        @param indexToInsertAt  the index at which to insert the new element - if this is
                                below zero, it will be added to the end of the list
        @see addChildElement, insertChildElement
    */
    z0 insertChildElement (XmlElement* newChildElement,
                             i32 indexToInsertAt) noexcept;

    /** Inserts an element at the beginning of this element's list of children.

        Child elements are deleted automatically when their parent is deleted, so
        make sure the object that you pass in will not be deleted by anything else,
        and make sure it's not already the child of another element.

        Note that due to the XmlElement using a singly-linked-list, prependChildElement()
        is an O(1) operation, but addChildElement() is an O(N) operation - so if
        you're adding large number of elements, you may prefer to do so in reverse order!

        @see addChildElement, insertChildElement
    */
    z0 prependChildElement (XmlElement* newChildElement) noexcept;

    /** Creates a new element with the given name and returns it, after adding it
        as a child element.

        This is a handy method that means that instead of writing this:
        @code
        XmlElement* newElement = new XmlElement ("foobar");
        myParentElement->addChildElement (newElement);
        @endcode

        ..you could just write this:
        @code
        XmlElement* newElement = myParentElement->createNewChildElement ("foobar");
        @endcode
    */
    XmlElement* createNewChildElement (StringRef tagName);

    /** Replaces one of this element's children with another node.

        If the current element passed-in isn't actually a child of this element,
        this will return false and the new one won't be added. Otherwise, the
        existing element will be deleted, replaced with the new one, and it
        will return true.
    */
    b8 replaceChildElement (XmlElement* currentChildElement,
                              XmlElement* newChildNode) noexcept;

    /** Removes a child element.

        @param childToRemove            the child to look for and remove
        @param shouldDeleteTheChild     if true, the child will be deleted, if false it'll
                                        just remove it
    */
    z0 removeChildElement (XmlElement* childToRemove,
                             b8 shouldDeleteTheChild) noexcept;

    /** Deletes all the child elements in the element.
        @see removeChildElement, deleteAllChildElementsWithTagName
    */
    z0 deleteAllChildElements() noexcept;

    /** Deletes all the child elements with a given tag name.
        @see removeChildElement
    */
    z0 deleteAllChildElementsWithTagName (StringRef tagName) noexcept;

    /** Возвращает true, если the given element is a child of this one. */
    b8 containsChildElement (const XmlElement* possibleChild) const noexcept;

    /** Recursively searches all sub-elements of this one, looking for an element
        which is the direct parent of the specified element.

        Because elements don't store a pointer to their parent, if you have one
        and need to find its parent, the only way to do so is to exhaustively
        search the whole tree for it.

        If the given child is found somewhere in this element's hierarchy, then
        this method will return its parent. If not, it will return nullptr.
    */
    XmlElement* findParentElementOf (const XmlElement* childToSearchFor) noexcept;

    //==============================================================================
    /** Sorts the child elements using a comparator.

        This will use a comparator object to sort the elements into order. The object
        passed must have a method of the form:
        @code
        i32 compareElements (const XmlElement* first, const XmlElement* second);
        @endcode

        ..and this method must return:
          - a value of < 0 if the first comes before the second
          - a value of 0 if the two objects are equivalent
          - a value of > 0 if the second comes before the first

        To improve performance, the compareElements() method can be declared as static or const.

        @param comparator   the comparator to use for comparing elements.
        @param retainOrderOfEquivalentItems     if this is true, then items which the comparator
                            says are equivalent will be kept in the order in which they
                            currently appear in the array. This is slower to perform, but
                            may be important in some cases. If it's false, a faster algorithm
                            is used, but equivalent elements may be rearranged.
    */
    template <class ElementComparator>
    z0 sortChildElements (ElementComparator& comparator,
                            b8 retainOrderOfEquivalentItems = false)
    {
        auto num = getNumChildElements();

        if (num > 1)
        {
            HeapBlock<XmlElement*> elems (num);
            getChildElementsAsArray (elems);
            sortArray (comparator, (XmlElement**) elems, 0, num - 1, retainOrderOfEquivalentItems);
            reorderChildElements (elems, num);
        }
    }

    //==============================================================================
    /** Возвращает true, если this element is a section of text.

        Elements can either be an XML tag element or a section of text, so this
        is used to find out what kind of element this one is.

        @see getAllText, addTextElement, deleteAllTextElements
    */
    b8 isTextElement() const noexcept;

    /** Returns the text for a text element.

        Note that if you have an element like this:

        @code<xyz>hello</xyz>@endcode

        then calling getText on the "xyz" element won't return "hello", because that is
        actually stored in a special text sub-element inside the xyz element. To get the
        "hello" string, you could either call getText on the (unnamed) sub-element, or
        use getAllSubText() to do this automatically.

        Note that leading and trailing whitespace will be included in the string - to remove
        if, just call Txt::trim() on the result.

        @see isTextElement, getAllSubText, getChildElementAllSubText
    */
    const Txt& getText() const noexcept;

    /** Sets the text in a text element.

        Note that this is only a valid call if this element is a text element. If it's
        not, then no action will be performed. If you're trying to add text inside a normal
        element, you probably want to use addTextElement() instead.
    */
    z0 setText (const Txt& newText);

    /** Returns all the text from this element's child nodes.

        This iterates all the child elements and when it finds text elements,
        it concatenates their text into a big string which it returns.

        E.g. @code<xyz>hello <x>there</x> world</xyz>@endcode
        if you called getAllSubText on the "xyz" element, it'd return "hello there world".

        Note that leading and trailing whitespace will be included in the string - to remove
        if, just call Txt::trim() on the result.

        @see isTextElement, getChildElementAllSubText, getText, addTextElement
    */
    Txt getAllSubText() const;

    /** Returns all the sub-text of a named child element.

        If there is a child element with the given tag name, this will return
        all of its sub-text (by calling getAllSubText() on it). If there is
        no such child element, this will return the default string passed-in.

        @see getAllSubText
    */
    Txt getChildElementAllSubText (StringRef childTagName,
                                      const Txt& defaultReturnValue) const;

    /** Appends a section of text to this element.
        @see isTextElement, getText, getAllSubText
    */
    z0 addTextElement (const Txt& text);

    /** Removes all the text elements from this element.
        @see isTextElement, getText, getAllSubText, addTextElement
    */
    z0 deleteAllTextElements() noexcept;

    /** Creates a text element that can be added to a parent element. */
    static XmlElement* createTextElement (const Txt& text);

    /** Checks if a given string is a valid XML name */
    static b8 isValidXmlName (StringRef possibleName) noexcept;

private:
    //==============================================================================
    struct GetNextElement
    {
        XmlElement* getNext (const XmlElement& e) const { return e.getNextElement(); }
    };

    struct GetNextElementWithTagName
    {
        GetNextElementWithTagName() = default;
        explicit GetNextElementWithTagName (Txt n) : name (std::move (n)) {}
        XmlElement* getNext (const XmlElement& e) const { return e.getNextElementWithTagName (name); }

        Txt name;
    };

    //==============================================================================
    template <typename Traits>
    class Iterator : private Traits
    {
    public:
        using difference_type   = ptrdiff_t;
        using value_type        = XmlElement*;
        using pointer           = const value_type*;
        using reference         = value_type;
        using iterator_category = std::input_iterator_tag;

        Iterator() = default;

        template <typename... Args>
        Iterator (XmlElement* e, Args&&... args)
            : Traits (std::forward<Args> (args)...), element (e) {}

        Iterator begin()    const { return *this; }
        Iterator end()      const { return Iterator{}; }

        b8 operator== (const Iterator& other) const { return element == other.element; }
        b8 operator!= (const Iterator& other) const { return ! operator== (other); }

        reference operator*()  const { return  element; }
        pointer   operator->() const { return &element; }

        Iterator& operator++()
        {
            element = Traits::getNext (*element);
            return *this;
        }

        Iterator operator++ (i32)
        {
            auto copy = *this;
            ++(*this);
            return copy;
        }

    private:
        value_type element = nullptr;
    };

public:
    //==============================================================================
    /** Allows iterating the children of an XmlElement using range-for syntax.

        @code
        z0 doSomethingWithXmlChildren (const XmlElement& myParentXml)
        {
            for (auto* element : myParentXml.getChildIterator())
                doSomethingWithXmlElement (element);
        }
        @endcode
    */
    Iterator<GetNextElement> getChildIterator() const
    {
        return Iterator<GetNextElement> { getFirstChildElement() };
    }

    /** Allows iterating children of an XmlElement with a specific tag using range-for syntax.

        @code
        z0 doSomethingWithXmlChildren (const XmlElement& myParentXml)
        {
            for (auto* element : myParentXml.getChildWithTagNameIterator ("MYTAG"))
                doSomethingWithXmlElement (element);
        }
        @endcode
    */
    Iterator<GetNextElementWithTagName> getChildWithTagNameIterator (StringRef name) const
    {
        return Iterator<GetNextElementWithTagName> { getChildByName (name), name };
    }

   #ifndef DOXYGEN
    [[deprecated]] z0 macroBasedForLoop() const noexcept {}

    [[deprecated ("This has been deprecated in favour of the toString method.")]]
    Txt createDocument (StringRef dtdToUse,
                           b8 allOnOneLine = false,
                           b8 includeXmlHeader = true,
                           StringRef encodingType = "UTF-8",
                           i32 lineWrapLength = 60) const;

    [[deprecated ("This has been deprecated in favour of the writeTo method.")]]
    z0 writeToStream (OutputStream& output,
                        StringRef dtdToUse,
                        b8 allOnOneLine = false,
                        b8 includeXmlHeader = true,
                        StringRef encodingType = "UTF-8",
                        i32 lineWrapLength = 60) const;

    [[deprecated ("This has been deprecated in favour of the writeTo method.")]]
    b8 writeToFile (const File& destinationFile,
                      StringRef dtdToUse,
                      StringRef encodingType = "UTF-8",
                      i32 lineWrapLength = 60) const;
   #endif

private:
    //==============================================================================
    struct XmlAttributeNode
    {
        XmlAttributeNode (const XmlAttributeNode&) noexcept;
        XmlAttributeNode (const Identifier&, const Txt&) noexcept;
        XmlAttributeNode (Txt::CharPointerType, Txt::CharPointerType);

        LinkedListPointer<XmlAttributeNode> nextListItem;
        Identifier name;
        Txt value;

    private:
        XmlAttributeNode& operator= (const XmlAttributeNode&) = delete;
    };

    friend class XmlDocument;
    friend class LinkedListPointer<XmlAttributeNode>;
    friend class LinkedListPointer<XmlElement>;
    friend class LinkedListPointer<XmlElement>::Appender;
    friend class NamedValueSet;

    LinkedListPointer<XmlElement> nextListItem, firstChildElement;
    LinkedListPointer<XmlAttributeNode> attributes;
    Txt tagName;

    XmlElement (i32) noexcept;
    z0 copyChildrenAndAttributesFrom (const XmlElement&);
    z0 writeElementAsText (OutputStream&, i32, i32, tukk) const;
    z0 getChildElementsAsArray (XmlElement**) const noexcept;
    z0 reorderChildElements (XmlElement**, i32) noexcept;
    XmlAttributeNode* getAttribute (StringRef) const noexcept;

    // Sigh.. L"" or _T ("") string literals are problematic in general, and really inappropriate
    // for XML tags. Use a UTF-8 encoded literal instead, or if you're really determined to use
    // UTF-16, cast it to a Txt and use the other constructor.
    XmlElement (const wchar_t*) = delete;

    DRX_LEAK_DETECTOR (XmlElement)
};

//==============================================================================
#ifndef DOXYGEN

/** DEPRECATED: A handy macro to make it easy to iterate all the child elements in an XmlElement.

    New code should avoid this macro, and instead use getChildIterator directly.

    The parentXmlElement should be a reference to the parent XML, and the childElementVariableName
    will be the name of a pointer to each child element.

    E.g. @code
    XmlElement* myParentXml = createSomeKindOfXmlDocument();

    forEachXmlChildElement (*myParentXml, child)
    {
        if (child->hasTagName ("FOO"))
            doSomethingWithXmlElement (child);
    }

    @endcode

    @see forEachXmlChildElementWithTagName
*/
#define forEachXmlChildElement(parentXmlElement, childElementVariableName) \
    for (auto* (childElementVariableName) : ((parentXmlElement).macroBasedForLoop(), (parentXmlElement).getChildIterator()))

/** DEPRECATED: A macro that makes it easy to iterate all the child elements of an XmlElement
    which have a specified tag.

    New code should avoid this macro, and instead use getChildWithTagNameIterator directly.

    This does the same job as the forEachXmlChildElement macro, but only for those
    elements that have a particular tag name.

    The parentXmlElement should be a reference to the parent XML, and the childElementVariableName
    will be the name of a pointer to each child element. The requiredTagName is the
    tag name to match.

    E.g. @code
    XmlElement* myParentXml = createSomeKindOfXmlDocument();

    forEachXmlChildElementWithTagName (*myParentXml, child, "MYTAG")
    {
        // the child object is now guaranteed to be a <MYTAG> element..
        doSomethingWithMYTAGElement (child);
    }

    @endcode

    @see forEachXmlChildElement
*/
#define forEachXmlChildElementWithTagName(parentXmlElement, childElementVariableName, requiredTagName) \
    for (auto* (childElementVariableName) : ((parentXmlElement).macroBasedForLoop(), (parentXmlElement).getChildWithTagNameIterator ((requiredTagName))))

#endif

} // namespace drx
