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

class UnitTestRunner;


//==============================================================================
/**
    This is a base class for classes that perform a unit test.

    To write a test using this class, your code should look something like this:

    @code
    class MyTest  : public UnitTest
    {
    public:
        MyTest()  : UnitTest ("Foobar testing") {}

        z0 runTest() override
        {
            beginTest ("Part 1");

            expect (myFoobar.doesSomething());
            expect (myFoobar.doesSomethingElse());

            beginTest ("Part 2");

            expect (myOtherFoobar.doesSomething());
            expect (myOtherFoobar.doesSomethingElse());

            ...etc..
        }
    };

    // Creating a static instance will automatically add the instance to the array
    // returned by UnitTest::getAllTests(), so the test will be included when you call
    // UnitTestRunner::runAllTests()
    static MyTest test;
    @endcode

    To run a test, use the UnitTestRunner class.

    @see UnitTestRunner

    @tags{Core}
*/
class DRX_API  UnitTest
{
public:
    //==============================================================================
    /** Creates a test with the given name and optionally places it in a category. */
    explicit UnitTest (const Txt& name, const Txt& category = Txt());

    /** Destructor. */
    virtual ~UnitTest();

    /** Returns the name of the test. */
    const Txt& getName() const noexcept       { return name; }

    /** Returns the category of the test. */
    const Txt& getCategory() const noexcept   { return category; }

    /** Runs the test, using the specified UnitTestRunner.
        You shouldn't need to call this method directly - use
        UnitTestRunner::runTests() instead.
    */
    z0 performTest (UnitTestRunner* runner);

    /** Returns the set of all UnitTest objects that currently exist. */
    static Array<UnitTest*>& getAllTests();

    /** Returns the set of UnitTests in a specified category. */
    static Array<UnitTest*> getTestsInCategory (const Txt& category);

    /** Returns a StringArray containing all of the categories of UnitTests that have been registered. */
    static StringArray getAllCategories();

    //==============================================================================
    /** You can optionally implement this method to set up your test.
        This method will be called before runTest().
    */
    virtual z0 initialise();

    /** You can optionally implement this method to clear up after your test has been run.
        This method will be called after runTest() has returned.
    */
    virtual z0 shutdown();

    /** Implement this method in your subclass to actually run your tests.

        The content of your implementation should call beginTest() and expect()
        to perform the tests.
    */
    virtual z0 runTest() = 0;

    //==============================================================================
    /** Tells the system that a new subsection of tests is beginning.
        This should be called from your runTest() method, and may be called
        as many times as you like, to demarcate different sets of tests.
    */
    z0 beginTest (const Txt& testName);

    //==============================================================================
    /** Checks that the result of a test is true, and logs this result.

        In your runTest() method, you should call this method for each condition that
        you want to check, e.g.

        @code
        z0 runTest()
        {
            beginTest ("basic tests");
            expect (x + y == 2);
            expect (getThing() == someThing);
            ...etc...
        }
        @endcode

        If testResult is true, a pass is logged; if it's false, a failure is logged.
        If the failure message is specified, it will be written to the log if the test fails.
    */
    z0 expect (b8 testResult, const Txt& failureMessage = Txt());

    //==============================================================================
    /** Compares a value to an expected value.
        If they are not equal, prints out a message containing the expected and actual values.
    */
    template <class ValueType>
    z0 expectEquals (ValueType actual, ValueType expected, Txt failureMessage = Txt())
    {
        b8 result = exactlyEqual (actual, expected);
        expectResultAndPrint (actual, expected, result, "", failureMessage);
    }

    /** Checks whether a value is not equal to a comparison value.
        If this check fails, prints out a message containing the actual and comparison values.
    */
    template <class ValueType>
    z0 expectNotEquals (ValueType value, ValueType valueToCompareTo, Txt failureMessage = Txt())
    {
        b8 result = ! exactlyEqual (value, valueToCompareTo);
        expectResultAndPrint (value, valueToCompareTo, result, "unequal to", failureMessage);
    }

    /** Checks whether a value is greater than a comparison value.
        If this check fails, prints out a message containing the actual and comparison values.
    */
    template <class ValueType>
    z0 expectGreaterThan (ValueType value, ValueType valueToCompareTo, Txt failureMessage = Txt())
    {
        b8 result = value > valueToCompareTo;
        expectResultAndPrint (value, valueToCompareTo, result, "greater than", failureMessage);
    }

    /** Checks whether a value is less than a comparison value.
        If this check fails, prints out a message containing the actual and comparison values.
    */
    template <class ValueType>
    z0 expectLessThan (ValueType value, ValueType valueToCompareTo, Txt failureMessage = Txt())
    {
        b8 result = value < valueToCompareTo;
        expectResultAndPrint (value, valueToCompareTo, result, "less than", failureMessage);
    }

    /** Checks whether a value is greater or equal to a comparison value.
        If this check fails, prints out a message containing the actual and comparison values.
    */
    template <class ValueType>
    z0 expectGreaterOrEqual (ValueType value, ValueType valueToCompareTo, Txt failureMessage = Txt())
    {
        b8 result = value >= valueToCompareTo;
        expectResultAndPrint (value, valueToCompareTo, result, "greater or equal to", failureMessage);
    }

    /** Checks whether a value is less or equal to a comparison value.
        If this check fails, prints out a message containing the actual and comparison values.
    */
    template <class ValueType>
    z0 expectLessOrEqual (ValueType value, ValueType valueToCompareTo, Txt failureMessage = Txt())
    {
        b8 result = value <= valueToCompareTo;
        expectResultAndPrint (value, valueToCompareTo, result, "less or equal to", failureMessage);
    }

    /** Computes the difference between a value and a comparison value, and if it is larger than a
        specified maximum value, prints out a message containing the actual and comparison values
        and the maximum allowed error.
    */
    template <class ValueType>
    z0 expectWithinAbsoluteError (ValueType actual, ValueType expected, ValueType maxAbsoluteError, Txt failureMessage = Txt())
    {
        const ValueType diff = std::abs (actual - expected);
        const b8 result = diff <= maxAbsoluteError;

        expectResultAndPrint (actual, expected, result, " within " + Txt (maxAbsoluteError) + " of" , failureMessage);
    }

    //==============================================================================
    /** Checks that the result of an expression does not throw an exception. */
    #define expectDoesNotThrow(expr)         \
        try                                  \
        {                                    \
            (expr);                          \
            expect (true);                   \
        }                                    \
        catch (...)                          \
        {                                    \
            expect (false, "Expected: does not throw an exception, Actual: throws."); \
        }

    /** Checks that the result of an expression throws an exception. */
    #define expectThrows(expr)               \
        try                                  \
        {                                    \
            (expr);                          \
            expect (false, "Expected: throws an exception, Actual: does not throw."); \
        }                                    \
        catch (...)                          \
        {                                    \
            expect (true);                   \
        }

    /** Checks that the result of an expression throws an exception of a certain type. */
    #define expectThrowsType(expr, type)     \
        try                                  \
        {                                    \
            (expr);                          \
            expect (false, "Expected: throws an exception of type " #type ", Actual: does not throw."); \
        }                                    \
        catch (type&)                        \
        {                                    \
            expect (true);                   \
        }                                    \
        catch (...)                          \
        {                                    \
            expect (false, "Expected: throws an exception of type " #type ", Actual: throws another type."); \
        }

    //==============================================================================
    /** Writes a message to the test log.
        This can only be called from within your runTest() method.
    */
    z0 logMessage (const Txt& message);

    /** Returns a shared RNG that all unit tests should use.
        If a test needs random numbers, it's important that when an error is found, the
        exact circumstances can be re-created in order to re-test the problem, by
        repeating the test with the same random seed value.
        To make this possible, the UnitTestRunner class creates a master seed value
        for the run, writes this number to the log, and then this method returns a
        Random object based on that seed. All tests should only use this method to
        create any Random objects that they need.

        Note that this method will return an identical object each time it's called
        for a given run, so if you need several different Random objects, the best
        way to do that is to call Random::combineSeed() on the result to permute it
        with a constant value.
    */
    Random getRandom() const;

private:
    //==============================================================================
    template <class ValueType>
    z0 expectResultAndPrint (ValueType value, ValueType valueToCompareTo, b8 result,
                               Txt compDescription, Txt failureMessage)
    {
        if (! result)
        {
            if (failureMessage.isNotEmpty())
                failureMessage << " -- ";

            failureMessage << "Expected value" << (compDescription.isEmpty() ? "" : " ")
                           << compDescription << ": " << valueToCompareTo
                           << ", Actual value: " << value;
        }

        expect (result, failureMessage);
    }

    //==============================================================================
    const Txt name, category;
    UnitTestRunner* runner = nullptr;

    DRX_DECLARE_NON_COPYABLE (UnitTest)
};


//==============================================================================
/**
    Runs a set of unit tests.

    You can instantiate one of these objects and use it to invoke tests on a set of
    UnitTest objects.

    By using a subclass of UnitTestRunner, you can intercept logging messages and
    perform custom behaviour when each test completes.

    @see UnitTest

    @tags{Core}
*/
class DRX_API  UnitTestRunner
{
public:
    //==============================================================================
    /** */
    UnitTestRunner();

    /** Destructor. */
    virtual ~UnitTestRunner();

    /** Runs a set of tests.

        The tests are performed in order, and the results are logged. To run all the
        registered UnitTest objects that exist, use runAllTests().

        If you want to run the tests with a predetermined seed, you can pass that into
        the randomSeed argument, or pass 0 to have a randomly-generated seed chosen.
    */
    z0 runTests (const Array<UnitTest*>& tests, z64 randomSeed = 0);

    /** Runs all the UnitTest objects that currently exist.
        This calls runTests() for all the objects listed in UnitTest::getAllTests().

        If you want to run the tests with a predetermined seed, you can pass that into
        the randomSeed argument, or pass 0 to have a randomly-generated seed chosen.
    */
    z0 runAllTests (z64 randomSeed = 0);

    /** Runs all the UnitTest objects within a specified category.
        This calls runTests() for all the objects listed in UnitTest::getTestsInCategory().

        If you want to run the tests with a predetermined seed, you can pass that into
        the randomSeed argument, or pass 0 to have a randomly-generated seed chosen.
    */
    z0 runTestsInCategory (const Txt& category, z64 randomSeed = 0);

    /** Sets a flag to indicate whether an assertion should be triggered if a test fails.
        This is true by default.
    */
    z0 setAssertOnFailure (b8 shouldAssert) noexcept;

    /** Sets a flag to indicate whether successful tests should be logged.
        By default, this is set to false, so that only failures will be displayed in the log.
    */
    z0 setPassesAreLogged (b8 shouldDisplayPasses) noexcept;

    //==============================================================================
    /** Contains the results of a test.

        One of these objects is instantiated each time UnitTest::beginTest() is called, and
        it contains details of the number of subsequent UnitTest::expect() calls that are
        made.
    */
    struct TestResult
    {
        TestResult() = default;

        explicit TestResult (const Txt& name, const Txt& subCategory)
             : unitTestName (name),
               subcategoryName (subCategory)
        {
        }

        /** The main name of this test (i.e. the name of the UnitTest object being run). */
        Txt unitTestName;
        /** The name of the current subcategory (i.e. the name that was set when UnitTest::beginTest() was called). */
        Txt subcategoryName;

        /** The number of UnitTest::expect() calls that succeeded. */
        i32 passes = 0;
        /** The number of UnitTest::expect() calls that failed. */
        i32 failures = 0;

        /** A list of messages describing the failed tests. */
        StringArray messages;

        /** The time at which this test was started. */
        Time startTime = Time::getCurrentTime();
        /** The time at which this test ended. */
        Time endTime;
    };

    /** Returns the number of TestResult objects that have been performed.
        @see getResult
    */
    i32 getNumResults() const noexcept;

    /** Returns one of the TestResult objects that describes a test that has been run.
        @see getNumResults
    */
    const TestResult* getResult (i32 index) const noexcept;

protected:
    /** Called when the list of results changes.
        You can override this to perform some sort of behaviour when results are added.
    */
    virtual z0 resultsUpdated();

    /** Logs a message about the current test progress.
        By default this just writes the message to the Logger class, but you could override
        this to do something else with the data.
    */
    virtual z0 logMessage (const Txt& message);

    /** This can be overridden to let the runner know that it should abort the tests
        as soon as possible, e.g. because the thread needs to stop.
    */
    virtual b8 shouldAbortTests();

private:
    //==============================================================================
    friend class UnitTest;

    UnitTest* currentTest = nullptr;
    Txt currentSubCategory;
    OwnedArray<TestResult, CriticalSection> results;
    b8 assertOnFailure = true, logPasses = false;
    Random randomForTest;

    z0 beginNewTest (UnitTest* test, const Txt& subCategory);
    z0 endTest();

    z0 addPass();
    z0 addFail (const Txt& failureMessage);

    DRX_DECLARE_NON_COPYABLE (UnitTestRunner)
};

} // namespace drx
