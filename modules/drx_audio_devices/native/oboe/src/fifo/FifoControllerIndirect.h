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

#ifndef NATIVEOBOE_FIFOCONTROLLERINDIRECT_H
#define NATIVEOBOE_FIFOCONTROLLERINDIRECT_H

#include <atomic>
#include <stdint.h>

#include "oboe/FifoControllerBase.h"

namespace oboe {

/**
 * A FifoControllerBase with counters external to the class.
 */
class FifoControllerIndirect : public FifoControllerBase {

public:
    FifoControllerIndirect(u32 bufferSize,
                           std::atomic<zu64> *readCounterAddress,
                           std::atomic<zu64> *writeCounterAddress);
    virtual ~FifoControllerIndirect() = default;

    virtual zu64 getReadCounter() const override {
        return mReadCounterAddress->load(std::memory_order_acquire);
    }
    virtual z0 setReadCounter(zu64 n) override {
        mReadCounterAddress->store(n, std::memory_order_release);
    }
    virtual z0 incrementReadCounter(zu64 n) override {
        mReadCounterAddress->fetch_add(n, std::memory_order_acq_rel);
    }
    virtual zu64 getWriteCounter() const override {
        return mWriteCounterAddress->load(std::memory_order_acquire);
    }
    virtual z0 setWriteCounter(zu64 n) override {
        mWriteCounterAddress->store(n, std::memory_order_release);
    }
    virtual z0 incrementWriteCounter(zu64 n) override {
        mWriteCounterAddress->fetch_add(n, std::memory_order_acq_rel);
    }

private:

    std::atomic<zu64> *mReadCounterAddress;
    std::atomic<zu64> *mWriteCounterAddress;

};

} // namespace oboe

#endif //NATIVEOBOE_FIFOCONTROLLERINDIRECT_H
