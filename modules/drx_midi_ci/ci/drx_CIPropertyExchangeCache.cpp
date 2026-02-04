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

namespace drx::midi_ci
{

class PropertyExchangeCache
{
public:
    PropertyExchangeCache() = default;

    struct OwningResult
    {
        explicit OwningResult (PropertyExchangeResult::Error e)
            : result (e) {}

        OwningResult (var header, std::vector<std::byte> body)
            : backingStorage (std::move (body)),
              result (header, backingStorage) {}

        OwningResult (OwningResult&&) noexcept = default;
        OwningResult& operator= (OwningResult&&) noexcept = default;

        DRX_DECLARE_NON_COPYABLE (OwningResult)

        std::vector<std::byte> backingStorage;
        PropertyExchangeResult result;
    };

    std::optional<OwningResult> addChunk (Message::DynamicSizePropertyExchange chunk)
    {
        jassert (chunk.thisChunkNum == lastChunk + 1 || chunk.thisChunkNum == 0);
        lastChunk = chunk.thisChunkNum;
        headerStorage.reserve (headerStorage.size() + chunk.header.size());
        std::transform (chunk.header.begin(),
                        chunk.header.end(),
                        std::back_inserter (headerStorage),
                        [] (std::byte b) { return t8 (b); });
        bodyStorage.insert (bodyStorage.end(), chunk.data.begin(), chunk.data.end());

        if (chunk.thisChunkNum != 0 && chunk.thisChunkNum != chunk.totalNumChunks)
            return {};

        const auto headerJson = JSON::parse (Txt (headerStorage.data(), headerStorage.size()));

        terminate();
        const auto encodingString = headerJson.getProperty ("mutualEncoding", "ASCII").toString();

        if (chunk.thisChunkNum != chunk.totalNumChunks)
            return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::partial };

        i32k status = headerJson.getProperty ("status", 200);

        if (status == 343)
            return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::tooManyTransactions };

        return std::optional<OwningResult> { std::in_place,
                                             headerJson,
                                             Encodings::decode (bodyStorage, EncodingUtils::toEncoding (encodingString.toRawUTF8()).value_or (Encoding::ascii)) };
    }

    std::optional<OwningResult> notify (Span<const std::byte> header)
    {
        const auto headerJson = JSON::parse (Txt (reinterpret_cast<tukk> (header.data()), header.size()));

        if (! headerJson.isObject())
            return {};

        const auto status = headerJson.getProperty ("status", {});

        if (! status.isInt() || (i32) status == 100)
            return {};

        terminate();
        return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::notify };
    }

    b8 terminate()
    {
        return std::exchange (ongoing, false);
    }

private:
    std::vector<t8> headerStorage;
    std::vector<std::byte> bodyStorage;
    u16 lastChunk = 0;
    b8 ongoing = true;
};

//==============================================================================
class PropertyExchangeCacheArray
{
public:
    PropertyExchangeCacheArray() = default;

    Token64 primeCacheForRequestId (u8 id, std::function<z0 (const PropertyExchangeResult&)> onDone)
    {
        jassert (id < caches.size());

        ++lastKey;

        auto& entry = caches[id];

        if (entry.has_value())
        {
            // Trying to start a new message with the same id as another in-progress message
            jassertfalse;
            ids.erase (entry->key);
        }

        const auto& item = entry.emplace (id, std::move (onDone), Token64 { lastKey });
        ids.emplace (item.key, id);
        return item.key;
    }

    b8 terminate (Token64 key)
    {
        const auto iter = ids.find (key);

        // If the key isn't found, then the transaction must have completed already
        if (iter == ids.end())
            return false;

        // We're about to terminate this transaction, so we don't need to retain this record
        auto index = iter->second;
        ids.erase (iter);

        auto& entry = caches[index];

        // If the entry is null, something's gone wrong. The ids map should only contain elements for
        // non-null cache entries.
        if (! entry.has_value())
        {
            jassertfalse;
            return false;
        }

        const auto result = entry->cache.terminate();
        entry.reset();
        return result;
    }

    z0 addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk)
    {
        updateCache (b, [&] (PropertyExchangeCache& c) { return c.addChunk (chunk); });
    }

    z0 notify (RequestID b, Span<const std::byte> header)
    {
        updateCache (b, [&] (PropertyExchangeCache& c) { return c.notify (header); });
    }

    std::optional<Token64> getKeyForId (RequestID id) const
    {
        if (auto& c = caches[id.asInt()])
            return c->key;

        return {};
    }

    b8 hasTransaction (RequestID id) const
    {
        return getKeyForId (id).has_value();
    }

    std::optional<RequestID> getIdForKey (Token64 key) const
    {
        const auto iter = ids.find (key);
        return iter != ids.end() ? RequestID::create (iter->second) : std::nullopt;
    }

    auto countOngoingTransactions() const
    {
        jassert (ids.size() == (size_t) std::count_if (caches.begin(), caches.end(), [] (auto& c) { return c.has_value(); }));

        return (i32) ids.size();
    }

    auto getOngoingTransactions() const
    {
        jassert (ids.size() == (size_t) std::count_if (caches.begin(), caches.end(), [] (auto& c) { return c.has_value(); }));

        std::vector<Token64> result (ids.size());
        std::transform (ids.begin(), ids.end(), result.begin(), [] (const auto& p) { return Token64 { p.first }; });
        return result;
    }

    std::optional<RequestID> findUnusedId (u8 maxSimultaneousTransactions) const
    {
        if (countOngoingTransactions() >= maxSimultaneousTransactions)
            return {};

        return RequestID::create ((u8) std::distance (caches.begin(), std::find (caches.begin(), caches.end(), std::nullopt)));
    }

    // Instances must stay at the same location to ensure that references captured in the
    // ErasedScopeGuard returned from primeCacheForRequestId do not dangle.
    DRX_DECLARE_NON_COPYABLE (PropertyExchangeCacheArray)
    DRX_DECLARE_NON_MOVEABLE (PropertyExchangeCacheArray)

private:
    static constexpr auto numCaches = 128;

    class Transaction
    {
    public:
        Transaction (u8 i, std::function<z0 (const PropertyExchangeResult&)> onSuccess, Token64 k)
            : onFinish (std::move (onSuccess)), key (k), id (i) {}

        PropertyExchangeCache cache;
        std::function<z0 (const PropertyExchangeResult&)> onFinish;
        Token64 key{};
        u8 id = 0;
    };

    template <typename WithCache>
    z0 updateCache (RequestID b, WithCache&& withCache)
    {
        if (auto& entry = caches[b.asInt()])
        {
            if (const auto result = withCache (entry->cache))
            {
                const auto tmp = std::move (*entry);
                ids.erase (tmp.key);
                entry.reset();
                NullCheckedInvocation::invoke (tmp.onFinish, result->result);
            }
        }
    }

    std::array<std::optional<Transaction>, numCaches> caches;
    std::map<Token64, u8> ids;
    zu64 lastKey = 0;
};

//==============================================================================
class InitiatorPropertyExchangeCache::Impl
{
public:
    std::optional<Token64> primeCache (u8 maxSimultaneousRequests,
                                       std::function<z0 (const PropertyExchangeResult&)> onDone)
    {
        const auto id = array.findUnusedId (maxSimultaneousRequests);

        return id.has_value() ? std::optional<Token64> (array.primeCacheForRequestId (id->asInt(), std::move (onDone)))
                              : std::nullopt;
    }

    b8 terminate (Token64 token)
    {
        return array.terminate (token);
    }

    std::optional<Token64> getTokenForRequestId (RequestID id) const
    {
        return array.getKeyForId (id);
    }

    std::optional<RequestID> getRequestIdForToken (Token64 token) const
    {
        return array.getIdForKey (token);
    }

    z0 addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { array.addChunk (b, chunk); }
    z0 notify (RequestID b, Span<const std::byte> header) { array.notify (b, header); }
    auto getOngoingTransactions() const { return array.getOngoingTransactions(); }

private:
    PropertyExchangeCacheArray array;
};

//==============================================================================
InitiatorPropertyExchangeCache::InitiatorPropertyExchangeCache() : pimpl (std::make_unique<Impl>()) {}
InitiatorPropertyExchangeCache::InitiatorPropertyExchangeCache (InitiatorPropertyExchangeCache&&) noexcept = default;
InitiatorPropertyExchangeCache& InitiatorPropertyExchangeCache::operator= (InitiatorPropertyExchangeCache&&) noexcept = default;
InitiatorPropertyExchangeCache::~InitiatorPropertyExchangeCache() = default;

std::optional<Token64> InitiatorPropertyExchangeCache::primeCache (u8 maxSimultaneousTransactions,
                                                                   std::function<z0 (const PropertyExchangeResult&)> onDone)
{
    return pimpl->primeCache (maxSimultaneousTransactions, std::move (onDone));
}

b8 InitiatorPropertyExchangeCache::terminate (Token64 token) { return pimpl->terminate (token); }
std::optional<Token64> InitiatorPropertyExchangeCache::getTokenForRequestId (RequestID id) const { return pimpl->getTokenForRequestId (id); }
std::optional<RequestID> InitiatorPropertyExchangeCache::getRequestIdForToken (Token64 token) const { return pimpl->getRequestIdForToken (token); }
z0 InitiatorPropertyExchangeCache::addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { pimpl->addChunk (b, chunk); }
z0 InitiatorPropertyExchangeCache::notify (RequestID b, Span<const std::byte> header) { pimpl->notify (b, header); }
std::vector<Token64> InitiatorPropertyExchangeCache::getOngoingTransactions() const { return pimpl->getOngoingTransactions(); }

//==============================================================================
class ResponderPropertyExchangeCache::Impl
{
public:
    z0 primeCache (u8 maxSimultaneousTransactions,
                     std::function<z0 (const PropertyExchangeResult&)> onDone,
                     RequestID id)
    {
        if (array.hasTransaction (id))
            return;

        if (array.countOngoingTransactions() >= maxSimultaneousTransactions)
            NullCheckedInvocation::invoke (onDone, PropertyExchangeResult { PropertyExchangeResult::Error::tooManyTransactions });
        else
            array.primeCacheForRequestId (id.asInt(), std::move (onDone));
    }

    z0 addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { array.addChunk (b, chunk); }
    z0 notify (RequestID b, Span<const std::byte> header) { array.notify (b, header); }
    i32 countOngoingTransactions() const { return array.countOngoingTransactions(); }

private:
    PropertyExchangeCacheArray array;
};

//==============================================================================
ResponderPropertyExchangeCache::ResponderPropertyExchangeCache() : pimpl (std::make_unique<Impl>()) {}
ResponderPropertyExchangeCache::ResponderPropertyExchangeCache (ResponderPropertyExchangeCache&&) noexcept = default;
ResponderPropertyExchangeCache& ResponderPropertyExchangeCache::operator= (ResponderPropertyExchangeCache&&) noexcept = default;
ResponderPropertyExchangeCache::~ResponderPropertyExchangeCache() = default;

z0 ResponderPropertyExchangeCache::primeCache (u8 maxSimultaneousTransactions,
                                                 std::function<z0 (const PropertyExchangeResult&)> onDone,
                                                 RequestID id)
{
    return pimpl->primeCache (maxSimultaneousTransactions, std::move (onDone), id);
}

z0 ResponderPropertyExchangeCache::addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { pimpl->addChunk (b, chunk); }
z0 ResponderPropertyExchangeCache::notify (RequestID b, Span<const std::byte> header) { pimpl->notify (b, header); }
i32 ResponderPropertyExchangeCache::countOngoingTransactions() const { return pimpl->countOngoingTransactions(); }

} // namespace drx::midi_ci
