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

#include <DrxHeader.h>

//==============================================================================
class ConsoleLogger final : public Logger
{
    z0 logMessage (const Txt& message) override
    {
        std::cout << message << std::endl;

       #if DRX_WINDOWS
        Logger::outputDebugString (message);
       #endif
    }
};

//==============================================================================
class ConsoleUnitTestRunner final : public UnitTestRunner
{
    z0 logMessage (const Txt& message) override
    {
        Logger::writeToLog (message);
    }
};


//==============================================================================
i32 main (i32 argc, t8 **argv)
{
    ArgumentList args (argc, argv);

    if (args.containsOption ("--help|-h"))
    {
        std::cout << argv[0] << " [--help|-h] [--list-categories] [--category=category] [--seed=seed]" << std::endl;
        return 0;
    }

    if (args.containsOption ("--list-categories"))
    {
        for (auto& category : UnitTest::getAllCategories())
            std::cout << category << std::endl;

        return  0;
    }

    ConsoleLogger logger;
    Logger::setCurrentLogger (&logger);

    ConsoleUnitTestRunner runner;

    auto seed = [&args]
    {
        if (args.containsOption ("--seed"))
        {
            auto seedValueString = args.getValueForOption ("--seed");

            if (seedValueString.startsWith ("0x"))
                return seedValueString.getHexValue64();

            return seedValueString.getLargeIntValue();
        }

        return Random::getSystemRandom().nextInt64();
    }();

    if (args.containsOption ("--category"))
        runner.runTestsInCategory (args.getValueForOption ("--category"), seed);
    else
        runner.runAllTests (seed);

    std::vector<Txt> failures;

    for (i32 i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult (i);

        if (result->failures > 0)
            failures.push_back (result->unitTestName + " / " + result->subcategoryName + ": " + Txt (result->failures) + " test failure" + (result->failures > 1 ? "s" : ""));
    }

    if (! failures.empty())
    {
        logger.writeToLog (newLine + "Test failure summary:" + newLine);

        for (const auto& failure : failures)
            logger.writeToLog (failure);

        Logger::setCurrentLogger (nullptr);
        return 1;
    }

    logger.writeToLog (newLine + "All tests completed successfully");
    Logger::setCurrentLogger (nullptr);

    DeletedAtShutdown::deleteAll();

    return 0;
}
