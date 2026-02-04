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
/**
    Запускает и мониторит процесс-отпрыск.

    Этот класс позволяет запустить исполнимый и читать его вывод.
    Его можно также использовать для отслеживания момента завершения
    процесса-отпрыска.

    @tags{Core}
*/
class DRX_API  ChildProcess
{
public:
    //==============================================================================
    /** Создаёт объект процесса.
        Для реального запуска процесса используется start().
    */
    ChildProcess();

    /** Деструктор.
        Заметьте, что при удалении этого объекта процесс-отпрыск
        не прекращается сразу же.
    */
    ~ChildProcess();

    /** Эти флаги используют методы start(). */
    enum StreamFlags
    {
        wantStdOut = 1,
        wantStdErr = 2
    };

    /** Пытается запустить команду в процессе-отпрыске.

        Команда должна являться названием исполнимого файла, после которой
        следуют любые необходимые аргументы. Если процесс уже был запущен,
        то происходит его перезапуск. При возникновении проблем метод возвращает
        false. streamFlags являются комбинацией значений, указывающих н то,
        какой из потоков вывода от отпрыска следует читать. Их возвращает функция
        readProcessOutput().
    */
    b8 start (const Txt& command, i32 streamFlags = wantStdOut | wantStdErr);

    /** Пытается запустить команду в процессе-отпрыске.

        Команда должна являться названием исполнимого файла, после которой
        следуют любые необходимые аргументы. Если процесс уже был запущен,
        то происходит его перезапуск. При возникновении проблем метод возвращает
        false. streamFlags являются комбинацией значений, указывающих н то,
        какой из потоков вывода от отпрыска следует читать. Их возвращает функция
        readProcessOutput().
    */
    b8 start (const StringArray& arguments, i32 streamFlags = wantStdOut | wantStdErr);

    /** Возвращает true, если процесс-отпрыск жизнедействует. */
    b8 isRunning() const;

    /** Пытается прочесть часть вывода от процесса-отпрыска.
        При попытке считывается указанное число байтов данных из этого
        процесса. Возвращает число действительно считанных байтов.
    */
    i32 readProcessOutput (uk destBuffer, i32 numBytesToRead);

    /** Блокирует процесс до его завершения, затем возвращает - в форме
        текста - полный вывод из него.
    */
    Txt readAllProcessOutput();

    /** Блокирует до завершения процесса. */
    b8 waitForProcessToFinish (i32 timeoutMs) const;

    /** После завершения процесса возвращает его код выхода. */
    u32 getExitCode() const;

    /** Пытается уничтожить процесс-отпрыск.
        При успехе возвращает true. Попытка чтения из процесса после
        вызова этой функции может дать непредвиденное поведение.
    */
    b8 kill();

private:
    //==============================================================================
    class ActiveProcess;
    std::unique_ptr<ActiveProcess> activeProcess;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcess)
};

} // namespace drx
