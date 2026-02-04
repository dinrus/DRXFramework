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
    Содержит приватные методы для преобразования между форматами двоичной и
    стандартной кодировок base-64.

    @tags{Core}
*/
struct DRX_API Base64
{
    /** Преобразует двоичный блок данных в строку base-64.
        Данные итоговой строки записываются в заданный поток.
        Если в потоке происходит ошибка записи, метод терминирует и возвращает false.
    */
    static b8 convertToBase64 (OutputStream& base64Result, ukk sourceData, size_t sourceDataSize);

    /** Преобразует строку base-64 назад в её двоичное представление.
        Декодированные двоичные данные записываются в заданный поток.
        Если строке неполноценна как base-64, метод терминирует и возвращает false.
    */
    static b8 convertFromBase64 (OutputStream& binaryOutput, StringRef base64TextInput);

    /** Преобразует блок двоичных данных  в строку base-64. */
    static Txt toBase64 (ukk sourceData, size_t sourceDataSize);

    /** Преобразует представление строки в UTF-8 в строку base-64. */
    static Txt toBase64 (const Txt& textToEncode);
};

} // namespace drx
