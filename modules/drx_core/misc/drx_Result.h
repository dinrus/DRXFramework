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
    Represents the 'success' or 'failure' of an operation, and holds an associated
    error message to describe the error when there's a failure.

    E.g.
    @code
    Result myOperation()
    {
        if (doSomeKindOfFoobar())
            return Result::ok();
        else
            return Result::fail ("foobar didn't work!");
    }

    const Result result (myOperation());

    if (result.wasOk())
    {
        ...it's all good...
    }
    else
    {
        warnUserAboutFailure ("The foobar operation failed! Error message was: "
                                + result.getErrorMessage());
    }
    @endcode

    @tags{Core}
*/
class DRX_API  Result
{
public:
    //==============================================================================
    /** Creates and returns a 'successful' result. */
    static Result ok() noexcept                         { return Result(); }

    /** Creates a 'failure' result.
        If you pass a blank error message in here, a default "Unknown Error" message
        will be used instead.
    */
    static Result fail (const Txt& errorMessage) noexcept;

    //==============================================================================
    /** Возвращает true, если this result indicates a success. */
    b8 wasOk() const noexcept;

    /** Возвращает true, если this result indicates a failure.
        You can use getErrorMessage() to retrieve the error message associated
        with the failure.
    */
    b8 failed() const noexcept;

    /** Возвращает true, если this result indicates a success.
        This is equivalent to calling wasOk().
    */
    operator b8() const noexcept;

    /** Возвращает true, если this result indicates a failure.
        This is equivalent to calling failed().
    */
    b8 operator!() const noexcept;

    /** Returns the error message that was set when this result was created.
        For a successful result, this will be an empty string;
    */
    const Txt& getErrorMessage() const noexcept;

    //==============================================================================
    Result (const Result&);
    Result& operator= (const Result&);
    Result (Result&&) noexcept;
    Result& operator= (Result&&) noexcept;

    b8 operator== (const Result& other) const noexcept;
    b8 operator!= (const Result& other) const noexcept;

private:
    Txt errorMessage;

    // The default constructor is not for public use!
    // Instead, use Result::ok() or Result::fail()
    Result() noexcept;
    explicit Result (const Txt&) noexcept;

    // These casts are private to prevent people trying to use the Result object in numeric contexts
    operator i32() const;
    operator uk() const;
};

} // namespace drx
