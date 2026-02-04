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

#ifndef DOXYGEN

namespace drx
{

struct TypefaceFileAndIndex
{
    File file;
    i32 index{};

    auto tie() const { return std::tuple (file, index); }

    b8 operator< (const TypefaceFileAndIndex& other) const { return tie() < other.tie(); }
};

class TypefaceFileCache : public DeletedAtShutdown
{
public:
    ~TypefaceFileCache() override
    {
        clearSingletonInstance();
    }

    template <typename Fn>
    Typeface::Ptr get (const TypefaceFileAndIndex& key, Fn&& getTypeface)
    {
        return cachedTypefaces.get (key, std::forward<Fn> (getTypeface));
    }

    DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (TypefaceFileCache)

private:
    LruCache<TypefaceFileAndIndex, Typeface::Ptr> cachedTypefaces;
};

} // namespace drx

#endif
