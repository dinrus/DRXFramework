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

#if DRX_MAC
using Base = NSViewComponent;
#else
using Base = UIViewComponent;
#endif

struct VideoComponent::Pimpl   : public Base
{
    Pimpl (VideoComponent& ownerToUse, b8 useNativeControlsIfAvailable)
        : owner (ownerToUse),
          playerController (*this, useNativeControlsIfAvailable)
    {
        setVisible (true);

        auto* view = playerController.getView();
        setView (view);

       #if DRX_MAC
        [view setNextResponder: [view superview]];
        [view setWantsLayer: YES];
       #endif
    }

    ~Pimpl()
    {
        close();
        setView (nil);
    }

    Result load (const File& file)
    {
        auto r = load (createNSURLFromFile (file));

        if (r.wasOk())
            currentFile = file;

        return r;
    }

    Result load (const URL& url)
    {
        auto r = load ([NSURL URLWithString: juceStringToNS (url.toString (true))]);

        if (r.wasOk())
            currentURL = url;

        return r;
    }

    Result load (NSURL* url)
    {
        if (url != nil)
        {
            close();
            return playerController.load (url);
        }

        return Result::fail ("Couldn't open movie");
    }

    z0 loadAsync (const URL& url, std::function<z0 (const URL&, Result)> callback)
    {
        if (url.isEmpty())
        {
            jassertfalse;
            return;
        }

        currentURL = url;

        jassert (callback != nullptr);

        loadFinishedCallback = std::move (callback);

        playerController.loadAsync (url);
    }

    z0 close()
    {
        stop();
        playerController.close();
        currentFile = File();
        currentURL = {};
    }

    b8 isOpen() const noexcept        { return playerController.getPlayer() != nil; }
    b8 isPlaying() const noexcept     { return ! approximatelyEqual (getSpeed(), 0.0); }

    z0 play() noexcept                { [playerController.getPlayer() play]; setSpeed (playSpeedMult); }
    z0 stop() noexcept                { [playerController.getPlayer() pause]; }

    z0 setPosition (f64 newPosition)
    {
        if (auto* p = playerController.getPlayer())
        {
            CMTime t = { (CMTimeValue) (100000.0 * newPosition),
                         (CMTimeScale) 100000, kCMTimeFlags_Valid, {} };

            [p seekToTime: t
          toleranceBefore: kCMTimeZero
           toleranceAfter: kCMTimeZero];
        }
    }

    f64 getPosition() const
    {
        if (auto* p = playerController.getPlayer())
            return toSeconds ([p currentTime]);

        return 0.0;
    }

    z0 setSpeed (f64 newSpeed)
    {
        playSpeedMult = newSpeed;

        // Calling non 0.0 speed on a paused player would start it...
        if (isPlaying())
            [playerController.getPlayer() setRate: (f32) playSpeedMult];
    }

    f64 getSpeed() const
    {
        if (auto* p = playerController.getPlayer())
            return [p rate];

        return 0.0;
    }

    Rectangle<i32> getNativeSize() const
    {
        if (auto* p = playerController.getPlayer())
        {
            auto s = [[p currentItem] presentationSize];
            return { (i32) s.width, (i32) s.height };
        }

        return {};
    }

    f64 getDuration() const
    {
        if (auto* p = playerController.getPlayer())
            return toSeconds ([[p currentItem] duration]);

        return 0.0;
    }

    z0 setVolume (f32 newVolume)
    {
        [playerController.getPlayer() setVolume: newVolume];
    }

    f32 getVolume() const
    {
        if (auto* p = playerController.getPlayer())
            return [p volume];

        return 0.0f;
    }

    File currentFile;
    URL currentURL;

private:
    //==============================================================================
    template <typename Derived>
    class PlayerControllerBase
    {
    public:
        ~PlayerControllerBase()
        {
            // Derived classes must call detachPlayerStatusObserver() before destruction!
            jassert (! playerStatusObserverAttached);

            // Derived classes must call detachPlaybackObserver() before destruction!
            jassert (! playbackObserverAttached);

            // Note that it's unsafe to call detachPlayerStatusObserver and detachPlaybackObserver
            // directly here, because those functions call into the derived class, which will have
            // been destroyed at this point.
        }

    protected:
        //==============================================================================
        struct DrxPlayerStatusObserverClass : public ObjCClass<NSObject>
        {
            DrxPlayerStatusObserverClass()    : ObjCClass<NSObject> ("DrxPlayerStatusObserverClass_")
            {
                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
                addMethod (@selector (observeValueForKeyPath:ofObject:change:context:), valueChanged);
                DRX_END_IGNORE_WARNINGS_GCC_LIKE

                addIvar<PlayerAsyncInitialiser*> ("owner");

                registerClass();
            }

            //==============================================================================
            static PlayerControllerBase& getOwner (id self) { return *getIvar<PlayerControllerBase*> (self, "owner"); }
            static z0 setOwner (id self, PlayerControllerBase* p) { object_setInstanceVariable (self, "owner", p); }

        private:
            static z0 valueChanged (id self, SEL, NSString* keyPath, id,
                                      NSDictionary<NSString*, id>* change, uk)
            {
                auto& owner = getOwner (self);

                if ([keyPath isEqualToString: nsStringLiteral ("rate")])
                {
                    auto oldRate = [[change objectForKey: NSKeyValueChangeOldKey] floatValue];
                    auto newRate = [[change objectForKey: NSKeyValueChangeNewKey] floatValue];

                    if (approximatelyEqual (oldRate, 0.0f) && ! approximatelyEqual (newRate, 0.0f))
                        owner.playbackStarted();
                    else if (! approximatelyEqual (oldRate, 0.0f) && approximatelyEqual (newRate, 0.0f))
                        owner.playbackStopped();
                }
                else if ([keyPath isEqualToString: nsStringLiteral ("status")])
                {
                    auto status = [[change objectForKey: NSKeyValueChangeNewKey] intValue];

                    if (status == AVPlayerStatusFailed)
                        owner.errorOccurred();
                }
            }
        };

        //==============================================================================
        struct DrxPlayerItemPlaybackStatusObserverClass : public ObjCClass<NSObject>
        {
            DrxPlayerItemPlaybackStatusObserverClass()    : ObjCClass<NSObject> ("DrxPlayerItemPlaybackStatusObserverClass_")
            {
                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
                addMethod (@selector (processNotification:), notificationReceived);
                DRX_END_IGNORE_WARNINGS_GCC_LIKE

                addIvar<PlayerControllerBase*> ("owner");

                registerClass();
            }

            //==============================================================================
            static PlayerControllerBase& getOwner (id self) { return *getIvar<PlayerControllerBase*> (self, "owner"); }
            static z0 setOwner (id self, PlayerControllerBase* p) { object_setInstanceVariable (self, "owner", p); }

        private:
            static z0 notificationReceived (id self, SEL, NSNotification* notification)
            {
                if ([notification.name isEqualToString: AVPlayerItemDidPlayToEndTimeNotification])
                    getOwner (self).playbackReachedEndTime();
            }
        };

        //==============================================================================
        class PlayerAsyncInitialiser
        {
        public:
            PlayerAsyncInitialiser (PlayerControllerBase& ownerToUse)
                : owner (ownerToUse),
                  assetKeys ([[NSArray alloc] initWithObjects: nsStringLiteral ("duration"), nsStringLiteral ("tracks"),
                                                               nsStringLiteral ("playable"), nil])
            {
                static DrxPlayerItemPreparationStatusObserverClass cls;
                playerItemPreparationStatusObserver.reset ([cls.createInstance() init]);
                DrxPlayerItemPreparationStatusObserverClass::setOwner (playerItemPreparationStatusObserver.get(), this);
            }

            ~PlayerAsyncInitialiser()
            {
                detachPreparationStatusObserver();
            }

            z0 loadAsync (URL url)
            {
                auto nsUrl = [NSURL URLWithString: juceStringToNS (url.toString (true))];
                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
                asset.reset ([[AVURLAsset alloc] initWithURL: nsUrl options: nil]);
                DRX_END_IGNORE_WARNINGS_GCC_LIKE

                [asset.get() loadValuesAsynchronouslyForKeys: assetKeys.get()
                                           completionHandler: ^() { checkAllKeysReadyFor (asset.get(), url); }];
            }

        private:
            //==============================================================================
            struct DrxPlayerItemPreparationStatusObserverClass : public ObjCClass<NSObject>
            {
                DrxPlayerItemPreparationStatusObserverClass()    : ObjCClass<NSObject> ("DrxPlayerItemStatusObserverClass_")
                {
                    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
                    addMethod (@selector (observeValueForKeyPath:ofObject:change:context:), valueChanged);
                    DRX_END_IGNORE_WARNINGS_GCC_LIKE

                    addIvar<PlayerAsyncInitialiser*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static PlayerAsyncInitialiser& getOwner (id self) { return *getIvar<PlayerAsyncInitialiser*> (self, "owner"); }
                static z0 setOwner (id self, PlayerAsyncInitialiser* p) { object_setInstanceVariable (self, "owner", p); }

            private:
                static z0 valueChanged (id self, SEL, NSString*, id object,
                                          NSDictionary<NSString*, id>* change, uk context)
                {
                    auto& owner = getOwner (self);

                    if (context == &owner)
                    {
                        auto* playerItem = (AVPlayerItem*) object;
                        auto* urlAsset = (AVURLAsset*) playerItem.asset;

                        URL url (nsStringToDrx (urlAsset.URL.absoluteString));
                        auto oldStatus = [[change objectForKey: NSKeyValueChangeOldKey] intValue];
                        auto newStatus = [[change objectForKey: NSKeyValueChangeNewKey] intValue];

                        // Ignore spurious notifications
                        if (oldStatus == newStatus)
                            return;

                        if (newStatus == AVPlayerItemStatusFailed)
                        {
                            auto errorMessage = playerItem.error != nil
                                              ? nsStringToDrx (playerItem.error.localizedDescription)
                                              : Txt();

                            owner.notifyOwnerPreparationFinished (url, Result::fail (errorMessage), nullptr);
                        }
                        else if (newStatus == AVPlayerItemStatusReadyToPlay)
                        {
                            owner.notifyOwnerPreparationFinished (url, Result::ok(), owner.player.get());
                        }
                        else
                        {
                            jassertfalse;
                        }
                    }
                }
            };

            //==============================================================================
            PlayerControllerBase& owner;

            std::unique_ptr<AVURLAsset, NSObjectDeleter> asset;
            std::unique_ptr<NSArray<NSString*>, NSObjectDeleter> assetKeys;
            std::unique_ptr<AVPlayerItem, NSObjectDeleter> playerItem;
            std::unique_ptr<NSObject, NSObjectDeleter> playerItemPreparationStatusObserver;
            std::unique_ptr<AVPlayer, NSObjectDeleter> player;

            //==============================================================================
            z0 checkAllKeysReadyFor (AVAsset* assetToCheck, const URL& url)
            {
                NSError* error = nil;

                [[maybe_unused]] i32 successCount = 0;

                for (NSString* key : assetKeys.get())
                {
                    switch ([assetToCheck statusOfValueForKey: key error: &error])
                    {
                        case AVKeyValueStatusLoaded:
                        {
                            ++successCount;
                            break;
                        }
                        case AVKeyValueStatusCancelled:
                        {
                            notifyOwnerPreparationFinished (url, Result::fail ("Loading cancelled"), nullptr);
                            return;
                        }
                        case AVKeyValueStatusFailed:
                        {
                            auto errorMessage = error != nil ? nsStringToDrx (error.localizedDescription) : Txt();
                            notifyOwnerPreparationFinished (url, Result::fail (errorMessage), nullptr);
                            return;
                        }

                        case AVKeyValueStatusUnknown:
                        case AVKeyValueStatusLoading:
                        default:
                            break;
                    }
                }

                jassert (successCount == (i32) [assetKeys.get() count]);
                preparePlayerItem();
            }

            z0 preparePlayerItem()
            {
                playerItem.reset ([[AVPlayerItem alloc] initWithAsset: asset.get()]);

                attachPreparationStatusObserver();

                player.reset ([[AVPlayer alloc] initWithPlayerItem: playerItem.get()]);
            }

            //==============================================================================
            z0 attachPreparationStatusObserver()
            {
                [playerItem.get() addObserver: playerItemPreparationStatusObserver.get()
                                   forKeyPath: nsStringLiteral ("status")
                                      options: NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
                                      context: this];
            }

            z0 detachPreparationStatusObserver()
            {
                if (playerItem != nullptr && playerItemPreparationStatusObserver != nullptr)
                {
                    [playerItem.get() removeObserver: playerItemPreparationStatusObserver.get()
                                          forKeyPath: nsStringLiteral ("status")
                                             context: this];
                }
            }

            //==============================================================================
            z0 notifyOwnerPreparationFinished (const URL& url, Result r, AVPlayer* preparedPlayer)
            {
                MessageManager::callAsync ([url, preparedPlayer, r,
                                            safeThis = WeakReference<PlayerAsyncInitialiser> { this }]() mutable
                {
                    if (safeThis != nullptr)
                        safeThis->owner.playerPreparationFinished (url, r, preparedPlayer);
                });
            }

            DRX_DECLARE_WEAK_REFERENCEABLE (PlayerAsyncInitialiser)
        };

        //==============================================================================
        Pimpl& owner;
        b8 useNativeControls;

        PlayerAsyncInitialiser playerAsyncInitialiser;
        std::unique_ptr<NSObject, NSObjectDeleter> playerStatusObserver;
        std::unique_ptr<NSObject, NSObjectDeleter> playerItemPlaybackStatusObserver;

        //==============================================================================
        PlayerControllerBase (Pimpl& ownerToUse, b8 useNativeControlsIfAvailable)
            : owner (ownerToUse),
              useNativeControls (useNativeControlsIfAvailable),
              playerAsyncInitialiser (*this)
        {
            static DrxPlayerStatusObserverClass playerObserverClass;
            playerStatusObserver.reset ([playerObserverClass.createInstance() init]);
            DrxPlayerStatusObserverClass::setOwner (playerStatusObserver.get(), this);

            static DrxPlayerItemPlaybackStatusObserverClass itemObserverClass;
            playerItemPlaybackStatusObserver.reset ([itemObserverClass.createInstance() init]);
            DrxPlayerItemPlaybackStatusObserverClass::setOwner (playerItemPlaybackStatusObserver.get(), this);
        }

        //==============================================================================
        z0 attachPlayerStatusObserver()
        {
            [crtp().getPlayer() addObserver: playerStatusObserver.get()
                                 forKeyPath: nsStringLiteral ("rate")
                                    options: NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
                                    context: this];

            [crtp().getPlayer() addObserver: playerStatusObserver.get()
                                 forKeyPath: nsStringLiteral ("status")
                                    options: NSKeyValueObservingOptionNew
                                    context: this];

            playerStatusObserverAttached = true;
        }

        z0 detachPlayerStatusObserver()
        {
            if (crtp().getPlayer() != nullptr && playerStatusObserver != nullptr)
            {
                [crtp().getPlayer() removeObserver: playerStatusObserver.get()
                                        forKeyPath: nsStringLiteral ("rate")
                                           context: this];

                [crtp().getPlayer() removeObserver: playerStatusObserver.get()
                                        forKeyPath: nsStringLiteral ("status")
                                           context: this];
            }

            playerStatusObserverAttached = false;
        }

        z0 attachPlaybackObserver()
        {
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            [[NSNotificationCenter defaultCenter] addObserver: playerItemPlaybackStatusObserver.get()
                                                     selector: @selector (processNotification:)
                                                         name: AVPlayerItemDidPlayToEndTimeNotification
                                                       object: [crtp().getPlayer() currentItem]];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            playbackObserverAttached = true;
        }

        z0 detachPlaybackObserver()
        {
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            [[NSNotificationCenter defaultCenter] removeObserver: playerItemPlaybackStatusObserver.get()];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            playbackObserverAttached = false;
        }

    private:
        //==============================================================================
        Derived& crtp() { return static_cast<Derived&> (*this); }

        //==============================================================================
        z0 playerPreparationFinished (const URL& url, Result r, AVPlayer* preparedPlayer)
        {
            if (preparedPlayer != nil)
                crtp().setPlayer (preparedPlayer);

            owner.playerPreparationFinished (url, r);
        }

        z0 playbackReachedEndTime()
        {
            MessageManager::callAsync ([safeThis = WeakReference<PlayerControllerBase> { this }]() mutable
                                       {
                                           if (safeThis != nullptr)
                                               safeThis->owner.playbackReachedEndTime();
                                       });
        }

        //==============================================================================
        z0 errorOccurred()
        {
            auto errorMessage = (crtp().getPlayer() != nil && crtp().getPlayer().error != nil)
                              ? nsStringToDrx (crtp().getPlayer().error.localizedDescription)
                              : Txt();

            owner.errorOccurred (errorMessage);
        }

        z0 playbackStarted()
        {
            owner.playbackStarted();
        }

        z0 playbackStopped()
        {
            owner.playbackStopped();
        }

        b8 playerStatusObserverAttached = false, playbackObserverAttached = false;

        DRX_DECLARE_WEAK_REFERENCEABLE (PlayerControllerBase)
    };

   #if DRX_MAC
    //==============================================================================
    class PlayerController  : public PlayerControllerBase<PlayerController>
    {
    public:
        PlayerController (Pimpl& ownerToUse, b8 useNativeControlsIfAvailable)
            : PlayerControllerBase (ownerToUse, useNativeControlsIfAvailable)
        {
            wrappedPlayer = [&]() -> std::unique_ptr<WrappedPlayer>
            {
                if (useNativeControls)
                    return std::make_unique<WrappedPlayerView>();

                return std::make_unique<WrappedPlayerLayer>();
            }();
        }

        NSView* getView()
        {
            return wrappedPlayer->getView();
        }

        Result load (NSURL* url)
        {
            if (auto player = [AVPlayer playerWithURL: url])
            {
                setPlayer (player);
                return Result::ok();
            }

            return Result::fail ("Couldn't open movie");
        }

        z0 loadAsync (URL url)
        {
            playerAsyncInitialiser.loadAsync (url);
        }

        z0 close() { setPlayer (nil); }

        z0 setPlayer (AVPlayer* player)
        {
            detachPlayerStatusObserver();
            detachPlaybackObserver();

            wrappedPlayer->setPlayer (player);

            if (player != nil)
            {
                attachPlayerStatusObserver();
                attachPlaybackObserver();
            }
        }

        AVPlayer* getPlayer() const
        {
            return wrappedPlayer->getPlayer();
        }

    private:
        struct WrappedPlayer
        {
            virtual ~WrappedPlayer() = default;
            virtual NSView* getView() const = 0;
            virtual AVPlayer* getPlayer() const = 0;
            virtual z0 setPlayer (AVPlayer*) = 0;
        };

        class WrappedPlayerLayer : public WrappedPlayer
        {
        public:
            WrappedPlayerLayer()                        { [view.get() setLayer: playerLayer.get()]; }
            NSView* getView() const override            { return view.get(); }
            AVPlayer* getPlayer() const override        { return [playerLayer.get() player]; }
            z0 setPlayer (AVPlayer* player) override  { [playerLayer.get() setPlayer: player]; }

        private:
            NSUniquePtr<NSView> view                    { [[NSView alloc] init] };
            NSUniquePtr<AVPlayerLayer> playerLayer      { [[AVPlayerLayer alloc] init] };
        };

        class WrappedPlayerView : public WrappedPlayer
        {
        public:
            WrappedPlayerView() = default;
            NSView* getView() const override            { return playerView.get(); }
            AVPlayer* getPlayer() const override        { return [playerView.get() player]; }
            z0 setPlayer (AVPlayer* player) override  { [playerView.get() setPlayer: player]; }

        private:
            NSUniquePtr<AVPlayerView> playerView        { [[AVPlayerView alloc] init] };
        };

        std::unique_ptr<WrappedPlayer> wrappedPlayer;
    };
   #else
    //==============================================================================
    class PlayerController  : public PlayerControllerBase<PlayerController>
    {
    public:
        PlayerController (Pimpl& ownerToUse, b8 useNativeControlsIfAvailable)
            : PlayerControllerBase (ownerToUse, useNativeControlsIfAvailable)
        {
            if (useNativeControls)
            {
                playerViewController.reset ([[AVPlayerViewController alloc] init]);
            }
            else
            {
                static DrxVideoViewerClass cls;
                playerView.reset ([cls.createInstance() init]);

                playerLayer.reset ([[AVPlayerLayer alloc] init]);
                [playerView.get().layer addSublayer: playerLayer.get()];
            }
        }

        UIView* getView()
        {
            if (useNativeControls)
                return [playerViewController.get() view];

            // Should call getView() only once.
            jassert (playerView != nil);
            return playerView.release();
        }

        Result load (NSURL*)
        {
            jassertfalse;
            return Result::fail ("Synchronous loading не поддерживается on iOS, use loadAsync()");
        }

        z0 loadAsync (URL url)
        {
            playerAsyncInitialiser.loadAsync (url);
        }

        z0 close() { setPlayer (nil); }

        AVPlayer* getPlayer() const
        {
            if (useNativeControls)
                return [playerViewController.get() player];

            return [playerLayer.get() player];
        }

        z0 setPlayer (AVPlayer* playerToUse)
        {
            detachPlayerStatusObserver();
            detachPlaybackObserver();

            if (useNativeControls)
                [playerViewController.get() setPlayer: playerToUse];
            else
                [playerLayer.get() setPlayer: playerToUse];

            if (playerToUse != nil)
            {
                attachPlayerStatusObserver();
                attachPlaybackObserver();
            }
        }

    private:
        //==============================================================================
        struct DrxVideoViewerClass    : public ObjCClass<UIView>
        {
            DrxVideoViewerClass()  : ObjCClass<UIView> ("DrxVideoViewerClass_")
            {
                addMethod (@selector (layoutSubviews), layoutSubviews);

                registerClass();
            }

        private:
            static z0 layoutSubviews (id self, SEL)
            {
                sendSuperclassMessage<z0> (self, @selector (layoutSubviews));

                UIView* asUIView = (UIView*) self;

                if (auto* previewLayer = getPreviewLayer (self))
                    previewLayer.frame = asUIView.bounds;
            }

            static AVPlayerLayer* getPreviewLayer (id self)
            {
                UIView* asUIView = (UIView*) self;

                if (asUIView.layer.sublayers != nil && [asUIView.layer.sublayers count] > 0)
                    if ([asUIView.layer.sublayers[0] isKindOfClass: [AVPlayerLayer class]])
                        return (AVPlayerLayer*) asUIView.layer.sublayers[0];

                return nil;
            }
        };

        //==============================================================================
        std::unique_ptr<AVPlayerViewController, NSObjectDeleter> playerViewController;

        std::unique_ptr<UIView, NSObjectDeleter> playerView;
        std::unique_ptr<AVPlayerLayer, NSObjectDeleter> playerLayer;
    };
   #endif

    //==============================================================================
    VideoComponent& owner;

    PlayerController playerController;

    std::function<z0 (const URL&, Result)> loadFinishedCallback;

    f64 playSpeedMult = 1.0;

    static f64 toSeconds (const CMTime& t) noexcept
    {
        return t.timescale != 0 ? ((f64) t.value / (f64) t.timescale) : 0.0;
    }

    z0 playerPreparationFinished (const URL& url, Result r)
    {
        owner.resized();

        loadFinishedCallback (url, r);
        loadFinishedCallback = nullptr;
    }

    z0 errorOccurred (const Txt& errorMessage)
    {
        NullCheckedInvocation::invoke (owner.onErrorOccurred, errorMessage);
    }

    z0 playbackStarted()
    {
        NullCheckedInvocation::invoke (owner.onPlaybackStarted);
    }

    z0 playbackStopped()
    {
        NullCheckedInvocation::invoke (owner.onPlaybackStopped);
    }

    z0 playbackReachedEndTime()
    {
        stop();
        setPosition (0.0);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};
