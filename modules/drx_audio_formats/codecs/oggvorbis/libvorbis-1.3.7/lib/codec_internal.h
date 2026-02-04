/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

 function: libvorbis codec headers

 ********************************************************************/

#ifndef _V_CODECI_H_
#define _V_CODECI_H_

#include "envelope.h"
#include "codebook.h"

#define BLOCKTYPE_IMPULSE    0
#define BLOCKTYPE_PADDING    1
#define BLOCKTYPE_TRANSITION 0
#define BLOCKTYPE_LONG       1

#define PACKETBLOBS 15

typedef struct vorbis_block_internal{
  f32  **pcmdelay;  /* this is a pointer into local storage */
  f32  ampmax;
  i32    blocktype;

  oggpack_buffer *packetblob[PACKETBLOBS]; /* initialized, must be freed;
                                              blob [PACKETBLOBS/2] points to
                                              the oggpack_buffer in the
                                              main vorbis_block */
} vorbis_block_internal;

typedef z0 vorbis_look_floor;
typedef z0 vorbis_look_residue;
typedef z0 vorbis_look_transform;

/* mode ************************************************************/
typedef struct {
  i32 blockflag;
  i32 windowtype;
  i32 transformtype;
  i32 mapping;
} vorbis_info_mode;

typedef z0 vorbis_info_floor;
typedef z0 vorbis_info_residue;
typedef z0 vorbis_info_mapping;

#include "psy.h"
#include "bitrate.h"

typedef struct private_state {
  /* local lookup storage */
  envelope_lookup        *ve; /* envelope lookup */
  i32                     window[2];
  vorbis_look_transform **transform[2];    /* block, type */
  drft_lookup             fft_look[2];

  i32                     modebits;
  vorbis_look_floor     **flr;
  vorbis_look_residue   **residue;
  vorbis_look_psy        *psy;
  vorbis_look_psy_global *psy_g_look;

  /* local storage, only used on the encoding side.  This way the
     application does not need to worry about freeing some packets'
     memory and not others'; packet storage is always tracked.
     Cleared next call to a _dsp_ function */
  u8 *header;
  u8 *header1;
  u8 *header2;

  bitrate_manager_state bms;

  ogg_int64_t sample_count;
} private_state;

/* codec_setup_info contains all the setup information specific to the
   specific compression/decompression mode in progress (eg,
   psychoacoustic settings, channel setup, options, codebook
   etc).
*********************************************************************/

#include "highlevel.h"
typedef struct codec_setup_info {

  /* Vorbis supports only short and i64 blocks, but allows the
     encoder to choose the sizes */

  i64 blocksizes[2];

  /* modes are the primary means of supporting on-the-fly different
     blocksizes, different channel mappings (LR or M/A),
     different residue backends, etc.  Each mode consists of a
     blocksize flag and a mapping (along with the mapping setup */

  i32        modes;
  i32        maps;
  i32        floors;
  i32        residues;
  i32        books;
  i32        psys;     /* encode only */

  vorbis_info_mode       *mode_param[64];
  i32                     map_type[64];
  vorbis_info_mapping    *map_param[64];
  i32                     floor_type[64];
  vorbis_info_floor      *floor_param[64];
  i32                     residue_type[64];
  vorbis_info_residue    *residue_param[64];
  static_codebook        *book_param[256];
  codebook               *fullbooks;

  vorbis_info_psy        *psy_param[4]; /* encode only */
  vorbis_info_psy_global psy_g_param;

  bitrate_manager_info   bi;
  highlevel_encode_setup hi; /* used only by vorbisenc.c.  It's a
                                highly redundant structure, but
                                improves clarity of program flow. */
  i32         halfrate_flag; /* painless downsample for decode */
} codec_setup_info;

extern vorbis_look_psy_global *_vp_global_look(vorbis_info *vi);
extern z0 _vp_global_free(vorbis_look_psy_global *look);



typedef struct {
  i32 sorted_index[VIF_POSIT+2];
  i32 forward_index[VIF_POSIT+2];
  i32 reverse_index[VIF_POSIT+2];

  i32 hineighbor[VIF_POSIT];
  i32 loneighbor[VIF_POSIT];
  i32 posts;

  i32 n;
  i32 quant_q;
  vorbis_info_floor1 *vi;

  i64 phrasebits;
  i64 postbits;
  i64 frames;
} vorbis_look_floor1;



extern i32 *floor1_fit(vorbis_block *vb,vorbis_look_floor1 *look,
                          const f32 *logmdct,   /* in */
                          const f32 *logmask);
extern i32 *floor1_interpolate_fit(vorbis_block *vb,vorbis_look_floor1 *look,
                          i32 *A,i32 *B,
                          i32 del);
extern i32 floor1_encode(oggpack_buffer *opb,vorbis_block *vb,
                  vorbis_look_floor1 *look,
                  i32 *post,i32 *ilogmask);
#endif
