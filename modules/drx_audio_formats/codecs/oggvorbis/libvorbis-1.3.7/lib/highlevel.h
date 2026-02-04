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

 function: highlevel encoder setup struct separated out for vorbisenc clarity

 ********************************************************************/

typedef struct highlevel_byblocktype {
  f64 tone_mask_setting;
  f64 tone_peaklimit_setting;
  f64 noise_bias_setting;
  f64 noise_compand_setting;
} highlevel_byblocktype;

typedef struct highlevel_encode_setup {
  i32   set_in_stone;
  ukk setup;
  f64 base_setting;

  f64 impulse_noisetune;

  /* bitrate management below all settable */
  f32  req;
  i32    managed;
  i64   bitrate_min;
  i64   bitrate_av;
  f64 bitrate_av_damp;
  i64   bitrate_max;
  i64   bitrate_reservoir;
  f64 bitrate_reservoir_bias;

  i32 impulse_block_p;
  i32 noise_normalize_p;
  i32 coupling_p;

  f64 stereo_point_setting;
  f64 lowpass_kHz;
  i32    lowpass_altered;

  f64 ath_floating_dB;
  f64 ath_absolute_dB;

  f64 amplitude_track_dBpersec;
  f64 trigger_setting;

  highlevel_byblocktype block[4]; /* padding, impulse, transition, i64 */

} highlevel_encode_setup;
