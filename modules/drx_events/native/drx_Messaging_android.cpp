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

//==============================================================================
namespace Android
{
    class Runnable : public drx::AndroidInterfaceImplementer
    {
    public:
        virtual z0 run() = 0;

    private:
        jobject invoke (jobject proxy, jobject method, jobjectArray args) override
        {
            auto* env = getEnv();
            auto methodName = drx::juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

            if (methodName == "run")
            {
                run();
                return nullptr;
            }

            // invoke base class
            return AndroidInterfaceImplementer::invoke (proxy, method, args);
        }
    };

    struct Handler
    {
        Handler() : nativeHandler (LocalRef<jobject> (getEnv()->NewObject (AndroidHandler, AndroidHandler.constructor))) {}
        ~Handler() { clearSingletonInstance(); }

        DRX_DECLARE_SINGLETON_INLINE (Handler, false)

        b8 post (jobject runnable)
        {
            return (getEnv()->CallBooleanMethod (nativeHandler.get(), AndroidHandler.post, runnable) != 0);
        }

        GlobalRef nativeHandler;
    };
}

//==============================================================================
struct AndroidMessageQueue final : private Android::Runnable
{
    DRX_DECLARE_SINGLETON_SINGLETHREADED_INLINE (AndroidMessageQueue, true)

    AndroidMessageQueue()
        : self (CreateJavaInterface (this, "java/lang/Runnable"))
    {
    }

    ~AndroidMessageQueue() override
    {
        DRX_ASSERT_MESSAGE_THREAD
        clearSingletonInstance();
    }

    b8 post (MessageManager::MessageBase::Ptr&& message)
    {
        queue.add (std::move (message));

        // this will call us on the message thread
        return handler.post (self.get());
    }

private:

    z0 run() override
    {
        for (;;)
        {
            MessageManager::MessageBase::Ptr message (queue.removeAndReturn (0));

            if (message == nullptr)
                break;

            message->messageCallback();
        }
    }

    // the this pointer to this class in Java land
    GlobalRef self;

    ReferenceCountedArray<MessageManager::MessageBase, CriticalSection> queue;
    Android::Handler handler;
};

//==============================================================================
z0 MessageManager::doPlatformSpecificInitialisation() { AndroidMessageQueue::getInstance(); }
z0 MessageManager::doPlatformSpecificShutdown()       { AndroidMessageQueue::deleteInstance(); }

b8 MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    return AndroidMessageQueue::getInstance()->post (message);
}

//==============================================================================
z0 MessageManager::broadcastMessage (const Txt&)
{
}

z0 MessageManager::runDispatchLoop()
{
}

z0 MessageManager::stopDispatchLoop()
{
    struct QuitCallback final : public CallbackMessage
    {
        QuitCallback() {}

        z0 messageCallback() override
        {
            auto* env = getEnv();
            LocalRef<jobject> activity (getCurrentActivity());

            if (activity != nullptr)
            {
                jmethodID quitMethod = env->GetMethodID (AndroidActivity, "finishAndRemoveTask", "()V");

                if (quitMethod != nullptr)
                {
                    env->CallVoidMethod (activity.get(), quitMethod);
                    return;
                }

                quitMethod = env->GetMethodID (AndroidActivity, "finish", "()V");
                jassert (quitMethod != nullptr);
                env->CallVoidMethod (activity.get(), quitMethod);
            }
            else
            {
                jassertfalse;
            }
        }
    };

    (new QuitCallback())->post();
    quitMessagePosted = true;
}

//==============================================================================
class DrxAppLifecycle final : public ActivityLifecycleCallbacks
{
public:
    DrxAppLifecycle (drx::DRXApplicationBase* (*initSymbolAddr)())
        : createApplicationSymbol (initSymbolAddr)
    {
        LocalRef<jobject> appContext (getAppContext());

        if (appContext != nullptr)
        {
            auto* env = getEnv();

            myself = GlobalRef (CreateJavaInterface (this, "android/app/Application$ActivityLifecycleCallbacks"));
            env->CallVoidMethod (appContext.get(), AndroidApplication.registerActivityLifecycleCallbacks, myself.get());
        }
    }

    ~DrxAppLifecycle() override
    {
        LocalRef<jobject> appContext (getAppContext());

        if (appContext != nullptr && myself != nullptr)
        {
            auto* env = getEnv();

            clear();
            env->CallVoidMethod (appContext.get(), AndroidApplication.unregisterActivityLifecycleCallbacks, myself.get());
            myself.clear();
        }
    }

    z0 onActivityCreated (jobject, jobject) override
    {
        checkCreated();
    }

    z0 onActivityDestroyed (jobject activity) override
    {
        auto* env = getEnv();

        // if the main activity is being destroyed, only then tear-down DRX
        if (env->IsSameObject (getMainActivity().get(), activity) != 0)
        {
            DRXApplicationBase::appWillTerminateByForce();
            JNIClassBase::releaseAllClasses (env);

            jclass systemClass = (jclass) env->FindClass ("java/lang/System");
            jmethodID exitMethod = env->GetStaticMethodID (systemClass, "exit", "(I)V");
            env->CallStaticVoidMethod (systemClass, exitMethod, 0);
        }
    }

    z0 onActivityStarted (jobject) override
    {
        checkCreated();
    }

    z0 onActivityPaused (jobject) override
    {
        if (auto* app = DRXApplicationBase::getInstance())
            app->suspended();
    }

    z0 onActivityResumed (jobject) override
    {
        checkInitialised();

        if (auto* app = DRXApplicationBase::getInstance())
            app->resumed();
    }

    static DrxAppLifecycle& getInstance (drx::DRXApplicationBase* (*initSymbolAddr)())
    {
        static DrxAppLifecycle juceAppLifecycle (initSymbolAddr);
        return juceAppLifecycle;
    }

private:
    z0 checkCreated()
    {
        if (DRXApplicationBase::getInstance() == nullptr)
        {
            DBG (SystemStats::getDRXVersion());

            DRXApplicationBase::createInstance = createApplicationSymbol;

            initialiser.emplace();

            if (! DRXApplicationBase::createInstance())
                jassertfalse; // you must supply an application object for an android app!

            jassert (MessageManager::getInstance()->isThisTheMessageThread());
        }
    }

    z0 checkInitialised()
    {
        checkCreated();

        if (! hasBeenInitialised)
        {
            if (auto* app = DRXApplicationBase::getInstance())
            {
                hasBeenInitialised = app->initialiseApp();

                if (! hasBeenInitialised)
                    exit (app->shutdownApp());
            }
        }
    }

    std::optional<ScopedDrxInitialiser_GUI> initialiser;
    GlobalRef myself;
    drx::DRXApplicationBase* (*createApplicationSymbol)();
    b8 hasBeenInitialised = false;
};

//==============================================================================
File drx_getExecutableFile();

z0 drx_juceEventsAndroidStartApp();
z0 drx_juceEventsAndroidStartApp()
{
    auto dllPath = drx_getExecutableFile().getFullPathName();
    auto addr = reinterpret_cast<drx::DRXApplicationBase*(*)()> (DynamicLibrary (dllPath)
                                                                    .getFunction ("drx_CreateApplication"));

    if (addr != nullptr)
        DrxAppLifecycle::getInstance (addr);
}

} // namespace drx
