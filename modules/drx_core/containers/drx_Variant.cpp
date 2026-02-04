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

enum VariantStreamMarkers
{
    varMarker_Int       = 1,
    varMarker_BoolTrue  = 2,
    varMarker_BoolFalse = 3,
    varMarker_Double    = 4,
    varMarker_String    = 5,
    varMarker_Int64     = 6,
    varMarker_Array     = 7,
    varMarker_Binary    = 8,
    varMarker_Undefined = 9
};

//==============================================================================
struct var::VariantType
{
    struct VoidTag      {};
    struct UndefinedTag {};
    struct IntTag       {};
    struct Int64Tag     {};
    struct DoubleTag    {};
    struct BoolTag      {};
    struct StringTag    {};
    struct ObjectTag    {};
    struct ArrayTag     {};
    struct BinaryTag    {};
    struct MethodTag    {};

    // members =====================================================================
    b8 isVoid         = false;
    b8 isUndefined    = false;
    b8 isInt          = false;
    b8 isInt64        = false;
    b8 isBool         = false;
    b8 isDouble       = false;
    b8 isString       = false;
    b8 isObject       = false;
    b8 isArray        = false;
    b8 isBinary       = false;
    b8 isMethod       = false;
    b8 isComparable   = false;

    i32                     (*toInt)         (const ValueUnion&)                 = defaultToInt;
    z64                   (*toInt64)       (const ValueUnion&)                 = defaultToInt64;
    f64                  (*toDouble)      (const ValueUnion&)                 = defaultToDouble;
    Txt                  (*toString)      (const ValueUnion&)                 = defaultToString;
    b8                    (*toBool)        (const ValueUnion&)                 = defaultToBool;
    ReferenceCountedObject* (*toObject)      (const ValueUnion&)                 = defaultToObject;
    Array<var>*             (*toArray)       (const ValueUnion&)                 = defaultToArray;
    MemoryBlock*            (*toBinary)      (const ValueUnion&)                 = defaultToBinary;
    var                     (*clone)         (const var&)                        = defaultClone;
    z0                    (*cleanUp)       (ValueUnion&)                       = defaultCleanUp;
    z0                    (*createCopy)    (ValueUnion&, const ValueUnion&)    = defaultCreateCopy;

    b8                    (*equals)        (const ValueUnion&, const ValueUnion&, const VariantType&) = nullptr;
    z0                    (*writeToStream) (const ValueUnion&, OutputStream&) = nullptr;

    // defaults ====================================================================
    static i32                     defaultToInt         (const ValueUnion&)                          { return 0; }
    static z64                   defaultToInt64       (const ValueUnion&)                          { return 0; }
    static f64                  defaultToDouble      (const ValueUnion&)                          { return 0; }
    static Txt                  defaultToString      (const ValueUnion&)                          { return {}; }
    static b8                    defaultToBool        (const ValueUnion&)                          { return false; }
    static ReferenceCountedObject* defaultToObject      (const ValueUnion&)                          { return nullptr; }
    static Array<var>*             defaultToArray       (const ValueUnion&)                          { return nullptr; }
    static MemoryBlock*            defaultToBinary      (const ValueUnion&)                          { return nullptr; }
    static var                     defaultClone         (const var& other)                           { return other; }
    static z0                    defaultCleanUp       (ValueUnion&)                                {}
    static z0                    defaultCreateCopy    (ValueUnion& dest, const ValueUnion& source) { dest = source; }

    // z0 ========================================================================
    static b8 voidEquals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) noexcept
    {
        return otherType.isVoid || otherType.isUndefined;
    }

    static z0 voidWriteToStream (const ValueUnion&, OutputStream& output)
    {
        output.writeCompressedInt (0);
    }

    constexpr explicit VariantType (VoidTag) noexcept
        : isVoid            (true),
          isComparable      (true),
          equals            (voidEquals),
          writeToStream     (voidWriteToStream) {}

    // undefined ===================================================================
    static Txt undefinedToString (const ValueUnion&) { return "undefined"; }

    static b8 undefinedEquals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) noexcept
    {
        return otherType.isVoid || otherType.isUndefined;
    }

    static z0 undefinedWriteToStream (const ValueUnion&, OutputStream& output)
    {
        output.writeCompressedInt (1);
        output.writeByte (varMarker_Undefined);
    }

    constexpr explicit VariantType (UndefinedTag) noexcept
        : isUndefined   (true),
          toString      (undefinedToString),
          equals        (undefinedEquals),
          writeToStream (undefinedWriteToStream) {}

    // i32 =========================================================================
    static i32    intToInt    (const ValueUnion& data) noexcept   { return data.intValue; }
    static z64  intToInt64  (const ValueUnion& data) noexcept   { return (z64) data.intValue; }
    static f64 intToDouble (const ValueUnion& data) noexcept   { return (f64) data.intValue; }
    static Txt intToString (const ValueUnion& data)            { return Txt (data.intValue); }
    static b8   intToBool   (const ValueUnion& data) noexcept   { return data.intValue != 0; }

    static b8 intEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        if (otherType.isDouble || otherType.isInt64 || otherType.isString)
            return otherType.equals (otherData, data, VariantType { IntTag{} });

        return otherType.toInt (otherData) == data.intValue;
    }

    static z0 intWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (5);
        output.writeByte (varMarker_Int);
        output.writeInt (data.intValue);
    }

    constexpr explicit VariantType (IntTag) noexcept
        : isInt         (true),
          isComparable  (true),
          toInt         (intToInt),
          toInt64       (intToInt64),
          toDouble      (intToDouble),
          toString      (intToString),
          toBool        (intToBool),
          equals        (intEquals),
          writeToStream (intWriteToStream) {}

    // z64 =======================================================================
    static i32    int64ToInt    (const ValueUnion& data) noexcept   { return (i32) data.int64Value; }
    static z64  int64ToInt64  (const ValueUnion& data) noexcept   { return data.int64Value; }
    static f64 int64ToDouble (const ValueUnion& data) noexcept   { return (f64) data.int64Value; }
    static Txt int64ToString (const ValueUnion& data)            { return Txt (data.int64Value); }
    static b8   int64ToBool   (const ValueUnion& data) noexcept   { return data.int64Value != 0; }

    static b8 int64Equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        if (otherType.isDouble || otherType.isString)
            return otherType.equals (otherData, data, VariantType { Int64Tag{} });

        return otherType.toInt64 (otherData) == data.int64Value;
    }

    static z0 int64WriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Int64);
        output.writeInt64 (data.int64Value);
    }

    constexpr explicit VariantType (Int64Tag) noexcept
        : isInt64       (true),
          isComparable  (true),
          toInt         (int64ToInt),
          toInt64       (int64ToInt64),
          toDouble      (int64ToDouble),
          toString      (int64ToString),
          toBool        (int64ToBool),
          equals        (int64Equals),
          writeToStream (int64WriteToStream) {}

    // f64 ======================================================================
    static i32    doubleToInt    (const ValueUnion& data) noexcept   { return (i32) data.doubleValue; }
    static z64  doubleToInt64  (const ValueUnion& data) noexcept   { return (z64) data.doubleValue; }
    static f64 doubleToDouble (const ValueUnion& data) noexcept   { return data.doubleValue; }
    static Txt doubleToString (const ValueUnion& data)            { return serialiseDouble (data.doubleValue); }
    static b8   doubleToBool   (const ValueUnion& data) noexcept   { return ! exactlyEqual (data.doubleValue, 0.0); }

    static b8 doubleEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return std::abs (otherType.toDouble (otherData) - data.doubleValue) < std::numeric_limits<f64>::epsilon();
    }

    static z0 doubleWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Double);
        output.writeDouble (data.doubleValue);
    }

    constexpr explicit VariantType (DoubleTag) noexcept
        : isDouble      (true),
          isComparable  (true),
          toInt         (doubleToInt),
          toInt64       (doubleToInt64),
          toDouble      (doubleToDouble),
          toString      (doubleToString),
          toBool        (doubleToBool),
          equals        (doubleEquals),
          writeToStream (doubleWriteToStream) {}

    // b8 ========================================================================
    static i32    boolToInt    (const ValueUnion& data) noexcept   { return data.boolValue ? 1 : 0; }
    static z64  boolToInt64  (const ValueUnion& data) noexcept   { return data.boolValue ? 1 : 0; }
    static f64 boolToDouble (const ValueUnion& data) noexcept   { return data.boolValue ? 1.0 : 0.0; }
    static Txt boolToString (const ValueUnion& data)            { return Txt::charToString (data.boolValue ? (t32) '1' : (t32) '0'); }
    static b8   boolToBool   (const ValueUnion& data) noexcept   { return data.boolValue; }

    static b8 boolEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.toBool (otherData) == data.boolValue;
    }

    static z0 boolWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (1);
        output.writeByte (data.boolValue ? (t8) varMarker_BoolTrue : (t8) varMarker_BoolFalse);
    }

    constexpr explicit VariantType (BoolTag) noexcept
        : isBool        (true),
          isComparable  (true),
          toInt         (boolToInt),
          toInt64       (boolToInt64),
          toDouble      (boolToDouble),
          toString      (boolToString),
          toBool        (boolToBool),
          equals        (boolEquals),
          writeToStream (boolWriteToStream) {}

    // string ======================================================================
    static const Txt* getString (const ValueUnion& data) noexcept   { return unalignedPointerCast<const Txt*> (data.stringValue); }
    static       Txt* getString (      ValueUnion& data) noexcept   { return unalignedPointerCast<Txt*> (data.stringValue); }

    static i32    stringToInt    (const ValueUnion& data) noexcept   { return getString (data)->getIntValue(); }
    static z64  stringToInt64  (const ValueUnion& data) noexcept   { return getString (data)->getLargeIntValue(); }
    static f64 stringToDouble (const ValueUnion& data) noexcept   { return getString (data)->getDoubleValue(); }
    static Txt stringToString (const ValueUnion& data)            { return *getString (data); }
    static b8   stringToBool   (const ValueUnion& data) noexcept
    {
        return getString (data)->getIntValue() != 0
               || getString (data)->trim().equalsIgnoreCase ("true")
               || getString (data)->trim().equalsIgnoreCase ("yes");
    }

    static z0 stringCleanUp    (ValueUnion& data) noexcept                    { getString (data)-> ~Txt(); }
    static z0 stringCreateCopy (ValueUnion& dest, const ValueUnion& source)   { new (dest.stringValue) Txt (*getString (source)); }

    static b8 stringEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.toString (otherData) == *getString (data);
    }

    static z0 stringWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        auto* s = getString (data);
        const size_t len = s->getNumBytesAsUTF8() + 1;
        HeapBlock<t8> temp (len);
        s->copyToUTF8 (temp, len);
        output.writeCompressedInt ((i32) (len + 1));
        output.writeByte (varMarker_String);
        output.write (temp, len);
    }

    constexpr explicit VariantType (StringTag) noexcept
        : isString      (true),
          isComparable  (true),
          toInt         (stringToInt),
          toInt64       (stringToInt64),
          toDouble      (stringToDouble),
          toString      (stringToString),
          toBool        (stringToBool),
          cleanUp       (stringCleanUp),
          createCopy    (stringCreateCopy),
          equals        (stringEquals),
          writeToStream (stringWriteToStream) {}

    // object ======================================================================
    static Txt objectToString (const ValueUnion& data)
    {
        return "Object 0x" + Txt::toHexString ((i32) (pointer_sized_int) data.objectValue);
    }

    static b8                    objectToBool   (const ValueUnion& data) noexcept   { return data.objectValue != nullptr; }
    static ReferenceCountedObject* objectToObject (const ValueUnion& data) noexcept   { return data.objectValue; }

    static var objectClone (const var& original)
    {
        if (auto* d = original.getDynamicObject())
            return d->clone().release();

        jassertfalse; // can only clone DynamicObjects!
        return {};
    }

    static z0 objectCleanUp (ValueUnion& data) noexcept   { if (data.objectValue != nullptr) data.objectValue->decReferenceCount(); }

    static z0 objectCreateCopy (ValueUnion& dest, const ValueUnion& source)
    {
        dest.objectValue = source.objectValue;
        if (dest.objectValue != nullptr)
            dest.objectValue->incReferenceCount();
    }

    static b8 objectEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.toObject (otherData) == data.objectValue;
    }

    static z0 objectWriteToStream (const ValueUnion&, OutputStream& output)
    {
        jassertfalse; // Can't write an object to a stream!
        output.writeCompressedInt (0);
    }

    constexpr explicit VariantType (ObjectTag) noexcept
        : isObject      (true),
          toString      (objectToString),
          toBool        (objectToBool),
          toObject      (objectToObject),
          clone         (objectClone),
          cleanUp       (objectCleanUp),
          createCopy    (objectCreateCopy),
          equals        (objectEquals),
          writeToStream (objectWriteToStream) {}

    // array =======================================================================
    static Txt                  arrayToString (const ValueUnion&)            { return "[Array]"; }
    static ReferenceCountedObject* arrayToObject (const ValueUnion&) noexcept   { return nullptr; }

    static Array<var>* arrayToArray (const ValueUnion& data) noexcept
    {
        if (auto* a = dynamic_cast<RefCountedArray*> (data.objectValue))
            return &(a->array);

        return nullptr;
    }

    static b8 arrayEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        auto* thisArray = arrayToArray (data);
        auto* otherArray = otherType.toArray (otherData);
        return thisArray == otherArray || (thisArray != nullptr && otherArray != nullptr && *otherArray == *thisArray);
    }

    static var arrayClone (const var& original)
    {
        Array<var> arrayCopy;

        if (auto* array = arrayToArray (original.value))
        {
            arrayCopy.ensureStorageAllocated (array->size());

            for (auto& i : *array)
                arrayCopy.add (i.clone());
        }

        return var (arrayCopy);
    }

    static z0 arrayWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        if (auto* array = arrayToArray (data))
        {
            MemoryOutputStream buffer (512);
            buffer.writeCompressedInt (array->size());

            for (auto& i : *array)
                i.writeToStream (buffer);

            output.writeCompressedInt (1 + (i32) buffer.getDataSize());
            output.writeByte (varMarker_Array);
            output << buffer;
        }
    }

    struct RefCountedArray final : public ReferenceCountedObject
    {
        RefCountedArray (const Array<var>& a)  : array (a)  { incReferenceCount(); }
        RefCountedArray (Array<var>&& a)  : array (std::move (a)) { incReferenceCount(); }
        Array<var> array;
    };

    constexpr explicit VariantType (ArrayTag) noexcept
        : isObject      (true),
          isArray       (true),
          toString      (arrayToString),
          toBool        (objectToBool),
          toObject      (arrayToObject),
          toArray       (arrayToArray),
          clone         (arrayClone),
          cleanUp       (objectCleanUp),
          createCopy    (objectCreateCopy),
          equals        (arrayEquals),
          writeToStream (arrayWriteToStream) {}

    // binary ======================================================================
    static z0 binaryCleanUp    (ValueUnion& data) noexcept                    { delete data.binaryValue; }
    static z0 binaryCreateCopy (ValueUnion& dest, const ValueUnion& source)   { dest.binaryValue = new MemoryBlock (*source.binaryValue); }

    static Txt       binaryToString (const ValueUnion& data)            { return data.binaryValue->toBase64Encoding(); }
    static MemoryBlock* binaryToBinary (const ValueUnion& data) noexcept   { return data.binaryValue; }

    static b8 binaryEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        const MemoryBlock* const otherBlock = otherType.toBinary (otherData);
        return otherBlock != nullptr && *otherBlock == *data.binaryValue;
    }

    static z0 binaryWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (1 + (i32) data.binaryValue->getSize());
        output.writeByte (varMarker_Binary);
        output << *data.binaryValue;
    }

    constexpr explicit VariantType (BinaryTag) noexcept
        : isBinary      (true),
          toString      (binaryToString),
          toBinary      (binaryToBinary),
          cleanUp       (binaryCleanUp),
          createCopy    (binaryCreateCopy),
          equals        (binaryEquals),
          writeToStream (binaryWriteToStream) {}

    // method ======================================================================
    static z0 methodCleanUp    (ValueUnion& data) noexcept                    { if (data.methodValue != nullptr ) delete data.methodValue; }
    static z0 methodCreateCopy (ValueUnion& dest, const ValueUnion& source)   { dest.methodValue = new NativeFunction (*source.methodValue); }

    static Txt methodToString (const ValueUnion&)                 { return "Method"; }
    static b8   methodToBool   (const ValueUnion& data) noexcept   { return data.methodValue != nullptr; }

    static b8 methodEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.isMethod && otherData.methodValue == data.methodValue;
    }

    static z0 methodWriteToStream (const ValueUnion&, OutputStream& output)
    {
        jassertfalse; // Can't write a method to a stream!
        output.writeCompressedInt (0);
    }

    constexpr explicit VariantType (MethodTag) noexcept
        : isMethod      (true),
          toString      (methodToString),
          toBool        (methodToBool),
          cleanUp       (methodCleanUp),
          createCopy    (methodCreateCopy),
          equals        (methodEquals),
          writeToStream (methodWriteToStream) {}
};

struct var::Instance
{
    static constexpr VariantType attributesVoid           { VariantType::VoidTag{} };
    static constexpr VariantType attributesUndefined      { VariantType::UndefinedTag{} };
    static constexpr VariantType attributesInt            { VariantType::IntTag{} };
    static constexpr VariantType attributesInt64          { VariantType::Int64Tag{} };
    static constexpr VariantType attributesBool           { VariantType::BoolTag{} };
    static constexpr VariantType attributesDouble         { VariantType::DoubleTag{} };
    static constexpr VariantType attributesMethod         { VariantType::MethodTag{} };
    static constexpr VariantType attributesArray          { VariantType::ArrayTag{} };
    static constexpr VariantType attributesString         { VariantType::StringTag{} };
    static constexpr VariantType attributesBinary         { VariantType::BinaryTag{} };
    static constexpr VariantType attributesObject         { VariantType::ObjectTag{} };
};

//==============================================================================
var::var() noexcept : type (&Instance::attributesVoid) {}
var::var (const VariantType& t) noexcept  : type (&t) {}
var::~var() noexcept  { type->cleanUp (value); }

//==============================================================================
var::var (const var& valueToCopy)  : type (valueToCopy.type)
{
    type->createCopy (value, valueToCopy.value);
}

var::var (i32k v) noexcept       : type (&Instance::attributesInt)    { value.intValue = v; }
var::var (const z64 v) noexcept     : type (&Instance::attributesInt64)  { value.int64Value = v; }
var::var (const b8 v) noexcept      : type (&Instance::attributesBool)   { value.boolValue = v; }
var::var (const f64 v) noexcept    : type (&Instance::attributesDouble) { value.doubleValue = v; }
var::var (NativeFunction m) noexcept  : type (&Instance::attributesMethod) { value.methodValue = new NativeFunction (m); }
var::var (const Array<var>& v)        : type (&Instance::attributesArray)  { value.objectValue = new VariantType::RefCountedArray (v); }
var::var (const Txt& v)            : type (&Instance::attributesString) { new (value.stringValue) Txt (v); }
var::var (tukk const v)        : type (&Instance::attributesString) { new (value.stringValue) Txt (v); }
var::var (const wchar_t* const v)     : type (&Instance::attributesString) { new (value.stringValue) Txt (v); }
var::var (ukk v, size_t sz)   : type (&Instance::attributesBinary) { value.binaryValue = new MemoryBlock (v, sz); }
var::var (const MemoryBlock& v)       : type (&Instance::attributesBinary) { value.binaryValue = new MemoryBlock (v); }

var::var (const StringArray& v)       : type (&Instance::attributesArray)
{
    Array<var> strings;
    strings.ensureStorageAllocated (v.size());

    for (auto& i : v)
        strings.add (var (i));

    value.objectValue = new VariantType::RefCountedArray (strings);
}

var::var (ReferenceCountedObject* const object)  : type (&Instance::attributesObject)
{
    value.objectValue = object;

    if (object != nullptr)
        object->incReferenceCount();
}

var var::undefined() noexcept           { return var (Instance::attributesUndefined); }

//==============================================================================
b8 var::isVoid() const noexcept       { return type->isVoid; }
b8 var::isUndefined() const noexcept  { return type->isUndefined; }
b8 var::isInt() const noexcept        { return type->isInt; }
b8 var::isInt64() const noexcept      { return type->isInt64; }
b8 var::isBool() const noexcept       { return type->isBool; }
b8 var::isDouble() const noexcept     { return type->isDouble; }
b8 var::isString() const noexcept     { return type->isString; }
b8 var::isObject() const noexcept     { return type->isObject; }
b8 var::isArray() const noexcept      { return type->isArray; }
b8 var::isBinaryData() const noexcept { return type->isBinary; }
b8 var::isMethod() const noexcept     { return type->isMethod; }

var::operator i32() const noexcept                      { return type->toInt (value); }
var::operator z64() const noexcept                    { return type->toInt64 (value); }
var::operator b8() const noexcept                     { return type->toBool (value); }
var::operator f32() const noexcept                    { return (f32) type->toDouble (value); }
var::operator f64() const noexcept                   { return type->toDouble (value); }
Txt var::toString() const                            { return type->toString (value); }
var::operator Txt() const                            { return type->toString (value); }
ReferenceCountedObject* var::getObject() const noexcept { return type->toObject (value); }
Array<var>* var::getArray() const noexcept              { return type->toArray (value); }
MemoryBlock* var::getBinaryData() const noexcept        { return type->toBinary (value); }
DynamicObject* var::getDynamicObject() const noexcept   { return dynamic_cast<DynamicObject*> (getObject()); }

//==============================================================================
z0 var::swapWith (var& other) noexcept
{
    std::swap (type, other.type);
    std::swap (value, other.value);
}

var& var::operator= (const var& v)               { type->cleanUp (value); type = v.type; type->createCopy (value, v.value); return *this; }
var& var::operator= (i32k v)                { type->cleanUp (value); type = &Instance::attributesInt; value.intValue = v; return *this; }
var& var::operator= (const z64 v)              { type->cleanUp (value); type = &Instance::attributesInt64; value.int64Value = v; return *this; }
var& var::operator= (const b8 v)               { type->cleanUp (value); type = &Instance::attributesBool; value.boolValue = v; return *this; }
var& var::operator= (const f64 v)             { type->cleanUp (value); type = &Instance::attributesDouble; value.doubleValue = v; return *this; }
var& var::operator= (tukk const v)        { type->cleanUp (value); type = &Instance::attributesString; new (value.stringValue) Txt (v); return *this; }
var& var::operator= (const wchar_t* const v)     { type->cleanUp (value); type = &Instance::attributesString; new (value.stringValue) Txt (v); return *this; }
var& var::operator= (const Txt& v)            { type->cleanUp (value); type = &Instance::attributesString; new (value.stringValue) Txt (v); return *this; }
var& var::operator= (const MemoryBlock& v)       { type->cleanUp (value); type = &Instance::attributesBinary; value.binaryValue = new MemoryBlock (v); return *this; }
var& var::operator= (const Array<var>& v)        { var v2 (v); swapWith (v2); return *this; }
var& var::operator= (ReferenceCountedObject* v)  { var v2 (v); swapWith (v2); return *this; }
var& var::operator= (NativeFunction v)           { var v2 (v); swapWith (v2); return *this; }

var::var (var&& other) noexcept
    : type (other.type),
      value (other.value)
{
    other.type = &Instance::attributesVoid;
}

var& var::operator= (var&& other) noexcept
{
    swapWith (other);
    return *this;
}

var::var (Txt&& v)  : type (&Instance::attributesString)
{
    new (value.stringValue) Txt (std::move (v));
}

var::var (MemoryBlock&& v)  : type (&Instance::attributesBinary)
{
    value.binaryValue = new MemoryBlock (std::move (v));
}

var::var (Array<var>&& v)  : type (&Instance::attributesArray)
{
    value.objectValue = new VariantType::RefCountedArray (std::move (v));
}

var& var::operator= (Txt&& v)
{
    type->cleanUp (value);
    type = &Instance::attributesString;
    new (value.stringValue) Txt (std::move (v));
    return *this;
}

//==============================================================================
b8 var::equals (const var& other) const noexcept
{
    return type->equals (value, other.value, *other.type);
}

b8 var::equalsWithSameType (const var& other) const noexcept
{
    return hasSameTypeAs (other) && equals (other);
}

b8 var::hasSameTypeAs (const var& other) const noexcept
{
    return type == other.type;
}

b8 canCompare (const var& v1, const var& v2)
{
    return v1.type->isComparable && v2.type->isComparable;
}

static i32 compare (const var& v1, const var& v2)
{
    if (v1.isString() && v2.isString())
        return v1.toString().compare (v2.toString());

    auto diff = static_cast<f64> (v1) - static_cast<f64> (v2);
    return exactlyEqual (diff, 0.0) ? 0 : (diff < 0 ? -1 : 1);
}

b8 operator== (const var& v1, const var& v2)     { return v1.equals (v2); }
b8 operator!= (const var& v1, const var& v2)     { return ! v1.equals (v2); }
b8 operator<  (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) <  0; }
b8 operator>  (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) >  0; }
b8 operator<= (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) <= 0; }
b8 operator>= (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) >= 0; }

b8 operator== (const var& v1, const Txt& v2)  { return v1.toString() == v2; }
b8 operator!= (const var& v1, const Txt& v2)  { return v1.toString() != v2; }
b8 operator== (const var& v1, tukk v2)    { return v1.toString() == v2; }
b8 operator!= (const var& v1, tukk v2)    { return v1.toString() != v2; }

//==============================================================================
var var::clone() const noexcept
{
    return type->clone (*this);
}

//==============================================================================
const var& var::operator[] (const Identifier& propertyName) const
{
    if (auto* o = getDynamicObject())
        return o->getProperty (propertyName);

    return getNullVarRef();
}

const var& var::operator[] (tukk const propertyName) const
{
    return operator[] (Identifier (propertyName));
}

var var::getProperty (const Identifier& propertyName, const var& defaultReturnValue) const
{
    if (auto* o = getDynamicObject())
        return o->getProperties().getWithDefault (propertyName, defaultReturnValue);

    return defaultReturnValue;
}

b8 var::hasProperty (const Identifier& propertyName) const noexcept
{
    if (auto* o = getDynamicObject())
        return o->hasProperty (propertyName);

    return false;
}

var::NativeFunction var::getNativeFunction() const
{
    return isMethod() && (value.methodValue != nullptr) ? *value.methodValue : nullptr;
}

var var::invoke (const Identifier& method, const var* arguments, i32 numArguments) const
{
    if (auto* o = getDynamicObject())
        return o->invokeMethod (method, var::NativeFunctionArgs (*this, arguments, numArguments));

    return {};
}

var var::call (const Identifier& method) const
{
    return invoke (method, nullptr, 0);
}

var var::call (const Identifier& method, const var& arg1) const
{
    return invoke (method, &arg1, 1);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2) const
{
    var args[] = { arg1, arg2 };
    return invoke (method, args, 2);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3)
{
    var args[] = { arg1, arg2, arg3 };
    return invoke (method, args, 3);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const
{
    var args[] = { arg1, arg2, arg3, arg4 };
    return invoke (method, args, 4);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const
{
    var args[] = { arg1, arg2, arg3, arg4, arg5 };
    return invoke (method, args, 5);
}

//==============================================================================
i32 var::size() const
{
    if (auto array = getArray())
        return array->size();

    return 0;
}

const var& var::operator[] (i32 arrayIndex) const
{
    auto array = getArray();

    // When using this method, the var must actually be an array, and the index
    // must be in-range!
    jassert (array != nullptr && isPositiveAndBelow (arrayIndex, array->size()));

    return array->getReference (arrayIndex);
}

var& var::operator[] (i32 arrayIndex)
{
    auto array = getArray();

    // When using this method, the var must actually be an array, and the index
    // must be in-range!
    jassert (array != nullptr && isPositiveAndBelow (arrayIndex, array->size()));

    return array->getReference (arrayIndex);
}

Array<var>* var::convertToArray()
{
    if (auto array = getArray())
        return array;

    Array<var> tempVar;

    if (! isVoid())
        tempVar.add (*this);

    *this = tempVar;
    return getArray();
}

z0 var::append (const var& n)
{
    convertToArray()->add (n);
}

z0 var::remove (i32k index)
{
    if (auto array = getArray())
        array->remove (index);
}

z0 var::insert (i32k index, const var& n)
{
    convertToArray()->insert (index, n);
}

z0 var::resize (i32k numArrayElementsWanted)
{
    convertToArray()->resize (numArrayElementsWanted);
}

i32 var::indexOf (const var& n) const
{
    if (auto array = getArray())
        return array->indexOf (n);

    return -1;
}

//==============================================================================
z0 var::writeToStream (OutputStream& output) const
{
    type->writeToStream (value, output);
}

var var::readFromStream (InputStream& input)
{
    i32k numBytes = input.readCompressedInt();

    if (numBytes > 0)
    {
        switch (input.readByte())
        {
            case varMarker_Int:         return var (input.readInt());
            case varMarker_Int64:       return var (input.readInt64());
            case varMarker_BoolTrue:    return var (true);
            case varMarker_BoolFalse:   return var (false);
            case varMarker_Double:      return var (input.readDouble());

            case varMarker_String:
            {
                MemoryOutputStream mo;
                mo.writeFromInputStream (input, numBytes - 1);
                return var (mo.toUTF8());
            }

            case varMarker_Binary:
            {
                MemoryBlock mb ((size_t) numBytes - 1);

                if (numBytes > 1)
                {
                    i32k numRead = input.read (mb.getData(), numBytes - 1);
                    mb.setSize ((size_t) numRead);
                }

                return var (mb);
            }

            case varMarker_Array:
            {
                var v;
                auto* destArray = v.convertToArray();

                for (i32 i = input.readCompressedInt(); --i >= 0;)
                    destArray->add (readFromStream (input));

                return v;
            }

            default:
                input.skipNextBytes (numBytes - 1); break;
        }
    }

    return {};
}

var::NativeFunctionArgs::NativeFunctionArgs (const var& t, const var* args, i32 numArgs) noexcept
    : thisObject (t), arguments (args), numArguments (numArgs)
{
}

//==============================================================================
#if DRX_ALLOW_STATIC_NULL_VARIABLES

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

const var var::null;

DRX_END_IGNORE_DEPRECATION_WARNINGS

#endif

} // namespace drx
