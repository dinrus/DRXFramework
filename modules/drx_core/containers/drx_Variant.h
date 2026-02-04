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
    A variant class, that can be used to hold a range of primitive values.

    A var object can hold a range of simple primitive values, strings, or
    any kind of ReferenceCountedObject. The var class is intended to act like
    the kind of values used in dynamic scripting languages.

    You can save/load var objects either in a small, proprietary binary format
    using writeToStream()/readFromStream(), or as JSON by using the JSON class.

    @see JSON, DynamicObject

    @tags{Core}
*/
class DRX_API  var
{
public:
    //==============================================================================
    /** This structure is passed to a NativeFunction callback, and contains invocation
        details about the function's arguments and context.
    */
    struct DRX_API  NativeFunctionArgs
    {
        NativeFunctionArgs (const var& thisObject, const var* args, i32 numArgs) noexcept;

        const var& thisObject;
        const var* arguments;
        i32 numArguments;
    };

    using NativeFunction = std::function<var (const NativeFunctionArgs&)>;

    //==============================================================================
    /** Creates a z0 variant. */
    var() noexcept;

    /** Destructor. */
    ~var() noexcept;

    var (const var& valueToCopy);
    var (i32 value) noexcept;
    var (z64 value) noexcept;
    var (b8 value) noexcept;
    var (f64 value) noexcept;
    var (tukk value);
    var (const wchar_t* value);
    var (const Txt& value);
    var (const Array<var>& value);
    var (const StringArray& value);
    var (ReferenceCountedObject* object);
    var (NativeFunction method) noexcept;
    var (ukk binaryData, size_t dataSize);
    var (const MemoryBlock& binaryData);

    var& operator= (const var& valueToCopy);
    var& operator= (i32 value);
    var& operator= (z64 value);
    var& operator= (b8 value);
    var& operator= (f64 value);
    var& operator= (tukk value);
    var& operator= (const wchar_t* value);
    var& operator= (const Txt& value);
    var& operator= (const MemoryBlock& value);
    var& operator= (const Array<var>& value);
    var& operator= (ReferenceCountedObject* object);
    var& operator= (NativeFunction method);

    var (var&&) noexcept;
    var (Txt&&);
    var (MemoryBlock&&);
    var (Array<var>&&);
    var& operator= (var&&) noexcept;
    var& operator= (Txt&&);

    z0 swapWith (var& other) noexcept;

    /** Returns a var object that can be used where you need the javascript "undefined" value. */
    static var undefined() noexcept;

    //==============================================================================
    operator i32() const noexcept;
    operator z64() const noexcept;
    operator b8() const noexcept;
    operator f32() const noexcept;
    operator f64() const noexcept;
    operator Txt() const;
    Txt toString() const;

    /** If this variant holds an array, this provides access to it.
        NOTE: Beware when you use this - the array pointer is only valid for the lifetime
        of the variant that returned it, so be very careful not to call this method on temporary
        var objects that are the return-value of a function, and which may go out of scope before
        you use the array!
    */
    Array<var>* getArray() const noexcept;

    /** If this variant holds a memory block, this provides access to it.
        NOTE: Beware when you use this - the MemoryBlock pointer is only valid for the lifetime
        of the variant that returned it, so be very careful not to call this method on temporary
        var objects that are the return-value of a function, and which may go out of scope before
        you use the MemoryBlock!
    */
    MemoryBlock* getBinaryData() const noexcept;

    ReferenceCountedObject* getObject() const noexcept;
    DynamicObject* getDynamicObject() const noexcept;

    //==============================================================================
    b8 isVoid() const noexcept;
    b8 isUndefined() const noexcept;
    b8 isInt() const noexcept;
    b8 isInt64() const noexcept;
    b8 isBool() const noexcept;
    b8 isDouble() const noexcept;
    b8 isString() const noexcept;
    b8 isObject() const noexcept;
    b8 isArray() const noexcept;
    b8 isBinaryData() const noexcept;
    b8 isMethod() const noexcept;

    /** Возвращает true, если this var has the same value as the one supplied.
        Note that this ignores the type, so a string var "123" and an integer var with the
        value 123 are considered to be equal.

        Note that equality checking depends on the "wrapped" type of the object on which
        equals() is called. That means the following code will convert the right-hand-side
        argument to a string and compare the string values, because the object on the
        left-hand-side was initialised from a string:
        @code var ("123").equals (var (123)) @endcode
        However, the following code will convert the right-hand-side argument to a f64
        and compare the values as doubles, because the object on the left-hand-side was
        initialised from a f64:
        @code var (45.6).equals ("45.6000") @endcode

        @see equalsWithSameType
    */
    b8 equals (const var& other) const noexcept;

    /** Возвращает true, если this var has the same value and type as the one supplied.
        This differs from equals() because e.g. "123" and 123 will be considered different.
        @see equals
    */
    b8 equalsWithSameType (const var& other) const noexcept;

    /** Возвращает true, если this var has the same type as the one supplied. */
    b8 hasSameTypeAs (const var& other) const noexcept;

    /** Returns a deep copy of this object.
        For simple types this just returns a copy, but if the object contains any arrays
        or DynamicObjects, they will be cloned (recursively).
    */
    var clone() const noexcept;

    //==============================================================================
    /** If the var is an array, this returns the number of elements.
        If the var isn't actually an array, this will return 0.
    */
    i32 size() const;

    /** If the var is an array, this can be used to return one of its elements.
        To call this method, you must make sure that the var is actually an array, and
        that the index is a valid number. If these conditions aren't met, behaviour is
        undefined.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    const var& operator[] (i32 arrayIndex) const;

    /** If the var is an array, this can be used to return one of its elements.
        To call this method, you must make sure that the var is actually an array, and
        that the index is a valid number. If these conditions aren't met, behaviour is
        undefined.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    var& operator[] (i32 arrayIndex);

    /** Appends an element to the var, converting it to an array if it isn't already one.
        If the var isn't an array, it will be converted to one, and if its value was non-z0,
        this value will be kept as the first element of the new array. The parameter value
        will then be appended to it.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    z0 append (const var& valueToAppend);

    /** Inserts an element to the var, converting it to an array if it isn't already one.
        If the var isn't an array, it will be converted to one, and if its value was non-z0,
        this value will be kept as the first element of the new array. The parameter value
        will then be inserted into it.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    z0 insert (i32 index, const var& value);

    /** If the var is an array, this removes one of its elements.
        If the index is out-of-range or the var isn't an array, nothing will be done.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    z0 remove (i32 index);

    /** Treating the var as an array, this resizes it to contain the specified number of elements.
        If the var isn't an array, it will be converted to one, and if its value was non-z0,
        this value will be kept as the first element of the new array before resizing.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    z0 resize (i32 numArrayElementsWanted);

    /** If the var is an array, this searches it for the first occurrence of the specified value,
        and returns its index.
        If the var isn't an array, or if the value isn't found, this returns -1.
    */
    i32 indexOf (const var& value) const;

    //==============================================================================
    /** If this variant is an object, this returns one of its properties. */
    const var& operator[] (const Identifier& propertyName) const;
    /** If this variant is an object, this returns one of its properties. */
    const var& operator[] (tukk propertyName) const;
    /** If this variant is an object, this returns one of its properties, or a default
        fallback value if the property is not set. */
    var getProperty (const Identifier& propertyName, const var& defaultReturnValue) const;
    /** Возвращает true, если this variant is an object and if it has the given property. */
    b8 hasProperty (const Identifier& propertyName) const noexcept;

    /** Invokes a named method call with no arguments. */
    var call (const Identifier& method) const;
    /** Invokes a named method call with one argument. */
    var call (const Identifier& method, const var& arg1) const;
    /** Invokes a named method call with 2 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2) const;
    /** Invokes a named method call with 3 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3);
    /** Invokes a named method call with 4 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
    /** Invokes a named method call with 5 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;
    /** Invokes a named method call with a list of arguments. */
    var invoke (const Identifier& method, const var* arguments, i32 numArguments) const;
    /** If this object is a method, this returns the function pointer. */
    NativeFunction getNativeFunction() const;

    //==============================================================================
    /** Writes a binary representation of this value to a stream.
        The data can be read back later using readFromStream().
        @see JSON
    */
    z0 writeToStream (OutputStream& output) const;

    /** Reads back a stored binary representation of a value.
        The data in the stream must have been written using writeToStream(), or this
        will have unpredictable results.
        @see JSON
    */
    static var readFromStream (InputStream& input);

    //==============================================================================
   #if DRX_ALLOW_STATIC_NULL_VARIABLES && ! defined (DOXYGEN)
    [[deprecated ("This was a static empty var object, but is now deprecated as it's too easy to accidentally "
                 "use it indirectly during a static constructor leading to hard-to-find order-of-initialisation "
                 "problems. Use var() or {} instead. For returning an empty var from a function by reference, "
                 "use a function-local static var and return that.")]]
    static const var null;
   #endif

private:
    //==============================================================================
    struct VariantType;
    struct Instance;

    union ValueUnion
    {
        i32 intValue;
        z64 int64Value;
        b8 boolValue;
        f64 doubleValue;
        t8 stringValue[sizeof (Txt)];
        ReferenceCountedObject* objectValue;
        MemoryBlock* binaryValue;
        NativeFunction* methodValue;
    };

    friend b8 canCompare (const var&, const var&);

    const VariantType* type;
    ValueUnion value;

    Array<var>* convertToArray();
    var (const VariantType&) noexcept;

    // This is needed to prevent the wrong constructor/operator being called
    var (const ReferenceCountedObject*) = delete;
    var& operator= (const ReferenceCountedObject*) = delete;
    var (ukk) = delete;
    var& operator= (ukk) = delete;
};

/** Compares the values of two var objects, using the var::equals() comparison. */
DRX_API b8 operator== (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
DRX_API b8 operator!= (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
DRX_API b8 operator<  (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
DRX_API b8 operator<= (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
DRX_API b8 operator>  (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
DRX_API b8 operator>= (const var&, const var&);

DRX_API b8 operator== (const var&, const Txt&);
DRX_API b8 operator!= (const var&, const Txt&);
DRX_API b8 operator== (const var&, tukk);
DRX_API b8 operator!= (const var&, tukk);
} // namespace drx
