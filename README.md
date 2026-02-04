![alt text](https://assets.juce.com/juce/JUCE_banner_github.png "JUCE")

## Прототипирующий проект

JUCE - это опенсорсный кроссплатформный фреймворк для создания приложений на C++ для десктопов и мобильников, включающий аудиоплагины и хосты плагинов VST, VST3, AU, AUv3, AAX и LV2. JUCE может легко интегрироваться с существующими проетами через CMake, либо может использоваться как инстрмент генерации проектов с помощью
[Projucer](#projucer), который поддерживает экспортирование проектов для Xcode (macOS и iOS), Visual Studio, Android Studio и Linux Makefiles, также включая в себя редактор кода.

## Приступая

В репозитории JUCE содержатся ветви
[master](https://github.com/juce-framework/JUCE/tree/master) и
[develop](https://github.com/juce-framework/JUCE/tree/develop). Ветвь develop содержит последние исправления багов и фич, и время от времени маржируется в ветвь master в стабильных [выпусках с тэгами](https://github.com/juce-framework/JUCE/releases) (последний выпуск с предварительно построенными бмнарниками также можно загрузить с [веб-сайта JUCE](https://juce.com/get-juce)).

Проекты JUCE можно маржировать либо посредством Projucer (собственный инструмент конфигурирования JUCE), либо посредством CMake.

### Projucer

Репозитория не содержит уже построенный Projucer, его нужно будет построить самому для своей платформы - проекты Xcode, Visual Studio и Linux Makefile располагаются в [extras/Projucer/Builds](/extras/Projucer/Builds) (минимальные системные требования перечисленны в разделе [минимальные системные требования](#минимальные-системные-требования), расположенном ниже). Затем можно будет использовать Projucer для создания новых проектов JUCE, просмотра уроков и выполнения примеров. Можно также напрямую включать исходный код модулей JUCE в существующий проект, либо строить их в форме статических или динамических библиотек, которые можно линковать к проекту.

Дополнительная информация имеется на сайтах
[документации](https://juce.com/learn/documentation) и
[уроков](https://juce.com/learn/tutorials).

## Фреймворк DRX

Фреймворк DRX - это переработанная версия фреймворка JUCE, которая является ассимиляцией его в систему построений ИСР РНЦП "Динрус", с заменой типизации и с руссификацией справочной документации. Возможны иные существенные различия.

## Минимальные Системные Требования

#### Построение Проектов DRX

- __C++ Standard__: 17
- __macOS/iOS__: Xcode 12.4 (Intel macOS 10.15.4, Apple Silicon macOS 11.0)
- __Windows__: Visual Studio 2019 (Windows 10)
- __Linux__: g++ 7.0 or Clang 6.0 (for a full list of dependencies, see
[here](/docs/Linux%20Dependencies.md)).
- __Android__: Android Studio (NDK 26) on Windows, macOS or Linux

#### Цели Разворачивания

- __macOS__: macOS 10.11 (x86_64, Arm64)
- __Windows__: Windows 10 (x86_64, x86, Arm64, Arm64EC)
- __Linux__: Mainstream Linux distributions (x86_64, Arm64/aarch64, (32 bit Arm systems like armv7 should work but are not regularly tested))
- __iOS__: iOS 12 (Arm64, Arm64e, x86_64 (Simulator))
- __Android__: Android 7 - Nougat (API Level 24) (arm64-v8a, armeabi-v7a, x86_64, x86)

## Contributing

Please see our [contribution guidelines](.github/contributing.md).

## Licensing

See [LICENSE.md](LICENSE.md) for licensing and dependency information.

## AAX Plug-Ins

AAX plug-ins need to be digitally signed using PACE Anti-Piracy's signing tools
before they will run in commercially available versions of Pro Tools. These
tools are provided free of charge by Avid. Before obtaining the signing tools,
you will need to use a special build of Pro Tools, called Pro Tools Developer,
to test your unsigned plug-ins. The steps to obtain Pro Tools Developer are:

1. Sign up as an AAX Developer [here](https://developer.avid.com/aax/).
2. Request a Pro Tools Developer Bundle activation code by sending an email to
   [devauth@avid.com](mailto:devauth@avid.com).
3. Download the latest Pro Tools Developer build from your Avid Developer
   account.

When your plug-ins have been tested and debugged in Pro Tools Developer, and you
are ready to digitally sign them, please send an email to
[audiosdk@avid.com](mailto:audiosdk@avid.com) with the subject "PACE Eden
Signing Tools Request". You need to include an overview of each plug-in along
with a screen recording showing the plug-in running in Pro Tools Developer, with
audio if possible.

Please also include the following information:

- Company name
- Admin full name
- Telephone number

Once the request is submitted, PACE Anti-Piracy will contact you directly with
information about signing your plug-ins. When the plug-ins have been signed, you
are free to sell and distribute them. If you are interested in selling your
plug-ins on the Avid Marketplace, please send an email to
[audiosdk@avid.com](mailto:audiosdk@avid.com).
