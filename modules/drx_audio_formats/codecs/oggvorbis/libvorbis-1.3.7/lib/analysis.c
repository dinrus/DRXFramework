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

 function: single-block PCM analysis mode dispatch

 ********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../../ogg.h"
#include "../../codec.h"
#include "codec_internal.h"
#include "registry.h"
#include "scales.h"
#include "os.h"
#include "misc.h"

/* decides between modes, dispatches to the appropriate mapping. */
i32 vorbis_analysis(vorbis_block *vb, ogg_packet *op){
  i32 ret,i;
  vorbis_block_internal *vbi=(vorbis_block_internal *)vb->internal;

  vb->glue_bits=0;
  vb->time_bits=0;
  vb->floor_bits=0;
  vb->res_bits=0;

  /* first things first.  Make sure encode is ready */
  for(i=0;i<PACKETBLOBS;i++)
    oggpack_reset(vbi->packetblob[i]);

  /* we only have one mapping type (0), and we let the mapping code
     itself figure out what soft mode to use.  This allows easier
     bitrate management */

  if((ret=_mapping_P[0]->forward(vb)))
    return(ret);

  if(op){
    if(vorbis_bitrate_managed(vb))
      /* The app is using a bitmanaged mode... but not using the
         bitrate management interface. */
      return(OV_EINVAL);

    op->packet=oggpack_get_buffer(&vb->opb);
    op->bytes=oggpack_bytes(&vb->opb);
    op->b_o_s=0;
    op->e_o_s=vb->eofflag;
    op->granulepos=vb->granulepos;
    op->packetno=vb->sequence; /* for sake of completeness */
  }
  return(0);
}

#ifdef ANALYSIS
i32 analysis_noisy=1;

/* there was no great place to put this.... */
z0 _analysis_output_always(t8 *base,i32 i,f32 *v,i32 n,i32 bark,i32 dB,ogg_int64_t off){
  i32 j;
  FILE *of;
  t8 buffer[80];

  sprintf(buffer,"%s_%d.m",base,i);
  of=fopen(buffer,"w");

  if(!of)perror("failed to open data dump file");

  for(j=0;j<n;j++){
    if(bark){
      f32 b=toBARK((4000.f*j/n)+.25);
      fprintf(of,"%f ",b);
    }else
      if(off!=0)
        fprintf(of,"%f ",(f64)(j+off)/8000.);
      else
        fprintf(of,"%f ",(f64)j);

    if(dB){
      f32 val;
      if(v[j]==0.)
        val=-140.;
      else
        val=todB(v+j);
      fprintf(of,"%f\n",val);
    }else{
      fprintf(of,"%f\n",v[j]);
    }
  }
  fclose(of);
}

z0 _analysis_output(t8 *base,i32 i,f32 *v,i32 n,i32 bark,i32 dB,
                      ogg_int64_t off){
  if(analysis_noisy)_analysis_output_always(base,i,v,n,bark,dB,off);
}

#endif
