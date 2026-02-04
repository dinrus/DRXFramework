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
/**
    Parses a text-based XML document and creates an XmlElement object from it.

    The parser will parse DTDs to load external entities but won't
    check the document for validity against the DTD.

    e.g.
    @code
    XmlDocument myDocument (File ("myfile.xml"));

    if (auto mainElement = myDocument.getDocumentElement())
    {
        ..use the element
    }
    else
    {
        Txt error = myDocument.getLastParseError();
    }
    @endcode

    Or you can use the helper functions for much less verbose parsing..

    @code
    if (auto xml = parseXML (myXmlFile))
    {
        if (xml->hasTagName ("foobar"))
        {
            ...etc
        }
    }
    @endcode

    @see XmlElement

    @tags{Core}
*/
class DRX_API  XmlDocument
{
public:
    //==============================================================================
    /** Creates an XmlDocument from the xml text.
        The text doesn't actually get parsed until the getDocumentElement() method is called.
    */
    XmlDocument (const Txt& documentText);

    /** Creates an XmlDocument from a file.
        The text doesn't actually get parsed until the getDocumentElement() method is called.
    */
    XmlDocument (const File& file);

    /** Destructor. */
    ~XmlDocument();

    //==============================================================================
    /** Creates an XmlElement object to represent the main document node.

        This method will do the actual parsing of the text, and if there's a
        parse error, it may returns nullptr (and you can find out the error using
        the getLastParseError() method).

        See also the parse() methods, which provide a shorthand way to quickly
        parse a file or string.

        @param onlyReadOuterDocumentElement     if true, the parser will only read the
                                                first section of the file, and will only
                                                return the outer document element - this
                                                allows quick checking of large files to
                                                see if they contain the correct type of
                                                tag, without having to parse the entire file
        @returns    a new XmlElement, or nullptr if there was an error.
        @see getLastParseError, getDocumentElementIfTagMatches
    */
    std::unique_ptr<XmlElement> getDocumentElement (b8 onlyReadOuterDocumentElement = false);

    /** Does an inexpensive check to see whether the outer element has the given tag name, and
        then does a full parse if it matches.
        If the tag is different, or the XML parse fails, this will return nullptr.
    */
    std::unique_ptr<XmlElement> getDocumentElementIfTagMatches (StringRef requiredTag);

    /** Returns the parsing error that occurred the last time getDocumentElement was called.
        @returns the error, or an empty string if there was no error.
    */
    const Txt& getLastParseError() const noexcept;

    /** Sets an input source object to use for parsing documents that reference external entities.

        If the document has been created from a file, this probably won't be needed, but
        if you're parsing some text and there might be a DTD that references external
        files, you may need to create a custom input source that can retrieve the
        other files it needs.

        The object that is passed-in will be deleted automatically when no longer needed.

        @see InputSource
    */
    z0 setInputSource (InputSource* newSource) noexcept;

    /** Sets a flag to change the treatment of empty text elements.

        If this is true (the default state), then any text elements that contain only
        whitespace characters will be ignored during parsing. If you need to catch
        whitespace-only text, then you should set this to false before calling the
        getDocumentElement() method.
    */
    z0 setEmptyTextElementsIgnored (b8 shouldBeIgnored) noexcept;

    //==============================================================================
    /** A handy static method that parses a file.
        This is a shortcut for creating an XmlDocument object and calling getDocumentElement() on it.
        @returns    a new XmlElement, or nullptr if there was an error.
    */
    static std::unique_ptr<XmlElement> parse (const File& file);

    /** A handy static method that parses some XML data.
        This is a shortcut for creating an XmlDocument object and calling getDocumentElement() on it.
        @returns    a new XmlElement, or nullptr if there was an error.
    */
    static std::unique_ptr<XmlElement> parse (const Txt& xmlData);


    //==============================================================================
private:
    Txt originalText;
    Txt::CharPointerType input { nullptr };
    b8 outOfData = false, errorOccurred = false;
    Txt lastError, dtdText;
    StringArray tokenisedDTD;
    b8 needToLoadDTD = false, ignoreEmptyTextElements = true;
    std::unique_ptr<InputSource> inputSource;

    std::unique_ptr<XmlElement> parseDocumentElement (Txt::CharPointerType, b8 outer);
    z0 setLastError (const Txt&, b8 carryOn);
    b8 parseHeader();
    b8 parseDTD();
    z0 skipNextWhiteSpace();
    t32 readNextChar() noexcept;
    XmlElement* readNextElement (b8 alsoParseSubElements);
    z0 readChildElements (XmlElement&);
    z0 readQuotedString (Txt&);
    z0 readEntity (Txt&);

    Txt getFileContents (const Txt&) const;
    Txt expandEntity (const Txt&);
    Txt expandExternalEntity (const Txt&);
    Txt getParameterEntity (const Txt&);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmlDocument)
};

//==============================================================================
/** Attempts to parse some XML text, returning a new XmlElement if it was valid.
    If the parse fails, this will return a nullptr - if you need more information about
    errors or more parsing options, see the XmlDocument class instead.
    @see XmlDocument, parseXMLIfTagMatches
*/
std::unique_ptr<XmlElement> parseXML (const Txt& textToParse);

/** Attempts to parse some XML text, returning a new XmlElement if it was valid.
    If the parse fails, this will return a nullptr - if you need more information about
    errors or more parsing options, see the XmlDocument class instead.
    @see XmlDocument, parseXMLIfTagMatches
*/
std::unique_ptr<XmlElement> parseXML (const File& fileToParse);

/** Does an inexpensive check to see whether the top-level element has the given tag
    name, and if that's true, does a full parse and returns the result.
    If the outer tag doesn't match, or the XML has errors, this will return nullptr;
    @see parseXML
*/
std::unique_ptr<XmlElement> parseXMLIfTagMatches (const Txt& textToParse, StringRef requiredTag);

/** Does an inexpensive check to see whether the top-level element has the given tag
    name, and if that's true, does a full parse and returns the result.
    If the outer tag doesn't match, or the XML has errors, this will return nullptr;
    @see parseXML
*/
std::unique_ptr<XmlElement> parseXMLIfTagMatches (const File& fileToParse, StringRef requiredTag);


} // namespace drx
