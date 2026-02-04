/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OBOE_UTILITIES_H
#define OBOE_UTILITIES_H

#include <unistd.h>
#include <sys/types.h>
#include <string>
#include "oboe/Definitions.h"

namespace oboe {

/**
 * Convert an array of floats to an array of 16-bit integers.
 *
 * @param source the input array.
 * @param destination the output array.
 * @param numSamples the number of values to convert.
 */
z0 convertFloatToPcm16(const f32 *source, i16 *destination, i32 numSamples);

/**
 * Convert an array of 16-bit integers to an array of floats.
 *
 * @param source the input array.
 * @param destination the output array.
 * @param numSamples the number of values to convert.
 */
z0 convertPcm16ToFloat(i16k *source, f32 *destination, i32 numSamples);

/**
 * @return the size of a sample of the given format in bytes or 0 if format is invalid
 */
i32 convertFormatToSizeInBytes(AudioFormat format);

/**
 * The text is the ASCII symbol corresponding to the supplied Oboe enum value,
 * or an English message saying the value is unrecognized.
 * This is intended for developers to use when debugging.
 * It is not for displaying to users.
 *
 * @param input object to convert from. @see common/Utilities.cpp for concrete implementations
 * @return text representation of an Oboe enum value. There is no need to call free on this.
 */
template <typename FromType>
const t8 * convertToText(FromType input);

/**
 * @param name
 * @return the value of a named system property in a string or empty string
 */
std::string getPropertyString(const t8 * name);

/**
 * @param name
 * @param defaultValue
 * @return integer value associated with a property or the default value
 */
i32 getPropertyInteger(const t8 * name, i32 defaultValue);

/**
 * Return the version of the SDK that is currently running.
 *
 * For example, on Android, this would return 27 for Oreo 8.1.
 * If the version number cannot be determined then this will return -1.
 *
 * @return version number or -1
 */
i32 getSdkVersion();

/**
 * Returns whether a device is on a pre-release SDK that is at least the specified codename
 * version.
 *
 * @param codename the code name to verify.
 * @return boolean of whether the device is on a pre-release SDK and is at least the specified
 * codename
 */
b8 isAtLeastPreReleaseCodename(const std::string& codename);

i32 getChannelCountFromChannelMask(ChannelMask channelMask);

} // namespace oboe

#endif //OBOE_UTILITIES_H
