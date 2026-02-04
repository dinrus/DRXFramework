#include "MainComponent.h"
//#include <drx/Core/Core.h>
//==============================================================================
class GuiAppApplication final : public drx::DRXApplication
{
public:
    //==============================================================================
    GuiAppApplication() {
        //drx::SetLanguage(LNG_RUSSIAN);
	    //Установка дефолтного набсима.
	    //drx::SetDefaultCharset(CHARSET_UTF8);
    
    }

    const drx::Txt getApplicationName() override       { return DRX_APPLICATION_NAME_STRING; }
    const drx::Txt getApplicationVersion() override    { return DRX_APPLICATION_VERSION_STRING; }
    b8 moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    z0 initialise (const drx::Txt& commandLine) override
    {
        // В этом методе нужно помещать код инициализации приложения.
        drx::ignoreUnused (commandLine);

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    z0 shutdown() override
    {
        // Здесь добавляется код сворачивания приложения.

        mainWindow = nullptr; // (удалить наше окно)
    }

    //==============================================================================
    z0 systemRequestedQuit() override
    {
        // Вызывается при получение приложением запроса на завершение:
        // можно проигнорировать этот запрос и продолжить выполнение,
        // либо вызвать quit(), чтобы закрыть приложение.
        quit();
    }

    z0 anotherInstanceStarted (const drx::Txt& commandLine) override
    {
        // При запуске другого экземпляра приложения, когда этот ещё выполняется,
        // вызывается этот метод, а параметр commandLine сообщает об
        // аргументах командной строки, переданных другому экземпляру.
        drx::ignoreUnused (commandLine);
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow final : public drx::DocumentWindow
    {
    public:
        explicit MainWindow (drx::Txt name)
            : DocumentWindow (name,
                              drx::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColor (backgroundColorId),
                              allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if DRX_IOS || DRX_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        z0 closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_DRX_APPLICATION (GuiAppApplication)
