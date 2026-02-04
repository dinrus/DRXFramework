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

 function: bitrate tracking and management

 ********************************************************************/

#ifndef _V_BITRATE_H_
#define _V_BITRATE_H_

#include "../../codec.h"
#include "codec_internal.h"
#include "os.h"

/* encode side bitrate tracking */
typedef struct bitrate_manager_state {
  i32            managed;

  i64           avg_reservoir;
  i64           minmax_reservoir;
  i64           avg_bitsper;
  i64           min_bitsper;
  i64           max_bitsper;

  i64           short_per_long;
  f64         avgfloat;

  vorbis_block  *vb;
  i32            choice;
} bitrate_manager_state;

typedef struct bitrate_manager_info{
  i64           avg_rate;
  i64           min_rate;
  i64           max_rate;
  i64           reservoir_bits;
  f64         reservoir_bias;

  f64         slew_damp;

} bitrate_manager_info;

extern z0 vorbis_bitrate_init(vorbis_info *vi,bitrate_manager_state *bs);
extern z0 vorbis_bitrate_clear(bitrate_manager_state *bs);
extern i32 vorbis_bitrate_managed(vorbis_block *vb);
extern i32 vorbis_bitrate_addblock(vorbis_block *vb);
extern i32 vorbis_bitrate_flushpacket(vorbis_dsp_state *vd, ogg_packet *op);

#endif
