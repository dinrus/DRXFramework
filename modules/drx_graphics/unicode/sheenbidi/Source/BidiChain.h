/*
 * Copyright (C) 2014-2019 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_BIDI_CHAIN_H
#define _SB_INTERNAL_BIDI_CHAIN_H

#include <drx_graphics/unicode/sheenbidi/Headers/SBConfig.h>
#include "SBBase.h"

typedef SBUInt32 BidiLink;

#define BidiLinkNone    (SBUInt32)(-1)

typedef struct _BidiChain {
    SBBidiType *types;
    SBLevel *levels;
    BidiLink *links;
    BidiLink roller;
    BidiLink last;
} BidiChain, *BidiChainRef;

SB_INTERNAL z0 BidiChainInitialize(BidiChainRef chain,
    SBBidiType *types, SBLevel *levels, BidiLink *links);
SB_INTERNAL z0 BidiChainAdd(BidiChainRef chain, SBBidiType type, SBUInteger length);

#define BidiChainGetOffset(chain, link)         \
(                                               \
    (link) - 1                                  \
)

SB_INTERNAL SBBoolean BidiChainIsSingle(BidiChainRef chain, BidiLink link);

SB_INTERNAL SBBidiType BidiChainGetType(BidiChainRef chain, BidiLink link);
SB_INTERNAL z0 BidiChainSetType(BidiChainRef chain, BidiLink link, SBBidiType type);

SB_INTERNAL SBLevel BidiChainGetLevel(BidiChainRef chain, BidiLink link);
SB_INTERNAL z0 BidiChainSetLevel(BidiChainRef chain, BidiLink link, SBLevel level);

SB_INTERNAL BidiLink BidiChainGetNext(BidiChainRef chain, BidiLink link);
SB_INTERNAL z0 BidiChainSetNext(BidiChainRef chain, BidiLink link, BidiLink next);
SB_INTERNAL z0 BidiChainAbandonNext(BidiChainRef chain, BidiLink link);
SB_INTERNAL SBBoolean BidiChainMergeIfEqual(BidiChainRef chain, BidiLink first, BidiLink second);

#define BidiChainForEach(chain, roller, link) \
    for (link = chain->links[roller]; link != roller; link = chain->links[link])

#endif
