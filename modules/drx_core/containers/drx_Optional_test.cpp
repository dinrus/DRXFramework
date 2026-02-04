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

/*  Not nested, so that ADL works for the swap function. */
struct ThrowOnMoveOrSwap
{
    ThrowOnMoveOrSwap() = default;
    ThrowOnMoveOrSwap (ThrowOnMoveOrSwap&&) { throw std::bad_alloc{}; }
};
static z0 swap (ThrowOnMoveOrSwap&, ThrowOnMoveOrSwap&) { throw std::bad_alloc{}; }

class OptionalUnitTest final : public UnitTest
{
public:
    OptionalUnitTest() : UnitTest ("Optional", UnitTestCategories::containers) {}

    z0 runTest() override
    {
        beginTest ("Default-constructed optional is invalid");
        {
            Optional<i32> o;
            expect (! o.hasValue());
        }

        beginTest ("Constructing from Nullopt is invalid");
        {
            Optional<i32> o (nullopt);
            expect (! o.hasValue());
        }

        beginTest ("Optional constructed from value is valid");
        {
            Optional<i32> o = 5;
            expect (o.hasValue());
            expectEquals (*o, 5);
        }

        using Ptr = std::shared_ptr<i32>;
        const auto makePtr = [] { return std::make_shared<i32>(); };

        beginTest ("Constructing from a moved optional calls appropriate member functions");
        {
            auto ptr = makePtr();
            Optional<Ptr> original (ptr);
            expect (ptr.use_count() == 2);
            auto other = std::move (original);
            // A moved-from optional still contains a value!
            expect (original.hasValue());
            expect (other.hasValue());
            expect (ptr.use_count() == 2);
        }

        beginTest ("Moving an empty optional to a populated one destroys the instance");
        {
            auto ptr = makePtr();
            Optional<Ptr> original (ptr);
            expect (ptr.use_count() == 2);
            original = Optional<Ptr>();
            expect (ptr.use_count() == 1);
        }

        beginTest ("Copying an empty optional to a populated one destroys the instance");
        {
            auto ptr = makePtr();
            Optional<Ptr> original (ptr);
            expect (ptr.use_count() == 2);
            Optional<Ptr> empty;
            original = empty;
            expect (ptr.use_count() == 1);
        }

        beginTest ("Moving a populated optional calls appropriate member functions");
        {
            auto a = makePtr();
            auto b = makePtr();

            Optional<Ptr> aOpt (a);
            Optional<Ptr> bOpt (b);

            expect (a.use_count() == 2);
            expect (b.use_count() == 2);

            aOpt = std::move (bOpt);

            expect (aOpt.hasValue());
            expect (bOpt.hasValue());

            expect (a.use_count() == 1);
            expect (b.use_count() == 2);
        }

        beginTest ("Copying a populated optional calls appropriate member functions");
        {
            auto a = makePtr();
            auto b = makePtr();

            Optional<Ptr> aOpt (a);
            Optional<Ptr> bOpt (b);

            expect (a.use_count() == 2);
            expect (b.use_count() == 2);

            aOpt = bOpt;

            expect (aOpt.hasValue());
            expect (bOpt.hasValue());

            expect (a.use_count() == 1);
            expect (b.use_count() == 3);
        }

        beginTest ("Moving an empty optional to an empty one does nothing");
        {
            Optional<Ptr> original;
            original = Optional<Ptr>();
            expect (! original.hasValue());
        }

        beginTest ("Copying an empty optional to an empty one does nothing");
        {
            Optional<Ptr> original;
            Optional<Ptr> empty;
            original = empty;
            expect (! original.hasValue());
            expect (! empty.hasValue());
        }

        beginTest ("Moving a populated optional calls appropriate member functions");
        {
            auto a = makePtr();

            Optional<Ptr> aOpt (a);
            Optional<Ptr> empty;

            expect (a.use_count() == 2);

            empty = std::move (aOpt);

            expect (empty.hasValue());
            expect (aOpt.hasValue());

            expect (a.use_count() == 2);
        }

        beginTest ("Copying a populated optional calls appropriate member functions");
        {
            auto a = makePtr();

            Optional<Ptr> aOpt (a);
            Optional<Ptr> empty;

            expect (a.use_count() == 2);

            empty = aOpt;

            expect (aOpt.hasValue());
            expect (empty.hasValue());

            expect (a.use_count() == 3);
        }

        struct ThrowOnCopy
        {
            ThrowOnCopy() = default;

            // Put into an invalid state and throw
            ThrowOnCopy (const ThrowOnCopy&)
            {
                value = -100;
                throw std::bad_alloc{};
            }

            // Put into an invalid state and throw
            ThrowOnCopy& operator= (const ThrowOnCopy&)
            {
                value = -100;
                throw std::bad_alloc{};
            }

            ThrowOnCopy (ThrowOnCopy&&) noexcept = default;
            ThrowOnCopy& operator= (ThrowOnCopy&&) noexcept = default;

            ~ThrowOnCopy() = default;

            i32 value = 0;
        };

        beginTest ("Strong exception safety is maintained when forwarding over empty object");
        {
            b8 threw = false;
            Optional<ThrowOnCopy> a;

            try
            {
                ThrowOnCopy t;
                a = t;
            }
            catch (const std::bad_alloc&)
            {
                threw = true;
            }

            expect (threw);
            expect (! a.hasValue()); // If construction failed, this object should still be well-formed but empty
        }

        beginTest ("Weak exception safety is maintained when forwarding over populated object");
        {
            b8 threw = false;
            Optional<ThrowOnCopy> a = ThrowOnCopy();
            a->value = 5;

            try
            {
                ThrowOnCopy t;
                a = t;
            }
            catch (const std::bad_alloc&)
            {
                threw = true;
            }

            expect (threw);
            expect (a.hasValue());
            expect (a->value == -100); // If we assign to an extant object, it's up to that object to provide an exception guarantee
        }

        beginTest ("Strong exception safety is maintained when copying over empty object");
        {
            b8 threw = false;
            Optional<ThrowOnCopy> a;

            try
            {
                Optional<ThrowOnCopy> t = ThrowOnCopy{};
                a = t;
            }
            catch (const std::bad_alloc&)
            {
                threw = true;
            }

            expect (threw);
            expect (! a.hasValue());
        }

        beginTest ("Exception safety of contained type is maintained when copying over populated object");
        {
            b8 threw = false;
            Optional<ThrowOnCopy> a = ThrowOnCopy();
            a->value = 5;

            try
            {
                Optional<ThrowOnCopy> t = ThrowOnCopy{};
                a = t;
            }
            catch (const std::bad_alloc&)
            {
                threw = true;
            }

            expect (threw);
            expect (a.hasValue());
            expect (a->value == -100);
        }

        beginTest ("Assigning from nullopt clears the instance");
        {
            auto ptr = makePtr();
            Optional<Ptr> a (ptr);
            expect (ptr.use_count() == 2);
            a = nullopt;
            expect (ptr.use_count() == 1);
        }

        struct Foo {};
        struct Bar final : public Foo {};

        beginTest ("Can be constructed from compatible type");
        {
            Optional<std::shared_ptr<Foo>> opt { std::make_shared<Bar>() };
        }

        beginTest ("Can be assigned from compatible type");
        {
            Optional<std::shared_ptr<Foo>> opt;
            opt = std::make_shared<Bar>();
        }

        beginTest ("Can copy from compatible type");
        {
            auto ptr = std::make_shared<Bar>();
            Optional<std::shared_ptr<Bar>> bar (ptr);
            Optional<std::shared_ptr<Foo>> foo (bar);
            expect (ptr.use_count() == 3);
        }

        beginTest ("Can move from compatible type");
        {
            auto ptr = std::make_shared<Bar>();
            Optional<std::shared_ptr<Foo>> foo (Optional<std::shared_ptr<Bar>> { ptr });
            expect (ptr.use_count() == 2);
        }

        beginTest ("Can copy assign from compatible type");
        {
            auto ptr = std::make_shared<Bar>();
            Optional<std::shared_ptr<Bar>> bar (ptr);
            Optional<std::shared_ptr<Foo>> foo;
            foo = bar;
            expect (ptr.use_count() == 3);
        }

        beginTest ("Can move assign from compatible type");
        {
            auto ptr = std::make_shared<Bar>();
            Optional<std::shared_ptr<Foo>> foo;
            foo = Optional<std::shared_ptr<Bar>> (ptr);
            expect (ptr.use_count() == 2);
        }

        beginTest ("An exception thrown during emplace leaves the optional without a value");
        {
            Optional<ThrowOnCopy> opt { ThrowOnCopy{} };
            b8 threw = false;

            try
            {
                ThrowOnCopy t;
                opt.emplace (t);
            }
            catch (const std::bad_alloc&)
            {
                threw = true;
            }

            expect (threw);
            expect (! opt.hasValue());
        }

        beginTest ("Swap does nothing to two empty optionals");
        {
            Optional<Ptr> a, b;
            expect (! a.hasValue());
            expect (! b.hasValue());

            a.swap (b);

            expect (! a.hasValue());
            expect (! b.hasValue());
        }

        beginTest ("Swap transfers ownership if one optional contains a value");
        {
            {
                Ptr ptr = makePtr();
                Optional<Ptr> a, b = ptr;
                expect (! a.hasValue());
                expect (b.hasValue());
                expect (ptr.use_count() == 2);

                a.swap (b);

                expect (a.hasValue());
                expect (! b.hasValue());
                expect (ptr.use_count() == 2);
            }

            {
                auto ptr = makePtr();
                Optional<Ptr> a = ptr, b;
                expect (a.hasValue());
                expect (! b.hasValue());
                expect (ptr.use_count() == 2);

                a.swap (b);

                expect (! a.hasValue());
                expect (b.hasValue());
                expect (ptr.use_count() == 2);
            }
        }

        beginTest ("Swap calls std::swap to swap two populated optionals");
        {
            auto x = makePtr(), y = makePtr();
            Optional<Ptr> a = x, b = y;
            expect (a.hasValue());
            expect (b.hasValue());
            expect (x.use_count() == 2);
            expect (y.use_count() == 2);
            expect (*a == x);
            expect (*b == y);

            a.swap (b);

            expect (a.hasValue());
            expect (b.hasValue());
            expect (x.use_count() == 2);
            expect (y.use_count() == 2);
            expect (*a == y);
            expect (*b == x);
        }

        beginTest ("An exception thrown during a swap leaves both objects in the previous populated state");
        {
            {
                Optional<ThrowOnMoveOrSwap> a, b;
                a.emplace();

                expect (a.hasValue());
                expect (! b.hasValue());

                b8 threw = false;

                try
                {
                    a.swap (b);
                }
                catch (const std::bad_alloc&)
                {
                    threw = true;
                }

                expect (threw);
                expect (a.hasValue());
                expect (! b.hasValue());
            }

            {
                Optional<ThrowOnMoveOrSwap> a, b;
                b.emplace();

                expect (! a.hasValue());
                expect (b.hasValue());

                b8 threw = false;

                try
                {
                    a.swap (b);
                }
                catch (const std::bad_alloc&)
                {
                    threw = true;
                }

                expect (threw);
                expect (! a.hasValue());
                expect (b.hasValue());
            }

            {
                Optional<ThrowOnMoveOrSwap> a, b;
                a.emplace();
                b.emplace();

                expect (a.hasValue());
                expect (b.hasValue());

                b8 threw = false;

                try
                {
                    a.swap (b);
                }
                catch (const std::bad_alloc&)
                {
                    threw = true;
                }

                expect (threw);
                expect (a.hasValue());
                expect (b.hasValue());
            }
        }

        beginTest ("Relational tests");
        {
            expect (Optional<i32> (1) == Optional<i32> (1));
            expect (Optional<i32>() == Optional<i32>());
            expect (! (Optional<i32> (1) == Optional<i32>()));
            expect (! (Optional<i32>() == Optional<i32> (1)));
            expect (! (Optional<i32> (1) == Optional<i32> (2)));

            expect (Optional<i32> (1) != Optional<i32> (2));
            expect (! (Optional<i32>() != Optional<i32>()));
            expect (Optional<i32> (1) != Optional<i32>());
            expect (Optional<i32>() != Optional<i32> (1));
            expect (! (Optional<i32> (1) != Optional<i32> (1)));

            expect (Optional<i32>() < Optional<i32> (1));
            expect (! (Optional<i32> (1) < Optional<i32>()));
            expect (! (Optional<i32>() < Optional<i32>()));
            expect (Optional<i32> (1) < Optional<i32> (2));

            expect (Optional<i32>() <= Optional<i32> (1));
            expect (! (Optional<i32> (1) <= Optional<i32>()));
            expect (Optional<i32>() <= Optional<i32>());
            expect (Optional<i32> (1) <= Optional<i32> (2));

            expect (! (Optional<i32>() > Optional<i32> (1)));
            expect (Optional<i32> (1) > Optional<i32>());
            expect (! (Optional<i32>() > Optional<i32>()));
            expect (! (Optional<i32> (1) > Optional<i32> (2)));

            expect (! (Optional<i32>() >= Optional<i32> (1)));
            expect (Optional<i32> (1) >= Optional<i32>());
            expect (Optional<i32>() >= Optional<i32>());
            expect (! (Optional<i32> (1) >= Optional<i32> (2)));

            expect (Optional<i32>() == nullopt);
            expect (! (Optional<i32> (1) == nullopt));
            expect (nullopt == Optional<i32>());
            expect (! (nullopt == Optional<i32> (1)));

            expect (! (Optional<i32>() != nullopt));
            expect (Optional<i32> (1) != nullopt);
            expect (! (nullopt != Optional<i32>()));
            expect (nullopt != Optional<i32> (1));

            expect (! (Optional<i32>() < nullopt));
            expect (! (Optional<i32> (1) < nullopt));

            expect (! (nullopt < Optional<i32>()));
            expect (nullopt < Optional<i32> (1));

            expect (Optional<i32>() <= nullopt);
            expect (! (Optional<i32> (1) <= nullopt));

            expect (nullopt <= Optional<i32>());
            expect (nullopt <= Optional<i32> (1));

            expect (! (Optional<i32>() > nullopt));
            expect (Optional<i32> (1) > nullopt);

            expect (! (nullopt > Optional<i32>()));
            expect (! (nullopt > Optional<i32> (1)));

            expect (Optional<i32>() >= nullopt);
            expect (Optional<i32> (1) >= nullopt);

            expect (nullopt >= Optional<i32>());
            expect (! (nullopt >= Optional<i32> (1)));

            expect (! (Optional<i32>() == 5));
            expect (! (Optional<i32> (1) == 5));
            expect (Optional<i32> (1) == 1);
            expect (! (5 == Optional<i32>()));
            expect (! (5 == Optional<i32> (1)));
            expect (1 == Optional<i32> (1));

            expect (Optional<i32>() != 5);
            expect (Optional<i32> (1) != 5);
            expect (! (Optional<i32> (1) != 1));
            expect (5 != Optional<i32>());
            expect (5 != Optional<i32> (1));
            expect (! (1 != Optional<i32> (1)));

            expect (Optional<i32>() < 5);
            expect (Optional<i32> (1) < 5);
            expect (! (Optional<i32> (1) < 1));
            expect (! (Optional<i32> (1) < 0));

            expect (! (5 < Optional<i32>()));
            expect (! (5 < Optional<i32> (1)));
            expect (! (1 < Optional<i32> (1)));
            expect (0 < Optional<i32> (1));

            expect (Optional<i32>() <= 5);
            expect (Optional<i32> (1) <= 5);
            expect (Optional<i32> (1) <= 1);
            expect (! (Optional<i32> (1) <= 0));

            expect (! (5 <= Optional<i32>()));
            expect (! (5 <= Optional<i32> (1)));
            expect (1 <= Optional<i32> (1));
            expect (0 <= Optional<i32> (1));

            expect (! (Optional<i32>() > 5));
            expect (! (Optional<i32> (1) > 5));
            expect (! (Optional<i32> (1) > 1));
            expect (Optional<i32> (1) > 0);

            expect (5 > Optional<i32>());
            expect (5 > Optional<i32> (1));
            expect (! (1 > Optional<i32> (1)));
            expect (! (0 > Optional<i32> (1)));

            expect (! (Optional<i32>() >= 5));
            expect (! (Optional<i32> (1) >= 5));
            expect (Optional<i32> (1) >= 1);
            expect (Optional<i32> (1) >= 0);

            expect (5 >= Optional<i32>());
            expect (5 >= Optional<i32> (1));
            expect (1 >= Optional<i32> (1));
            expect (! (0 >= Optional<i32> (1)));
        }
    }
};

static OptionalUnitTest optionalUnitTest;

} // namespace drx
