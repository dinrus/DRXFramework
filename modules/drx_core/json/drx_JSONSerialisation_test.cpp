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

struct TypeWithExternalUnifiedSerialisation
{
    i32 a;
    std::string b;
    std::vector<i32> c;
    std::map<std::string, i32> d;

    auto operator== (const TypeWithExternalUnifiedSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b, x.c, x.d); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithExternalUnifiedSerialisation& other) const { return ! operator== (other); }
};

template <>
struct SerialisationTraits<TypeWithExternalUnifiedSerialisation>
{
    static constexpr auto marshallingVersion = 2;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a),
                 named ("b", t.b),
                 named ("c", t.c),
                 named ("d", t.d));
    }
};

// Now that the serialiser trait is visible, it should be detected
static_assert (detail::serialisationKind<TypeWithExternalUnifiedSerialisation> == detail::SerialisationKind::external);

struct TypeWithInternalUnifiedSerialisation
{
    f64 a;
    f32 b;
    Txt c;
    StringArray d;

    auto operator== (const TypeWithInternalUnifiedSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b, x.c, x.d); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithInternalUnifiedSerialisation& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = 5;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a),
                 named ("b", t.b),
                 named ("c", t.c),
                 named ("d", t.d));
    }
};

static_assert (detail::serialisationKind<TypeWithInternalUnifiedSerialisation> == detail::SerialisationKind::internal);

struct TypeWithExternalSplitSerialisation
{
    std::optional<Txt> a;
    Array<i32> b;

    auto operator== (const TypeWithExternalSplitSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithExternalSplitSerialisation& other) const { return ! operator== (other); }
};

template <>
struct SerialisationTraits<TypeWithExternalSplitSerialisation>
{
    static constexpr auto marshallingVersion = 10;

    template <typename Archive>
    static z0 load (Archive& archive, TypeWithExternalSplitSerialisation& t)
    {
        std::optional<Txt> a;
        Array<Txt> hexStrings;
        archive (named ("a", a), named ("b", hexStrings));

        Array<i32> b;

        for (auto& i : hexStrings)
            b.add (i.getHexValue32());

        t = { a, b };
    }

    template <typename Archive>
    static z0 save (Archive& archive, const TypeWithExternalSplitSerialisation& t)
    {
        Array<Txt> hexStrings;

        for (auto& i : t.b)
            hexStrings.add ("0x" + Txt::toHexString (i));

        archive (named ("a", t.a), named ("b", hexStrings));
    }
};

// Now that the serialiser trait is visible, it should be detected
static_assert (detail::serialisationKind<TypeWithExternalSplitSerialisation> == detail::SerialisationKind::external);

// Check that serialisation kinds are correctly detected for primitives
static_assert (detail::serialisationKind<b8>               == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<  i8>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< u8>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< i16>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<u16>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< i32>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<u32>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< z64>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<zu64>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<f32>              == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<f64>             == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<std::byte>          == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<Txt>             == detail::SerialisationKind::primitive);

// Check that serialisation is disabled for types with no serialsation defined
static_assert (detail::serialisationKind<Logger>             == detail::SerialisationKind::none);
static_assert (detail::serialisationKind<CriticalSection>    == detail::SerialisationKind::none);

struct TypeWithInternalSplitSerialisation
{
    std::string a;
    Array<i32> b;

    auto operator== (const TypeWithInternalSplitSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithInternalSplitSerialisation& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = 1;

    template <typename Archive>
    static z0 load (Archive& archive, TypeWithInternalSplitSerialisation& t)
    {
        std::string a;
        Array<Txt> hexStrings;
        archive (named ("a", a), named ("b", hexStrings));

        Array<i32> b;

        for (auto& i : hexStrings)
            b.add (i.getHexValue32());

        t = { a, b };
    }

    template <typename Archive>
    static z0 save (Archive& archive, const TypeWithInternalSplitSerialisation& t)
    {
        Array<Txt> hexStrings;

        for (auto& i : t.b)
            hexStrings.add ("0x" + Txt::toHexString (i));

        archive (named ("a", t.a), named ("b", hexStrings));
    }
};

static_assert (detail::serialisationKind<TypeWithInternalSplitSerialisation> == detail::SerialisationKind::internal);

struct TypeWithBrokenObjectSerialisation
{
    i32 a;
    i32 b;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        // Archiving a named value will start reading/writing an object
        archive (named ("a", t.a));
        // Archiving a non-named value will assume that the current node is convertible
        archive (t.b);
    }
};

struct TypeWithBrokenPrimitiveSerialisation
{
    i32 a;
    i32 b;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        // Archiving a non-named value will assume that the current node is convertible
        archive (t.a);
        // Archiving a named value will fail if the current node holds a non-object type
        archive (named ("b", t.b));
    }
};

struct TypeWithBrokenArraySerialisation
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T&)
    {
        size_t size = 5;
        archive (size);

        // serialisationSize should always be serialised first!
        archive (serialisationSize (size));
    }
};

struct TypeWithBrokenNestedSerialisation
{
    i32 a;
    TypeWithBrokenObjectSerialisation b;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a), named ("b", t.b));
    }
};

struct TypeWithBrokenDynamicSerialisation
{
    std::vector<TypeWithBrokenObjectSerialisation> a;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (t.a);
    }
};

struct TypeWithVersionedSerialisation
{
    i32 a{}, b{}, c{}, d{};

    b8 operator== (const TypeWithVersionedSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b, x.c, x.d); };
        return tie (*this) == tie (other);
    }

    b8 operator!= (const TypeWithVersionedSerialisation& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = 3;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a));

        if (archive.getVersion() >= 1)
            archive (named ("b", t.b));

        if (archive.getVersion() >= 2)
            archive (named ("c", t.c));

        if (archive.getVersion() >= 3)
            archive (named ("d", t.d));
    }
};

struct TypeWithRawVarLast
{
    i32 status = 0;
    Txt message;
    var extended;

    b8 operator== (const TypeWithRawVarLast& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.status, x.message, x.extended); };
        return tie (*this) == tie (other);
    }

    b8 operator!= (const TypeWithRawVarLast& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("status", t.status),
                 named ("message", t.message),
                 named ("extended", t.extended));
    }
};

struct TypeWithRawVarFirst
{
    i32 status = 0;
    Txt message;
    var extended;

    b8 operator== (const TypeWithRawVarFirst& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.status, x.message, x.extended); };
        return tie (*this) == tie (other);
    }

    b8 operator!= (const TypeWithRawVarFirst& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("extended", t.extended),
                 named ("status", t.status),
                 named ("message", t.message));
    }
};

struct TypeWithInnerVar
{
    i32 eventId = 0;
    var payload;

    b8 operator== (const TypeWithInnerVar& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.eventId, x.payload); };
        return tie (*this) == tie (other);
    }

    b8 operator!= (const TypeWithInnerVar& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& t)
    {
        archive (named ("eventId", t.eventId),
                 named ("payload", t.payload));
    }
};

class JSONSerialisationTest final : public UnitTest
{
public:
    JSONSerialisationTest() : UnitTest ("JSONSerialisation", UnitTestCategories::json) {}

    z0 runTest() override
    {
        beginTest ("ToVar");
        {
            expectDeepEqual (ToVar::convert (false), false);
            expectDeepEqual (ToVar::convert (true), true);
            expectDeepEqual (ToVar::convert (1), 1);
            expectDeepEqual (ToVar::convert (5.0f), 5.0);
            expectDeepEqual (ToVar::convert (6LL), 6);
            expectDeepEqual (ToVar::convert ("hello world"), "hello world");
            expectDeepEqual (ToVar::convert (Txt ("hello world")), "hello world");
            expectDeepEqual (ToVar::convert (std::vector<i32> { 1, 2, 3 }), Array<var> { 1, 2, 3 });
            expectDeepEqual (ToVar::convert (TypeWithExternalUnifiedSerialisation { 7,
                                                                                    "hello world",
                                                                                    { 5, 6, 7 },
                                                                                    { { "foo", 4 }, { "bar", 5 } } }),
                             JSONUtils::makeObject ({ { "__version__", 2 },
                                                      { "a", 7 },
                                                      { "b", "hello world" },
                                                      { "c", Array<var> { 5, 6, 7 } },
                                                      { "d",
                                                        Array<var> { JSONUtils::makeObject ({ { "first", "bar" },
                                                                                              { "second", 5 } }),
                                                                     JSONUtils::makeObject ({ { "first", "foo" },
                                                                                              { "second", 4 } }) } } }));
            expectDeepEqual (ToVar::convert (TypeWithInternalUnifiedSerialisation { 7.89,
                                                                                    4.321f,
                                                                                    "custom string",
                                                                                    { "foo", "bar", "baz" } }),
                             JSONUtils::makeObject ({ { "__version__", 5 },
                                                      { "a", 7.89 },
                                                      { "b", 4.321f },
                                                      { "c", "custom string" },
                                                      { "d", Array<var> { "foo", "bar", "baz" } } }));
            expectDeepEqual (ToVar::convert (TypeWithExternalSplitSerialisation { "string", { 1, 2, 3 } }),
                             JSONUtils::makeObject ({ { "__version__", 10 },
                                                      { "a", JSONUtils::makeObject ({ { "engaged", true }, { "value", "string" } }) },
                                                      { "b", Array<var> { "0x1", "0x2", "0x3" } } }));
            expectDeepEqual (ToVar::convert (TypeWithInternalSplitSerialisation { "string", { 16, 32, 48 } }),
                             JSONUtils::makeObject ({ { "__version__", 1 },
                                                      { "a", "string" },
                                                      { "b", Array<var> { "0x10", "0x20", "0x30" } } }));

            expect (ToVar::convert (TypeWithBrokenObjectSerialisation { 1, 2 }) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenPrimitiveSerialisation { 1, 2 }) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenArraySerialisation {}) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenNestedSerialisation {}) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenDynamicSerialisation { std::vector<TypeWithBrokenObjectSerialisation> (10) }) == std::nullopt);

            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }),
                             JSONUtils::makeObject ({ { "__version__", 3 },
                                                      { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 },
                                                      { "d", 4 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withVersionIncluded (false)),
                             JSONUtils::makeObject ({ { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 },
                                                      { "d", 4 } }));
            // Requested explicit version is higher than the type's declared version
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (4)),
                             std::nullopt);
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (3)),
                             JSONUtils::makeObject ({ { "__version__", 3 },
                                                      { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 },
                                                      { "d", 4 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (2)),
                             JSONUtils::makeObject ({ { "__version__", 2 },
                                                      { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (1)),
                             JSONUtils::makeObject ({ { "__version__", 1 },
                                                      { "a", 1 },
                                                      { "b", 2 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (0)),
                             JSONUtils::makeObject ({ { "__version__", 0 },
                                                      { "a", 1 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (std::nullopt)),
                             JSONUtils::makeObject ({ { "a", 1 } }));

            expectDeepEqual (ToVar::convert (TypeWithRawVarLast { 200, "success", true }),
                             JSONUtils::makeObject ({ { "status", 200 }, { "message", "success" }, { "extended", true } }));
            expectDeepEqual (ToVar::convert (TypeWithRawVarLast { 200,
                                                                  "success",
                                                                  JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                           { "message", "failure" },
                                                                                           { "extended", true } }) }),
                             JSONUtils::makeObject ({ { "status", 200 },
                                                      { "message", "success" },
                                                      { "extended", JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                             { "message", "failure" },
                                                                                             { "extended", true } }) } }));

            expectDeepEqual (ToVar::convert (TypeWithRawVarFirst { 200, "success", true }),
                             JSONUtils::makeObject ({ { "status", 200 }, { "message", "success" }, { "extended", true } }));
            expectDeepEqual (ToVar::convert (TypeWithRawVarFirst { 200,
                                                                   "success",
                                                                   JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                            { "message", "failure" },
                                                                                            { "extended", true } }) }),
                             JSONUtils::makeObject ({ { "status", 200 },
                                                      { "message", "success" },
                                                      { "extended", JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                             { "message", "failure" },
                                                                                             { "extended", true } }) } }));

            const auto payload = JSONUtils::makeObject ({ { "foo", 1 }, { "bar", 2 } });
            expectDeepEqual (ToVar::convert (TypeWithInnerVar { 404, payload }),
                             JSONUtils::makeObject ({ { "eventId", 404 }, { "payload", payload } }));
        }

        beginTest ("FromVar");
        {
            expect (FromVar::convert<b8> (JSON::fromString ("false")) == false);
            expect (FromVar::convert<b8> (JSON::fromString ("true")) == true);
            expect (FromVar::convert<b8> (JSON::fromString ("0")) == false);
            expect (FromVar::convert<b8> (JSON::fromString ("1")) == true);
            expect (FromVar::convert<i32> (JSON::fromString ("1")) == 1);
            expect (FromVar::convert<f32> (JSON::fromString ("5.0f")) == 5.0f);
            expect (FromVar::convert<z64> (JSON::fromString ("6")) == 6);
            expect (FromVar::convert<Txt> (JSON::fromString ("\"hello world\"")) == "hello world");
            expect (FromVar::convert<std::vector<i32>> (JSON::fromString ("[1,2,3]")) == std::vector<i32> { 1, 2, 3 });
            expect (FromVar::convert<TypeWithExternalUnifiedSerialisation> (JSONUtils::makeObject ({ { "__version__", 2 },
                                                                                                     { "a", 7 },
                                                                                                     { "b", "hello world" },
                                                                                                     { "c", Array<var> { 5, 6, 7 } },
                                                                                                     { "d",
                                                                                                       Array<var> { JSONUtils::makeObject ({ { "first", "bar" },
                                                                                                                                             { "second", 5 } }),
                                                                                                                    JSONUtils::makeObject ({ { "first", "foo" },
                                                                                                                                             { "second", 4 } }) } } }))
                    == TypeWithExternalUnifiedSerialisation { 7,
                                                              "hello world",
                                                              { 5, 6, 7 },
                                                              { { "foo", 4 }, { "bar", 5 } } });

            expect (FromVar::convert<TypeWithInternalUnifiedSerialisation> (JSONUtils::makeObject ({ { "__version__", 5 },
                                                                                                     { "a", 7.89 },
                                                                                                     { "b", 4.321f },
                                                                                                     { "c", "custom string" },
                                                                                                     { "d", Array<var> { "foo", "bar", "baz" } } }))
                    == TypeWithInternalUnifiedSerialisation { 7.89,
                                                              4.321f,
                                                              "custom string",
                                                              { "foo", "bar", "baz" } });

            expect (FromVar::convert<TypeWithExternalSplitSerialisation> (JSONUtils::makeObject ({ { "__version__", 10 },
                                                                                                   { "a", JSONUtils::makeObject ({ { "engaged", true }, { "value", "string" } }) },
                                                                                                   { "b", Array<var> { "0x1", "0x2", "0x3" } } }))
                    == TypeWithExternalSplitSerialisation { "string", { 1, 2, 3 } });
            expect (FromVar::convert<TypeWithInternalSplitSerialisation> (JSONUtils::makeObject ({ { "__version__", 1 },
                                                                                                   { "a", "string" },
                                                                                                   { "b", Array<var> { "0x10", "0x20", "0x30" } } }))
                    == TypeWithInternalSplitSerialisation { "string", { 16, 32, 48 } });

            expect (FromVar::convert<TypeWithBrokenObjectSerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenPrimitiveSerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenArraySerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenNestedSerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenDynamicSerialisation> (JSON::fromString ("null")) == std::nullopt);

            expect (FromVar::convert<TypeWithInternalUnifiedSerialisation> (JSONUtils::makeObject ({ { "a", 7.89 },
                                                                                                     { "b", 4.321f } }))
                    == std::nullopt);

            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 3 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 },
                                                                                               { "c", 3 },
                                                                                               { "d", 4 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 3, 4 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 4 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 },
                                                                                               { "c", 3 },
                                                                                               { "d", 4 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 3, 4 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 2 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 },
                                                                                               { "c", 3 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 3, 0 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 1 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 0, 0 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 0 },
                                                                                               { "a", 1 } }))
                    == TypeWithVersionedSerialisation { 1, 0, 0, 0 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "a", 1 } }))
                    == TypeWithVersionedSerialisation { 1, 0, 0, 0 });

            const auto raw = JSONUtils::makeObject ({ { "status", 200 }, { "message", "success" }, { "extended", "another string" } });
            expect (FromVar::convert<TypeWithRawVarLast> (raw) == TypeWithRawVarLast { 200, "success", "another string" });
            expect (FromVar::convert<TypeWithRawVarFirst> (raw) == TypeWithRawVarFirst { 200, "success", "another string" });

            const var payloads[] { JSONUtils::makeObject ({ { "foo", 1 }, { "bar", 2 } }),
                                   var (Array<var> { 1, 2 }),
                                   var() };

            for (const auto& payload : payloads)
            {
                const auto objectWithPayload = JSONUtils::makeObject ({ { "eventId", 404 }, { "payload", payload } });
                expect (FromVar::convert<TypeWithInnerVar> (objectWithPayload) == TypeWithInnerVar { 404, payload });
            }
        }
    }

private:
    z0 expectDeepEqual (const std::optional<var>& a, const std::optional<var>& b)
    {
        const auto text = a.has_value() && b.has_value()
                        ? JSON::toString (*a) + " != " + JSON::toString (*b)
                        : Txt();
        expect (deepEqual (a, b), text);
    }

    static b8 deepEqual (const std::optional<var>& a, const std::optional<var>& b)
    {
        if (a.has_value() && b.has_value())
            return JSONUtils::deepEqual (*a, *b);

        return a == b;
    }
};

static JSONSerialisationTest jsonSerialisationTest;

} // namespace drx
