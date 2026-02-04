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

#if ! defined (_MSC_VER) && ! defined (__uuidof)
 #ifdef __uuidof
  #undef __uuidof
 #endif

 template <typename Type> struct UUIDGetter { static CLSID get() { jassertfalse; return {}; } };
 #define __uuidof(x)  UUIDGetter<x>::get()

 template <>
 struct UUIDGetter<::IUnknown>
 {
     static CLSID get()     { return { 0, 0, 0, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } }; }
 };

 #define DRX_DECLARE_UUID_GETTER(name, uuid) \
    template <> struct UUIDGetter<name> { static CLSID get()  { return uuidFromString (uuid); } };

 #define DRX_COMCLASS(name, guid) \
    struct name; \
    DRX_DECLARE_UUID_GETTER (name, guid) \
    struct name

#else
 #define DRX_DECLARE_UUID_GETTER(name, uuid)
 #define DRX_COMCLASS(name, guid)       struct DECLSPEC_UUID (guid) name
#endif

#define DRX_IUNKNOWNCLASS(name, guid)   DRX_COMCLASS(name, guid) : public IUnknown
#define DRX_COMRESULT                   HRESULT STDMETHODCALLTYPE
#define DRX_COMCALL                     virtual HRESULT STDMETHODCALLTYPE

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

inline GUID uuidFromString (tukk s) noexcept
{
    u32 ints[4] = {};

    for (u32 digitIndex = 0; digitIndex < 32;)
    {
        auto c = (u32) *s++;
        u32 digit;

        if (c >= '0' && c <= '9')       digit = c - '0';
        else if (c >= 'a' && c <= 'f')  digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')  digit = c - 'A' + 10;
        else if (c == '-')              continue;
        else break;

        ints[digitIndex / 8] |= (digit << 4 * (7 - (digitIndex & 7)));
        ++digitIndex;
    }

    return { ints[0],
             (u16) (ints[1] >> 16),
             (u16) ints[1],
             { (u8) (ints[2] >> 24), (u8) (ints[2] >> 16), (u8) (ints[2] >> 8), (u8) ints[2],
               (u8) (ints[3] >> 24), (u8) (ints[3] >> 16), (u8) (ints[3] >> 8), (u8) ints[3] }};
}

//==============================================================================
/** A simple COM smart pointer.

    @tags{Core}
*/
template <class ComClass>
class ComSmartPtr
{
public:
    ComSmartPtr() noexcept = default;
    ComSmartPtr (std::nullptr_t) noexcept {}

    template <typename U>
    ComSmartPtr (const ComSmartPtr<U>& other) : ComSmartPtr (other, true) {}
    ComSmartPtr (const ComSmartPtr&    other) : ComSmartPtr (other, true) {}

    ~ComSmartPtr() noexcept { release(); }

    template <typename U>
    ComSmartPtr& operator= (const ComSmartPtr<U>& newP) { ComSmartPtr copy { newP }; std::swap (copy.p, p); return *this; }
    ComSmartPtr& operator= (const ComSmartPtr&    newP) { ComSmartPtr copy { newP }; std::swap (copy.p, p); return *this; }

    ComClass* get() const noexcept { return p; }

    operator ComClass*() const noexcept     { return p; }
    ComClass& operator*() const noexcept    { return *p; }
    ComClass* operator->() const noexcept   { return p; }

    // Releases and nullifies this pointer and returns its address
    ComClass** resetAndGetPointerAddress()
    {
        release();
        return &p;
    }

    HRESULT CoCreateInstance (REFCLSID classUUID, DWORD dwClsContext = CLSCTX_INPROC_SERVER)
    {
        auto hr = ::CoCreateInstance (classUUID, nullptr, dwClsContext, __uuidof (ComClass), (uk*) resetAndGetPointerAddress());
        jassert (hr != CO_E_NOTINITIALIZED); // You haven't called CoInitialize for the current thread!
        return hr;
    }

    template <class OtherComClass>
    HRESULT QueryInterface (REFCLSID classUUID, ComSmartPtr<OtherComClass>& destObject) const
    {
        if (p == nullptr)
            return E_POINTER;

        return p->QueryInterface (classUUID, (uk*) destObject.resetAndGetPointerAddress());
    }

    template <class OtherComClass>
    HRESULT QueryInterface (ComSmartPtr<OtherComClass>& destObject) const
    {
        return this->QueryInterface (__uuidof (OtherComClass), destObject);
    }

    template <class OtherComClass>
    ComSmartPtr<OtherComClass> getInterface() const
    {
        ComSmartPtr<OtherComClass> destObject;

        if (const auto hr = QueryInterface (destObject); FAILED (hr))
            return {};

        return destObject;
    }

    /** Increments refcount. */
    static auto addOwner (ComClass* t)
    {
        return ComSmartPtr (t, true);
    }

    /** Does not initially increment refcount; assumes t has a positive refcount. */
    static auto becomeOwner (ComClass* t)
    {
        return ComSmartPtr (t, false);
    }

private:
    template <typename U>
    friend class ComSmartPtr;

    ComSmartPtr (ComClass* object, b8 autoAddRef) noexcept
        : p (object)
    {
        if (p != nullptr && autoAddRef)
            p->AddRef();
    }

    z0 release()
    {
        if (auto* q = std::exchange (p, nullptr))
            q->Release();
    }

    ComClass** operator&() noexcept; // private to avoid it being used accidentally

    ComClass* p = nullptr;
};

/** Increments refcount. */
template <class ObjectType>
auto addComSmartPtrOwner (ObjectType* t)
{
    return ComSmartPtr<ObjectType>::addOwner (t);
}

/** Does not initially increment refcount; assumes t has a positive refcount. */
template <class ObjectType>
auto becomeComSmartPtrOwner (ObjectType* t)
{
    return ComSmartPtr<ObjectType>::becomeOwner (t);
}

//==============================================================================
template <class First, class... ComClasses>
class ComBaseClassHelperBase   : public First, public ComClasses...
{
public:
    ComBaseClassHelperBase() = default;
    virtual ~ComBaseClassHelperBase() = default;

    ULONG STDMETHODCALLTYPE AddRef()    override { return ++refCount; }
    ULONG STDMETHODCALLTYPE Release()   override { auto r = --refCount; if (r == 0) delete this; return r; }

protected:
    ULONG refCount = 1;

    DRX_COMRESULT QueryInterface (REFIID refId, uk* result) override
    {
        if (refId == __uuidof (IUnknown))
            return castToType<First> (result);

        *result = nullptr;
        return E_NOINTERFACE;
    }

    template <class Type>
    DRX_COMRESULT castToType (uk* result)
    {
        this->AddRef();
        *result = dynamic_cast<Type*> (this);

        return S_OK;
    }
};

/** Handy base class for writing COM objects, providing ref-counting and a basic QueryInterface method.

    @tags{Core}
*/
template <class... ComClasses>
class ComBaseClassHelper   : public ComBaseClassHelperBase<ComClasses...>
{
public:
    ComBaseClassHelper() = default;

    DRX_COMRESULT QueryInterface (REFIID refId, uk* result) override
    {
        const std::tuple<IID, uk> bases[]
        {
            std::make_tuple (__uuidof (ComClasses),
                             static_cast<uk> (static_cast<ComClasses*> (this)))...
        };

        for (const auto& base : bases)
        {
            if (refId == std::get<0> (base))
            {
                this->AddRef();
                *result = std::get<1> (base);
                return S_OK;
            }
        }

        return ComBaseClassHelperBase<ComClasses...>::QueryInterface (refId, result);
    }
};

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx
