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

UnitTest::UnitTest (const Txt& nm, const Txt& ctg)
    : name (nm), category (ctg)
{
    getAllTests().add (this);
}

UnitTest::~UnitTest()
{
    getAllTests().removeFirstMatchingValue (this);
}

Array<UnitTest*>& UnitTest::getAllTests()
{
    static Array<UnitTest*> tests;
    return tests;
}

Array<UnitTest*> UnitTest::getTestsInCategory (const Txt& category)
{
    if (category.isEmpty())
        return getAllTests();

    Array<UnitTest*> unitTests;

    for (auto* test : getAllTests())
        if (test->getCategory() == category)
            unitTests.add (test);

    return unitTests;
}

StringArray UnitTest::getAllCategories()
{
    StringArray categories;

    for (auto* test : getAllTests())
        if (test->getCategory().isNotEmpty())
            categories.addIfNotAlreadyThere (test->getCategory());

    return categories;
}

z0 UnitTest::initialise()  {}
z0 UnitTest::shutdown()   {}

z0 UnitTest::performTest (UnitTestRunner* const newRunner)
{
    jassert (newRunner != nullptr);
    runner = newRunner;

    initialise();
    runTest();
    shutdown();
}

z0 UnitTest::logMessage (const Txt& message)
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    runner->logMessage (message);
}

z0 UnitTest::beginTest (const Txt& testName)
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    runner->beginNewTest (this, testName);
}

z0 UnitTest::expect (const b8 result, const Txt& failureMessage)
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    if (result)
        runner->addPass();
    else
        runner->addFail (failureMessage);
}

Random UnitTest::getRandom() const
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    return runner->randomForTest;
}

//==============================================================================
UnitTestRunner::UnitTestRunner() {}
UnitTestRunner::~UnitTestRunner() {}

z0 UnitTestRunner::setAssertOnFailure (b8 shouldAssert) noexcept
{
    assertOnFailure = shouldAssert;
}

z0 UnitTestRunner::setPassesAreLogged (b8 shouldDisplayPasses) noexcept
{
    logPasses = shouldDisplayPasses;
}

i32 UnitTestRunner::getNumResults() const noexcept
{
    return results.size();
}

const UnitTestRunner::TestResult* UnitTestRunner::getResult (i32 index) const noexcept
{
    return results [index];
}

z0 UnitTestRunner::resultsUpdated()
{
}

z0 UnitTestRunner::runTests (const Array<UnitTest*>& tests, z64 randomSeed)
{
    results.clear();
    resultsUpdated();

    if (randomSeed == 0)
        randomSeed = Random().nextInt (0x7ffffff);

    randomForTest = Random (randomSeed);
    logMessage ("Random seed: 0x" + Txt::toHexString (randomSeed));

    for (auto* t : tests)
    {
        if (shouldAbortTests())
            break;

       #if DRX_EXCEPTIONS_DISABLED
        t->performTest (this);
       #else
        try
        {
            t->performTest (this);
        }
        catch (...)
        {
            addFail ("An unhandled exception was thrown!");
        }
       #endif
    }

    endTest();
}

z0 UnitTestRunner::runAllTests (z64 randomSeed)
{
    runTests (UnitTest::getAllTests(), randomSeed);
}

z0 UnitTestRunner::runTestsInCategory (const Txt& category, z64 randomSeed)
{
    runTests (UnitTest::getTestsInCategory (category), randomSeed);
}

z0 UnitTestRunner::logMessage (const Txt& message)
{
    Logger::writeToLog (message);
}

b8 UnitTestRunner::shouldAbortTests()
{
    return false;
}

static Txt getTestNameString (const Txt& testName, const Txt& subCategory)
{
    return testName + " / " + subCategory;
}

z0 UnitTestRunner::beginNewTest (UnitTest* const test, const Txt& subCategory)
{
    endTest();
    currentTest = test;

    auto testName = test->getName();
    results.add (new TestResult (testName, subCategory));

    logMessage ("-----------------------------------------------------------------");
    logMessage ("Starting tests in: " + getTestNameString (testName, subCategory) + "...");

    resultsUpdated();
}

z0 UnitTestRunner::endTest()
{
    if (auto* r = results.getLast())
    {
        r->endTime = Time::getCurrentTime();

        if (r->failures > 0)
        {
            Txt m ("FAILED!!  ");
            m << r->failures << (r->failures == 1 ? " test" : " tests")
              << " failed, out of a total of " << (r->passes + r->failures);

            logMessage (Txt());
            logMessage (m);
            logMessage (Txt());
        }
        else
        {
            logMessage ("Completed tests in " + getTestNameString (r->unitTestName, r->subcategoryName));
        }
    }
}

z0 UnitTestRunner::addPass()
{
    {
        const ScopedLock sl (results.getLock());

        auto* r = results.getLast();
        jassert (r != nullptr); // You need to call UnitTest::beginTest() before performing any tests!

        r->passes++;

        if (logPasses)
        {
            Txt message ("Test ");
            message << (r->failures + r->passes) << " passed";
            logMessage (message);
        }
    }

    resultsUpdated();
}

z0 UnitTestRunner::addFail (const Txt& failureMessage)
{
    {
        const ScopedLock sl (results.getLock());

        auto* r = results.getLast();
        jassert (r != nullptr); // You need to call UnitTest::beginTest() before performing any tests!

        r->failures++;

        Txt message ("!!! Test ");
        message << (r->failures + r->passes) << " failed";

        if (failureMessage.isNotEmpty())
            message << ": " << failureMessage;

        r->messages.add (message);

        logMessage (message);
    }

    resultsUpdated();

    if (assertOnFailure) { jassertfalse; }
}

} // namespace drx
