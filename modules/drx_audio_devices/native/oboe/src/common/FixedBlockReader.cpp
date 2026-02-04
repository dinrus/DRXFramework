/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <stdint.h>
#include <memory.h>

#include "FixedBlockAdapter.h"

#include "FixedBlockReader.h"


FixedBlockReader::FixedBlockReader(FixedBlockProcessor &fixedBlockProcessor)
        : FixedBlockAdapter(fixedBlockProcessor) {
    mPosition = mSize;
}

i32 FixedBlockReader::open(i32 bytesPerFixedBlock) {
    i32 result = FixedBlockAdapter::open(bytesPerFixedBlock);
    mPosition = 0;
    mValid = 0;
    return result;
}

i32 FixedBlockReader::readFromStorage(u8 *buffer, i32 numBytes) {
    i32 bytesToRead = numBytes;
    i32 dataAvailable = mValid - mPosition;
    if (bytesToRead > dataAvailable) {
        bytesToRead = dataAvailable;
    }
    memcpy(buffer, mStorage.get() + mPosition, bytesToRead);
    mPosition += bytesToRead;
    return bytesToRead;
}

i32 FixedBlockReader::read(u8 *buffer, i32 numBytes) {
    i32 bytesRead;
    i32 bytesLeft = numBytes;
    while(bytesLeft > 0) {
        if (mPosition < mValid) {
            // Use up bytes currently in storage.
            bytesRead = readFromStorage(buffer, bytesLeft);
            buffer += bytesRead;
            bytesLeft -= bytesRead;
        } else if (bytesLeft >= mSize) {
            // Nothing in storage. Read through if enough for a complete block.
            bytesRead = mFixedBlockProcessor.onProcessFixedBlock(buffer, mSize);
            if (bytesRead < 0) return bytesRead;
            buffer += bytesRead;
            bytesLeft -= bytesRead;
        } else {
            // Just need a partial block so we have to reload storage.
            bytesRead = mFixedBlockProcessor.onProcessFixedBlock(mStorage.get(), mSize);
            if (bytesRead < 0) return bytesRead;
            mPosition = 0;
            mValid = bytesRead;
            if (bytesRead == 0) break;
        }
    }
    return numBytes - bytesLeft;
}
