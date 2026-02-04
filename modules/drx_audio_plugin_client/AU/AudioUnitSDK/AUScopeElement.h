/*!
	@file		AudioUnitSDK/AUScopeElement.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUScopeElement_h
#define AudioUnitSDK_AUScopeElement_h

// module
#include <AudioUnitSDK/AUBuffer.h>
#include <AudioUnitSDK/AUUtility.h>
#include <AudioUnitSDK/ComponentBase.h>

// OS
#include <AudioToolbox/AudioUnit.h>

// std
#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

namespace ausdk {

class AUBase;

/// Wrap an atomic in a copy-constructible/assignable object. This allows storing atomic values in a
/// vector (not directly possible since atomics are not copy-constructible/assignable).
template <typename T>
class AtomicValue {
public:
	AtomicValue() = default;
	explicit AtomicValue(T val) : mValue{ val } {}
	~AtomicValue() = default;

	AtomicValue(const AtomicValue& other) : mValue{ other.mValue.load() } {}
	AtomicValue(AtomicValue&& other) noexcept : mValue{ other.mValue.load() } {}

	AtomicValue& operator=(const AtomicValue& other)
	{
		if (&other != this) {
			mValue.store(other.mValue.load());
		}
		return *this;
	}

	AtomicValue& operator=(AtomicValue&& other) noexcept
	{
		mValue.store(other.mValue.load());
		return *this;
	}

	T load(std::memory_order m = std::memory_order_seq_cst) const { return mValue.load(m); }
	z0 store(T v, std::memory_order m = std::memory_order_seq_cst) { mValue.store(v, m); }

	operator T() const { return load(); } // NOLINT implicit conversions OK

	AtomicValue& operator=(T value)
	{
		store(value);
		return *this;
	}

private:
	std::atomic<T> mValue{};
};

/// A bare-bones reinvention of boost::flat_map, just enough to hold parameters in sorted vectors.
template <typename Key, typename Value>
class flat_map {
	using KVPair = std::pair<Key, Value>;
	using Impl = std::vector<std::pair<Key, Value>>;

	static b8 keyless(const KVPair& item, Key k) { return k > item.first; }

	Impl mImpl;

public:
	using iterator = typename Impl::iterator;
	using const_iterator = typename Impl::const_iterator;

	[[nodiscard]] b8 empty() const { return mImpl.empty(); }
	[[nodiscard]] size_t size() const { return mImpl.size(); }
	[[nodiscard]] const_iterator begin() const { return mImpl.begin(); }
	[[nodiscard]] const_iterator end() const { return mImpl.end(); }
	iterator begin() { return mImpl.begin(); }
	iterator end() { return mImpl.end(); }
	const_iterator cbegin() { return mImpl.cbegin(); }
	const_iterator cend() { return mImpl.cend(); }

	[[nodiscard]] const_iterator lower_bound(Key k) const
	{
		return std::lower_bound(mImpl.begin(), mImpl.end(), k, keyless);
	}

	iterator lower_bound(Key k) { return std::lower_bound(mImpl.begin(), mImpl.end(), k, keyless); }

	[[nodiscard]] const_iterator find(Key k) const
	{
		auto iter = lower_bound(k);
		if (iter != mImpl.end()) {
			if ((*iter).first != k) {
				iter = mImpl.end();
			}
		}
		return iter;
	}

	iterator find(Key k)
	{
		auto iter = lower_bound(k);
		if (iter != mImpl.end()) {
			if ((*iter).first != k) {
				iter = mImpl.end();
			}
		}
		return iter;
	}

	class ItemProxy {
	public:
		ItemProxy(flat_map& map, Key k) : mMap{ map }, mKey{ k } {}

		operator Value() const // NOLINT implicit conversion is OK
		{
			const auto iter = mMap.find(mKey);
			if (iter == mMap.end()) {
				throw std::runtime_error("Invalid map key");
			}
			return (*iter).second;
		}

		ItemProxy& operator=(const Value& v)
		{
			const auto iter = mMap.lower_bound(mKey);
			if (iter != mMap.end() && (*iter).first == mKey) {
				(*iter).second = v;
			} else {
				mMap.mImpl.insert(iter, { mKey, v });
			}
			return *this;
		}

	private:
		flat_map& mMap;
		const Key mKey;
	};

	ItemProxy operator[](Key k) { return ItemProxy{ *this, k }; }
};

// ____________________________________________________________________________
//
class AUIOElement;

/// An organizational unit for parameters, with a name.
class AUElement {
	using ParameterValue = AtomicValue<f32>;
	using ParameterMap = flat_map<AudioUnitParameterID, ParameterValue>;

public:
	explicit AUElement(AUBase& audioUnit) : mAudioUnit(audioUnit), mUseIndexedParameters(false) {}

	AUSDK_DEPRECATED("Construct with a reference")
	explicit AUElement(AUBase* audioUnit) : AUElement(*audioUnit) {}

	AUElement(const AUElement&) = delete;
	AUElement(AUElement&&) = delete;
	AUElement& operator=(const AUElement&) = delete;
	AUElement& operator=(AUElement&&) = delete;

	virtual ~AUElement() = default;

	virtual UInt32 GetNumberOfParameters()
	{
		return mUseIndexedParameters ? static_cast<UInt32>(mIndexedParameters.size())
									 : static_cast<UInt32>(mParameters.size());
	}
	virtual z0 GetParameterList(AudioUnitParameterID* outList);
	[[nodiscard]] b8 HasParameterID(AudioUnitParameterID paramID) const;
	[[nodiscard]] AudioUnitParameterValue GetParameter(AudioUnitParameterID paramID) const;

	// Only set okWhenInitialized to true when you know the outside world cannot access this
	// element. Otherwise the parameter map could get corrupted.
	z0 SetParameter(AudioUnitParameterID paramID, AudioUnitParameterValue value,
		b8 okWhenInitialized = false);

	// Only set okWhenInitialized to true when you know the outside world cannot access this
	// element. Otherwise the parameter map could get corrupted. N.B. This only handles
	// immediate parameters. Override to implement ramping. Called from
	// AUBase::ProcessForScheduledParams.
	virtual z0 SetScheduledEvent(AudioUnitParameterID paramID,
		const AudioUnitParameterEvent& inEvent, UInt32 inSliceOffsetInBuffer,
		UInt32 inSliceDurationFrames, b8 okWhenInitialized = false);

	[[nodiscard]] AUBase& GetAudioUnit() const noexcept { return mAudioUnit; }

	z0 SaveState(AudioUnitScope scope, CFMutableDataRef data);
	const UInt8* RestoreState(const UInt8* state);

	[[nodiscard]] Owned<CFStringRef> GetName() const { return mElementName; }
	z0 SetName(CFStringRef inName) { mElementName = inName; }

	[[nodiscard]] b8 HasName() const { return *mElementName != nil; }

	virtual z0 UseIndexedParameters(UInt32 inNumberOfParameters);

	virtual AUIOElement* AsIOElement() { return nullptr; }

private:
	// --
	AUBase& mAudioUnit;
	ParameterMap mParameters;
	b8 mUseIndexedParameters;
	std::vector<ParameterValue> mIndexedParameters;
	Owned<CFStringRef> mElementName;
};


// ____________________________________________________________________________
//

/// A subclass of AUElement which represents an input or output bus, and has an associated
/// audio format and buffers.
class AUIOElement : public AUElement {
public:
	explicit AUIOElement(AUBase& audioUnit);

	AUIOElement(AUBase& audioUnit, const AudioStreamBasicDescription& format)
		: AUIOElement{ audioUnit }
	{
		mStreamFormat = format;
	}

	AUSDK_DEPRECATED("Construct with a reference")
	explicit AUIOElement(AUBase* audioUnit) : AUIOElement(*audioUnit) {}

	[[nodiscard]] const AudioStreamBasicDescription& GetStreamFormat() const noexcept
	{
		return mStreamFormat;
	}

	virtual OSStatus SetStreamFormat(const AudioStreamBasicDescription& format);

	virtual z0 AllocateBuffer(UInt32 inFramesToAllocate = 0);

	z0 DeallocateBuffer();

	/// Determines (via subclass override) whether the element's buffer list needs to be allocated.
	[[nodiscard]] virtual b8 NeedsBufferSpace() const = 0;

	z0 SetWillAllocateBuffer(b8 inFlag) noexcept { mWillAllocate = inFlag; }

	[[nodiscard]] b8 WillAllocateBuffer() const noexcept { return mWillAllocate; }

	AudioBufferList& PrepareBuffer(UInt32 nFrames)
	{
		if (mWillAllocate) {
			return mIOBuffer.PrepareBuffer(mStreamFormat, nFrames);
		}
		Throw(kAudioUnitErr_InvalidPropertyValue);
	}

	AudioBufferList& PrepareNullBuffer(UInt32 nFrames)
	{
		return mIOBuffer.PrepareNullBuffer(mStreamFormat, nFrames);
	}
	AudioBufferList& SetBufferList(AudioBufferList& abl) { return mIOBuffer.SetBufferList(abl); }
	z0 SetBuffer(UInt32 index, AudioBuffer& ab) { mIOBuffer.SetBuffer(index, ab); }
	z0 InvalidateBufferList() { mIOBuffer.InvalidateBufferList(); }
	[[nodiscard]] AudioBufferList& GetBufferList() const { return mIOBuffer.GetBufferList(); }

	[[nodiscard]] f32* GetFloat32ChannelData(UInt32 ch)
	{
		if (IsInterleaved()) {
			return static_cast<f32*>(mIOBuffer.GetBufferList().mBuffers[0].mData) + ch; // NOLINT
		}
		return static_cast<f32*>(mIOBuffer.GetBufferList().mBuffers[ch].mData); // NOLINT
	}

	z0 CopyBufferListTo(AudioBufferList& abl) const { mIOBuffer.CopyBufferListTo(abl); }
	z0 CopyBufferContentsTo(AudioBufferList& abl) const { mIOBuffer.CopyBufferContentsTo(abl); }
	[[nodiscard]] b8 IsInterleaved() const noexcept { return ASBD::IsInterleaved(mStreamFormat); }
	[[nodiscard]] UInt32 NumberChannels() const noexcept { return mStreamFormat.mChannelsPerFrame; }
	[[nodiscard]] UInt32 NumberInterleavedChannels() const noexcept
	{
		return ASBD::NumberInterleavedChannels(mStreamFormat);
	}
	virtual std::vector<AudioChannelLayoutTag> GetChannelLayoutTags();

	[[nodiscard]] const AUChannelLayout& ChannelLayout() const { return mChannelLayout; }

	// Old layout methods
	virtual OSStatus SetAudioChannelLayout(const AudioChannelLayout& inLayout);
	virtual UInt32 GetAudioChannelLayout(AudioChannelLayout* outLayoutPtr, b8& outWritable);

	virtual OSStatus RemoveAudioChannelLayout();

	/*! @fn AsIOElement*/
	AUIOElement* AsIOElement() override { return this; }

protected:
	AUBufferList& IOBuffer() noexcept { return mIOBuffer; }
	z0 ForceSetAudioChannelLayout(const AudioChannelLayout& inLayout)
	{
		mChannelLayout = inLayout;
	}

private:
	AudioStreamBasicDescription mStreamFormat{};
	AUChannelLayout mChannelLayout{};
	AUBufferList mIOBuffer; // for input: input proc buffer, only allocated when needed
							// for output: output cache, usually allocated early on
	b8 mWillAllocate{ false };
};

// ____________________________________________________________________________
//
/*!
	@class	AUScopeDelegate
	@brief	Provides a way to customize a scope, thereby obtaining virtual scopes.

	Can be used to implement scopes with variable numbers of elements.
*/
class AUScopeDelegate {
public:
	AUScopeDelegate() = default;

	virtual ~AUScopeDelegate() = default;

	AUScopeDelegate(const AUScopeDelegate&) = delete;
	AUScopeDelegate(AUScopeDelegate&&) = delete;
	AUScopeDelegate& operator=(const AUScopeDelegate&) = delete;
	AUScopeDelegate& operator=(AUScopeDelegate&&) = delete;

	z0 Initialize(AUBase* creator, AudioUnitScope scope, UInt32 numElements)
	{
		mCreator = creator;
		mScope = scope;
		SetNumberOfElements(numElements);
	}
	virtual z0 SetNumberOfElements(UInt32 numElements) = 0;
	virtual UInt32 GetNumberOfElements() = 0;
	virtual AUElement* GetElement(UInt32 elementIndex) = 0;

	[[nodiscard]] AUBase* GetCreator() const noexcept { return mCreator; }
	[[nodiscard]] AudioUnitScope GetScope() const noexcept { return mScope; }


private:
	AUBase* mCreator{ nullptr };
	AudioUnitScope mScope{ 0 };
};

// ____________________________________________________________________________
//
/*!
	@class	AUScope
	@brief	Organizes one or more elements into an addressable group (e.g. global, input, output).
*/
class AUScope {
public:
	AUScope() = default;

	~AUScope() = default;

	AUScope(const AUScope&) = delete;
	AUScope(AUScope&&) = delete;
	AUScope& operator=(const AUScope&) = delete;
	AUScope& operator=(AUScope&&) = delete;

	z0 Initialize(AUBase* creator, AudioUnitScope scope, UInt32 numElements)
	{
		mCreator = creator;
		mScope = scope;

		if (mDelegate != nullptr) {
			return mDelegate->Initialize(creator, scope, numElements);
		}

		SetNumberOfElements(numElements);
	}
	z0 SetNumberOfElements(UInt32 numElements);
	[[nodiscard]] UInt32 GetNumberOfElements() const
	{
		if (mDelegate != nullptr) {
			return mDelegate->GetNumberOfElements();
		}

		return static_cast<UInt32>(mElements.size());
	}
	[[nodiscard]] AUElement* GetElement(UInt32 elementIndex) const
	{
		if (mDelegate != nullptr) {
			return mDelegate->GetElement(elementIndex);
		}
		return elementIndex < mElements.size() ? mElements[elementIndex].get() : nullptr;
	}
	[[nodiscard]] AUElement* SafeGetElement(UInt32 elementIndex) const
	{
		AUElement* const element = GetElement(elementIndex);
		ausdk::ThrowExceptionIf(element == nullptr, kAudioUnitErr_InvalidElement);
		return element;
	}
	[[nodiscard]] AUIOElement* GetIOElement(UInt32 elementIndex) const
	{
		AUElement* const element = GetElement(elementIndex);
		AUIOElement* const ioel = element != nullptr ? element->AsIOElement() : nullptr;
		ausdk::ThrowExceptionIf(ioel == nullptr, kAudioUnitErr_InvalidElement);
		return ioel;
	}

	[[nodiscard]] b8 HasElementWithName() const;
	z0 AddElementNamesToDict(CFMutableDictionaryRef inNameDict) const;

	[[nodiscard]] std::vector<AudioUnitElement> RestoreElementNames(
		CFDictionaryRef inNameDict) const;

	[[nodiscard]] AudioUnitScope GetScope() const noexcept { return mScope; }

	z0 SetDelegate(AUScopeDelegate* inDelegate) noexcept { mDelegate = inDelegate; }
	z0 SaveState(CFMutableDataRef data) const;
	const UInt8* RestoreState(const UInt8* state) const;

private:
	using ElementVector = std::vector<std::unique_ptr<AUElement>>;

	AUBase* mCreator{ nullptr };
	AudioUnitScope mScope{ 0 };
	ElementVector mElements;
	AUScopeDelegate* mDelegate{ nullptr };
};

} // namespace ausdk

#endif // AudioUnitSDK_AUScopeElement_h
