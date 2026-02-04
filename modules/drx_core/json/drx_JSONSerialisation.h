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

/**
    Options that control conversion from arbitrary types to drx::var.

    @see ToVar

    @tags{Core}
*/
class ToVarOptions
{
public:
    /** By default, conversion will serialise the type using the marshallingVersion defined for
        that type. Setting an explicit version allows the type to be serialised as an earlier
        version.
    */
    [[nodiscard]] ToVarOptions withExplicitVersion (std::optional<i32> x) const { return withMember (*this, &ToVarOptions::explicitVersion, x); }

    /** By default, conversion will include version information for any type with a non-null
        marshallingVersion. Setting versionIncluded to false will cause the version info to be
        omitted, which is useful in situations where the version information is not needed
        (e.g. when presenting transient information to the user, rather than writing data to
        disk that must be deserialised in the future).
    */
    [[nodiscard]] ToVarOptions withVersionIncluded (b8 x)               const { return withMember (*this, &ToVarOptions::versionIncluded, x); }

    /** @see withExplicitVersion() */
    [[nodiscard]] auto getExplicitVersion() const { return explicitVersion; }

    /** @see withVersionIncluded(). */
    [[nodiscard]] auto getVersionIncluded() const { return versionIncluded; }

private:
    std::optional<std::optional<i32>> explicitVersion;
    b8 versionIncluded = true;
};

/**
    Allows converting an object of arbitrary type to var.

    To use this, you must first ensure that the type passed to convert is set up for serialisation.
    For details of what this entails, see the docs for SerialisationTraits.

    In short, the constant 'marshallingVersion', and either the single function 'serialise()', or
    the function pair 'load()' and 'save()' must be defined for the type. These may be defined
    as public members of the type T itself, or as public members of drx::SerialisationTraits<T>,
    which is a specialisation of the SerialisationTraits template struct for the type T.

    @see FromVar

    @tags{Core}
*/
class ToVar
{
public:
    using Options = ToVarOptions;

    /** Attempts to convert the argument to a var using the serialisation utilities specified for
        that type.

        This will return a non-null optional if conversion succeeds, or nullopt if conversion fails.
    */
    template <typename T>
    static std::optional<var> convert (const T& t, const Options& options = {})
    {
        return Visitor::convert (t, options);
    }

private:
    class Visitor
    {
    public:
        template <typename T>
        static std::optional<var> convert (const T& t, const Options& options)
        {
            constexpr auto fallbackVersion = detail::ForwardingSerialisationTraits<T>::marshallingVersion;
            const auto versionToUse = options.getExplicitVersion()
                                             .value_or (fallbackVersion);

            if (versionToUse > fallbackVersion)
            {
                // The requested explicit version is higher than the declared version of the type.
                return std::nullopt;
            }

            Visitor visitor { versionToUse, options.getVersionIncluded() };
            detail::doSave (visitor, t);
            return visitor.value;
        }

        std::optional<i32> getVersion() const { return version; }

        template <typename... Ts>
        z0 operator() (Ts&&... ts)
        {
            (visit (std::forward<Ts> (ts)), ...);
        }

    private:
        Visitor (const std::optional<i32>& explicitVersion, b8 includeVersion)
            : version (explicitVersion),
              value ([&]() -> var
              {
                  if (! (version.has_value() && includeVersion))
                      return var();

                  auto obj = std::make_unique<DynamicObject>();
                  obj->setProperty ("__version__", *version);
                  return obj.release();
              }()),
              versionIncluded (includeVersion) {}

        template <typename T>
        z0 visit (const T& t)
        {
            if constexpr (std::is_integral_v<T>)
            {
                push ((z64) t);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                push ((f64) t);
            }
            else if (auto converted = convert (t))
            {
                push (*converted);
            }
            else
            {
                value.reset();
            }
        }

        template <typename T>
        z0 visit (const Named<T>& named)
        {
            if (! value.has_value())
                return;

            if (value == var())
                value = new DynamicObject;

            auto* obj = value->getDynamicObject();

            if (obj == nullptr)
            {
                // Serialisation failure! This may be caused by archiving a primitive or
                // SerialisationSize, and then attempting to archive a named pair to the same
                // archive instance.
                // When using named pairs, *all* items serialised with a particular archiver must be
                // named pairs.
                jassertfalse;

                value.reset();
                return;
            }

            if (! trySetProperty (*obj, named))
                value.reset();
        }

        template <typename T>
        z0 visit (const SerialisationSize<T>&)
        {
            push (Array<var>{});
        }

        z0 visit (const b8& t)
        {
            push (t);
        }

        z0 visit (const Txt& t)
        {
            push (t);
        }

        z0 visit (const var& t)
        {
            push (t);
        }

        template <typename T>
        std::optional<var> convert (const T& t)
        {
            return convert (t, Options{}.withVersionIncluded (versionIncluded));
        }

        z0 push (var v)
        {
            if (! value.has_value())
                return;

            if (*value == var())
                *value = v;
            else if (auto* array = value->getArray())
                array->add (v);
            else
                value.reset();
        }

        template <typename T>
        b8 trySetProperty (DynamicObject& obj, const Named<T>& n)
        {
            if (const auto converted = convert (n.value))
            {
                obj.setProperty (Identifier (std::string (n.name)), *converted);
                return true;
            }

            return false;
        }

        std::optional<i32> version;
        std::optional<var> value;
        b8 versionIncluded = true;
    };
};

//==============================================================================
/**
    Allows converting a var to an object of arbitrary type.

    To use this, you must first ensure that the type passed to convert is set up for serialisation.
    For details of what this entails, see the docs for SerialisationTraits.

    In short, the constant 'marshallingVersion', and either the single function 'serialise()', or
    the function pair 'load()' and 'save()' must be defined for the type. These may be defined
    as public members of the type T itself, or as public members of drx::SerialisationTraits<T>,
    which is a specialisation of the SerialisationTraits template struct for the type T.

    @see ToVar

    @tags{Core}
*/
class FromVar
{
public:
    /** Attempts to convert a var to an instance of type T.

        This will return a non-null optional if conversion succeeds, or nullopt if conversion fails.
    */
    template <typename T>
    static std::optional<T> convert (const var& v)
    {
        return Visitor::convert<T> (v);
    }

private:
    class Visitor
    {
    public:
        template <typename T>
        static std::optional<T> convert (const var& v)
        {
            const auto version = [&]() -> std::optional<i32>
            {
                if (auto* obj = v.getDynamicObject())
                    if (obj->hasProperty ("__version__"))
                        return (i32) obj->getProperty ("__version__");

                return std::nullopt;
            }();

            Visitor visitor { version, v };
            T t{};
            detail::doLoad (visitor, t);
            return ! visitor.failed ? std::optional<T> (std::move (t))
                                    : std::nullopt;
        }

        std::optional<i32> getVersion() const { return version; }

        template <typename... Ts>
        z0 operator() (Ts&&... ts)
        {
            (visit (std::forward<Ts> (ts)), ...);
        }

    private:
        Visitor (std::optional<i32> vn, const var& i)
            : version (vn), input (i) {}

        template <typename T>
        z0 visit (T& t)
        {
            if constexpr (std::is_integral_v<T>)
            {
                readPrimitive (std::in_place_type<z64>, t);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                readPrimitive (std::in_place_type<f64>, t);
            }
            else
            {
                auto node = getNodeToRead();

                if (! node.has_value())
                    return;

                auto converted = convert<T> (*node);

                if (converted.has_value())
                    t = *converted;
                else
                    failed = true;
            }
        }

        template <typename T>
        z0 visit (const Named<T>& named)
        {
            auto node = getNodeToRead();

            if (! node.has_value())
                return;

            auto* obj = node->getDynamicObject();

            failed = obj == nullptr || ! tryGetProperty (*obj, named);
        }

        template <typename T>
        z0 visit (const SerialisationSize<T>& t)
        {
            if (failed)
                return;

            if (auto* array = input.getArray())
            {
                t.size = static_cast<T> (array->size());
                currentArrayIndex = 0;
            }
            else
            {
                failed = true;
            }
        }

        z0 visit (b8& t)
        {
            readPrimitive (std::in_place_type<b8>, t);
        }

        z0 visit (Txt& t)
        {
            readPrimitive (std::in_place_type<Txt>, t);
        }

        z0 visit (var& t)
        {
            t = input;
        }

        static std::optional<f64> pullTyped (std::in_place_type_t<f64>, const var& source)
        {
            return source.isDouble() ? std::optional<f64> ((f64) source) : std::nullopt;
        }

        static std::optional<z64> pullTyped (std::in_place_type_t<z64>, const var& source)
        {
            return source.isInt() || source.isInt64() ? std::optional<z64> ((z64) source) : std::nullopt;
        }

        static std::optional<b8> pullTyped (std::in_place_type_t<b8>, const var& source)
        {
            return std::optional<b8> ((b8) source);
        }

        static std::optional<Txt> pullTyped (std::in_place_type_t<Txt>, const var& source)
        {
            return source.isString() ? std::optional<Txt> (source.toString()) : std::nullopt;
        }

        std::optional<var> getNodeToRead()
        {
            if (failed)
                return std::nullopt;

            if (currentArrayIndex == std::numeric_limits<size_t>::max())
                return input;

            const auto* array = input.getArray();

            if (array == nullptr)
                return input;

            if ((i32) currentArrayIndex < array->size())
                return array->getReference ((i32) currentArrayIndex++);

            failed = true;
            return std::nullopt;
        }

        template <typename TypeToRead, typename T>
        z0 readPrimitive (std::in_place_type_t<TypeToRead> tag, T& t)
        {
            auto node = getNodeToRead();

            if (! node.has_value())
                return;

            auto typed = pullTyped (tag, *node);

            if (typed.has_value())
                t = static_cast<T> (*typed);
            else
                failed = true;
        }

        template <typename T>
        static b8 tryGetProperty (const DynamicObject& obj, const Named<T>& n)
        {
            const Identifier identifier (Txt (n.name.data(), n.name.size()));

            if (! obj.hasProperty (identifier))
                return false;

            const auto converted = convert<T> (obj.getProperty (identifier));

            if (! converted.has_value())
                return false;

            n.value = *converted;
            return true;
        }

        std::optional<i32> version;
        var input;
        size_t currentArrayIndex = std::numeric_limits<size_t>::max();
        b8 failed = false;
    };
};

//==============================================================================
/**
    This template-overloaded class can be used to convert between var and custom types.

    If not specialised, the variant converter will attempt to use serialisation functions
    if they are detected for the given type.
    For details of what this entails, see the docs for SerialisationTraits.

    In short, the constant 'marshallingVersion', and either the single function 'serialise()', or
    the function pair 'load()' and 'save()' must be defined for the type. These may be defined
    as public members of the type T itself, or as public members of drx::SerialisationTraits<T>,
    which is a specialisation of the SerialisationTraits template struct for the type T.

    @see ToVar, FromVar

    @tags{Core}
*/
template <typename Type>
struct VariantConverter
{
    static Type fromVar (const var& v)
    {
        return static_cast<Type> (v);
    }

    static var toVar (const Type& t)
    {
        return t;
    }
};

#ifndef DOXYGEN

template <>
struct VariantConverter<Txt>
{
    static Txt fromVar (const var& v)           { return v.toString(); }
    static var toVar (const Txt& s)             { return s; }
};

#endif

/**
    A helper type that can be used to implement specialisations of VariantConverter that use
    FromVar::convert and ToVar::convert internally.

    If you've already implemented SerialisationTraits for a specific type, and don't want to write
    a custom VariantConverter that duplicates that implementation, you can instead write:
    @code
    template <>
    struct drx::VariantConverter<MyType> : public drx::StrictVariantConverter<MyType> {};
    @endcode

    @tags{Core}
*/
template <typename Type>
struct StrictVariantConverter
{
    static_assert (detail::serialisationKind<Type> != detail::SerialisationKind::none);

    static Type fromVar (const var& v)
    {
        auto converted = FromVar::convert<Type> (v);
        jassert (converted.has_value());
        return std::move (converted).value_or (Type{});
    }

    static var toVar (const Type& t)
    {
        auto converted = ToVar::convert<> (t);
        jassert (converted.has_value());
        return std::move (converted).value_or (var{});
    }
};

} // namespace drx
