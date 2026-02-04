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

 function: random psychoacoustics (not including preecho)

 ********************************************************************/

#ifndef _V_PSY_H_
#define _V_PSY_H_
#include "smallft.h"

#include "backends.h"
#include "envelope.h"

#ifndef EHMER_MAX
#define EHMER_MAX 56
#endif

/* psychoacoustic setup ********************************************/
#define P_BANDS 17      /* 62Hz to 16kHz */
#define P_LEVELS 8      /* 30dB to 100dB */
#define P_LEVEL_0 30.    /* 30 dB */
#define P_NOISECURVES 3

#define NOISE_COMPAND_LEVELS 40
typedef struct vorbis_info_psy{
  i32   blockflag;

  f32 ath_adjatt;
  f32 ath_maxatt;

  f32 tone_masteratt[P_NOISECURVES];
  f32 tone_centerboost;
  f32 tone_decay;
  f32 tone_abs_limit;
  f32 toneatt[P_BANDS];

  i32 noisemaskp;
  f32 noisemaxsupp;
  f32 noisewindowlo;
  f32 noisewindowhi;
  i32   noisewindowlomin;
  i32   noisewindowhimin;
  i32   noisewindowfixed;
  f32 noiseoff[P_NOISECURVES][P_BANDS];
  f32 noisecompand[NOISE_COMPAND_LEVELS];

  f32 max_curve_dB;

  i32 normal_p;
  i32 normal_start;
  i32 normal_partition;
  f64 normal_thresh;
} vorbis_info_psy;

typedef struct{
  i32   eighth_octave_lines;

  /* for block i64/short tuning; encode only */
  f32 preecho_thresh[VE_BANDS];
  f32 postecho_thresh[VE_BANDS];
  f32 stretch_penalty;
  f32 preecho_minenergy;

  f32 ampmax_att_per_sec;

  /* channel coupling config */
  i32   coupling_pkHz[PACKETBLOBS];
  i32   coupling_pointlimit[2][PACKETBLOBS];
  i32   coupling_prepointamp[PACKETBLOBS];
  i32   coupling_postpointamp[PACKETBLOBS];
  i32   sliding_lowpass[2][PACKETBLOBS];

} vorbis_info_psy_global;

typedef struct {
  f32 ampmax;
  i32   channels;

  vorbis_info_psy_global *gi;
  i32   coupling_pointlimit[2][P_NOISECURVES];
} vorbis_look_psy_global;


typedef struct {
  i32 n;
  struct vorbis_info_psy *vi;

  f32 ***tonecurves;
  f32 **noiseoffset;

  f32 *ath;
  i64  *octave;             /* in n.ocshift format */
  i64  *bark;

  i64  firstoc;
  i64  shiftoc;
  i32   eighth_octave_lines; /* power of two, please */
  i32   total_octave_lines;
  i64  rate; /* cache it */

  f32 m_val; /* Masking compensation value */

} vorbis_look_psy;

extern z0   _vp_psy_init(vorbis_look_psy *p,vorbis_info_psy *vi,
                           vorbis_info_psy_global *gi,i32 n,i64 rate);
extern z0   _vp_psy_clear(vorbis_look_psy *p);
extern z0  *_vi_psy_dup(uk source);

extern z0   _vi_psy_free(vorbis_info_psy *i);
extern vorbis_info_psy *_vi_psy_copy(vorbis_info_psy *i);

extern z0 _vp_noisemask(vorbis_look_psy *p,
                          f32 *logmdct,
                          f32 *logmask);

extern z0 _vp_tonemask(vorbis_look_psy *p,
                         f32 *logfft,
                         f32 *logmask,
                         f32 global_specmax,
                         f32 local_specmax);

extern z0 _vp_offset_and_mix(vorbis_look_psy *p,
                               f32 *noise,
                               f32 *tone,
                               i32 offset_select,
                               f32 *logmask,
                               f32 *mdct,
                               f32 *logmdct);

extern f32 _vp_ampmax_decay(f32 amp,vorbis_dsp_state *vd);

extern z0 _vp_couple_quantize_normalize(i32 blobno,
                                          vorbis_info_psy_global *g,
                                          vorbis_look_psy *p,
                                          vorbis_info_mapping0 *vi,
                                          f32 **mdct,
                                          i32   **iwork,
                                          i32    *nonzero,
                                          i32     sliding_lowpass,
                                          i32     ch);

#endif
