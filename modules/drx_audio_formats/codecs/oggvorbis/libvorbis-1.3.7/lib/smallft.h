/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

 function: fft transform

 ********************************************************************/

#ifndef _V_SMFT_H_
#define _V_SMFT_H_

#include "../../codec.h"

typedef struct {
  i32 n;
  f32 *trigcache;
  i32 *splitcache;
} drft_lookup;

extern z0 drft_forward(drft_lookup *l,f32 *data);
extern z0 drft_backward(drft_lookup *l,f32 *data);
extern z0 drft_init(drft_lookup *l,i32 n);
extern z0 drft_clear(drft_lookup *l);

#endif
