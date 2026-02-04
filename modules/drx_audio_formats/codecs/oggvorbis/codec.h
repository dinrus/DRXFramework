/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2001             *
 * by the Xiph.Org Foundation https://xiph.org/                     *

 ********************************************************************

 function: libvorbis codec headers

 ********************************************************************/

#ifndef _vorbis_codec_h_
#define _vorbis_codec_h_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "ogg.h"

typedef struct vorbis_info{
  i32 version;
  i32 channels;
  i64 rate;

  /* The below bitrate declarations are *hints*.
     Combinations of the three values carry the following implications:

     all three set to the same value:
       implies a fixed rate bitstream
     only nominal set:
       implies a VBR stream that averages the nominal bitrate.  No hard
       upper/lower limit
     upper and or lower set:
       implies a VBR bitstream that obeys the bitrate limits. nominal
       may also be set to give a nominal rate.
     none set:
       the coder does not care to speculate.
  */

  i64 bitrate_upper;
  i64 bitrate_nominal;
  i64 bitrate_lower;
  i64 bitrate_window;

  uk codec_setup;
} vorbis_info;

/* vorbis_dsp_state buffers the current vorbis audio
   analysis/synthesis state.  The DSP state belongs to a specific
   logical bitstream ****************************************************/
typedef struct vorbis_dsp_state{
  i32 analysisp;
  vorbis_info *vi;

  f32 **pcm;
  f32 **pcmret;
  i32      pcm_storage;
  i32      pcm_current;
  i32      pcm_returned;

  i32  preextrapolate;
  i32  eofflag;

  i64 lW;
  i64 W;
  i64 nW;
  i64 centerW;

  ogg_int64_t granulepos;
  ogg_int64_t sequence;

  ogg_int64_t glue_bits;
  ogg_int64_t time_bits;
  ogg_int64_t floor_bits;
  ogg_int64_t res_bits;

  z0       *backend_state;
} vorbis_dsp_state;

typedef struct vorbis_block{
  /* necessary stream state for linking to the framing abstraction */
  f32  **pcm;       /* this is a pointer into local storage */
  oggpack_buffer opb;

  i64  lW;
  i64  W;
  i64  nW;
  i32   pcmend;
  i32   mode;

  i32         eofflag;
  ogg_int64_t granulepos;
  ogg_int64_t sequence;
  vorbis_dsp_state *vd; /* For read-only access of configuration */

  /* local storage to avoid remallocing; it's up to the mapping to
     structure it */
  z0               *localstore;
  i64                localtop;
  i64                localalloc;
  i64                totaluse;
  struct alloc_chain *reap;

  /* bitmetrics for the frame */
  i64 glue_bits;
  i64 time_bits;
  i64 floor_bits;
  i64 res_bits;

  uk internal;

} vorbis_block;

/* vorbis_block is a single block of data to be processed as part of
the analysis/synthesis stream; it belongs to a specific logical
bitstream, but is independent from other vorbis_blocks belonging to
that logical bitstream. *************************************************/

struct alloc_chain{
  uk ptr;
  struct alloc_chain *next;
};

/* vorbis_info contains all the setup information specific to the
   specific compression/decompression mode in progress (eg,
   psychoacoustic settings, channel setup, options, codebook
   etc). vorbis_info and substructures are in backends.h.
*********************************************************************/

/* the comments are not part of vorbis_info so that vorbis_info can be
   static storage */
typedef struct vorbis_comment{
  /* unlimited user comment fields.  libvorbis writes 'libvorbis'
     whatever vendor is set to in encode */
  t8 **user_comments;
  i32   *comment_lengths;
  i32    comments;
  t8  *vendor;

} vorbis_comment;


/* libvorbis encodes in two abstraction layers; first we perform DSP
   and produce a packet (see docs/analysis.txt).  The packet is then
   coded into a framed OggSquish bitstream by the second layer (see
   docs/framing.txt).  Decode is the reverse process; we sync/frame
   the bitstream and extract individual packets, then decode the
   packet back into PCM audio.

   The extra framing/packetizing is used in streaming formats, such as
   files.  Over the net (such as with UDP), the framing and
   packetization aren't necessary as they're provided by the transport
   and the streaming layer is not used */

/* Vorbis PRIMITIVES: general ***************************************/

extern z0     vorbis_info_init(vorbis_info *vi);
extern z0     vorbis_info_clear(vorbis_info *vi);
extern i32      vorbis_info_blocksize(vorbis_info *vi,i32 zo);
extern z0     vorbis_comment_init(vorbis_comment *vc);
extern z0     vorbis_comment_add(vorbis_comment *vc, const t8 *comment);
extern z0     vorbis_comment_add_tag(vorbis_comment *vc,
                                       const t8 *tag, const t8 *contents);
extern t8    *vorbis_comment_query(vorbis_comment *vc, const t8 *tag, i32 count);
extern i32      vorbis_comment_query_count(vorbis_comment *vc, const t8 *tag);
extern z0     vorbis_comment_clear(vorbis_comment *vc);

extern i32      vorbis_block_init(vorbis_dsp_state *v, vorbis_block *vb);
extern i32      vorbis_block_clear(vorbis_block *vb);
extern z0     vorbis_dsp_clear(vorbis_dsp_state *v);
extern f64   vorbis_granule_time(vorbis_dsp_state *v,
                                    ogg_int64_t granulepos);

extern const t8 *vorbis_version_string(z0);

/* Vorbis PRIMITIVES: analysis/DSP layer ****************************/

extern i32      vorbis_analysis_init(vorbis_dsp_state *v,vorbis_info *vi);
extern i32      vorbis_commentheader_out(vorbis_comment *vc, ogg_packet *op);
extern i32      vorbis_analysis_headerout(vorbis_dsp_state *v,
                                          vorbis_comment *vc,
                                          ogg_packet *op,
                                          ogg_packet *op_comm,
                                          ogg_packet *op_code);
extern f32  **vorbis_analysis_buffer(vorbis_dsp_state *v,i32 vals);
extern i32      vorbis_analysis_wrote(vorbis_dsp_state *v,i32 vals);
extern i32      vorbis_analysis_blockout(vorbis_dsp_state *v,vorbis_block *vb);
extern i32      vorbis_analysis(vorbis_block *vb,ogg_packet *op);

extern i32      vorbis_bitrate_addblock(vorbis_block *vb);
extern i32      vorbis_bitrate_flushpacket(vorbis_dsp_state *vd,
                                           ogg_packet *op);

/* Vorbis PRIMITIVES: synthesis layer *******************************/
extern i32      vorbis_synthesis_idheader(ogg_packet *op);
extern i32      vorbis_synthesis_headerin(vorbis_info *vi,vorbis_comment *vc,
                                          ogg_packet *op);

extern i32      vorbis_synthesis_init(vorbis_dsp_state *v,vorbis_info *vi);
extern i32      vorbis_synthesis_restart(vorbis_dsp_state *v);
extern i32      vorbis_synthesis(vorbis_block *vb,ogg_packet *op);
extern i32      vorbis_synthesis_trackonly(vorbis_block *vb,ogg_packet *op);
extern i32      vorbis_synthesis_blockin(vorbis_dsp_state *v,vorbis_block *vb);
extern i32      vorbis_synthesis_pcmout(vorbis_dsp_state *v,f32 ***pcm);
extern i32      vorbis_synthesis_lapout(vorbis_dsp_state *v,f32 ***pcm);
extern i32      vorbis_synthesis_read(vorbis_dsp_state *v,i32 samples);
extern i64     vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op);

extern i32      vorbis_synthesis_halfrate(vorbis_info *v,i32 flag);
extern i32      vorbis_synthesis_halfrate_p(vorbis_info *v);

/* Vorbis ERRORS and return codes ***********************************/

#define OV_FALSE      -1
#define OV_EOF        -2
#define OV_HOLE       -3

#define OV_EREAD      -128
#define OV_EFAULT     -129
#define OV_EIMPL      -130
#define OV_EINVAL     -131
#define OV_ENOTVORBIS -132
#define OV_EBADHEADER -133
#define OV_EVERSION   -134
#define OV_ENOTAUDIO  -135
#define OV_EBADPACKET -136
#define OV_EBADLINK   -137
#define OV_ENOSEEK    -138

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
