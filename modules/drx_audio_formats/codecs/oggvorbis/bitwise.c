/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE Ogg CONTAINER SOURCE CODE.              *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2014             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function: packing variable sized words into an octet stream

 ********************************************************************/

#ifdef DRX_MSVC
 #pragma warning (disable: 4456 4457 4459)
#endif

/* We're 'LSb' endian; if we write a word but read individual bits,
   then we'll read the lsb first */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "ogg.h"

#define BUFFER_INCREMENT 256

static const u64 mask[]=
{0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
 0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
 0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
 0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
 0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
 0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
 0x3fffffff,0x7fffffff,0xffffffff };

static u32k mask8B[]=
{0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

z0 oggpack_writeinit(oggpack_buffer *b){
  memset(b,0,sizeof(*b));
  b->ptr=b->buffer=(u8*)_ogg_malloc(BUFFER_INCREMENT);
  b->buffer[0]='\0';
  b->storage=BUFFER_INCREMENT;
}

z0 oggpackB_writeinit(oggpack_buffer *b){
  oggpack_writeinit(b);
}

i32 oggpack_writecheck(oggpack_buffer *b){
  if(!b->ptr || !b->storage)return -1;
  return 0;
}

i32 oggpackB_writecheck(oggpack_buffer *b){
  return oggpack_writecheck(b);
}

z0 oggpack_writetrunc(oggpack_buffer *b,i64 bits){
  i64 bytes=bits>>3;
  if(b->ptr){
    bits-=bytes*8;
    b->ptr=b->buffer+bytes;
    b->endbit=bits;
    b->endbyte=bytes;
    *b->ptr&=mask[bits];
  }
}

z0 oggpackB_writetrunc(oggpack_buffer *b,i64 bits){
  i64 bytes=bits>>3;
  if(b->ptr){
    bits-=bytes*8;
    b->ptr=b->buffer+bytes;
    b->endbit=bits;
    b->endbyte=bytes;
    *b->ptr&=mask8B[bits];
  }
}

/* Takes only up to 32 bits. */
z0 oggpack_write(oggpack_buffer *b,u64 value,i32 bits){
  if(bits<0 || bits>32) goto err;
  if(b->endbyte>=b->storage-4){
    uk ret;
    if(!b->ptr)return;
    if(b->storage>LONG_MAX-BUFFER_INCREMENT) goto err;
    ret=_ogg_realloc(b->buffer,b->storage+BUFFER_INCREMENT);
    if(!ret) goto err;
    b->buffer=(u8*)ret;
    b->storage+=BUFFER_INCREMENT;
    b->ptr=b->buffer+b->endbyte;
  }

  value&=mask[bits];
  bits+=b->endbit;

  b->ptr[0]|=value<<b->endbit;

  if(bits>=8){
    b->ptr[1]=(u8)(value>>(8-b->endbit));
    if(bits>=16){
      b->ptr[2]=(u8)(value>>(16-b->endbit));
      if(bits>=24){
        b->ptr[3]=(u8)(value>>(24-b->endbit));
        if(bits>=32){
          if(b->endbit)
            b->ptr[4]=(u8)(value>>(32-b->endbit));
          else
            b->ptr[4]=0;
        }
      }
    }
  }

  b->endbyte+=bits/8;
  b->ptr+=bits/8;
  b->endbit=bits&7;
  return;
 err:
  oggpack_writeclear(b);
}

/* Takes only up to 32 bits. */
z0 oggpackB_write(oggpack_buffer *b,u64 value,i32 bits){
  if(bits<0 || bits>32) goto err;
  if(b->endbyte>=b->storage-4){
    uk ret;
    if(!b->ptr)return;
    if(b->storage>LONG_MAX-BUFFER_INCREMENT) goto err;
    ret=_ogg_realloc(b->buffer,b->storage+BUFFER_INCREMENT);
    if(!ret) goto err;
    b->buffer=(u8*)ret;
    b->storage+=BUFFER_INCREMENT;
    b->ptr=b->buffer+b->endbyte;
  }

  value=(value&mask[bits])<<(32-bits);
  bits+=b->endbit;

  b->ptr[0]|=value>>(24+b->endbit);

  if(bits>=8){
    b->ptr[1]=(u8)(value>>(16+b->endbit));
    if(bits>=16){
      b->ptr[2]=(u8)(value>>(8+b->endbit));
      if(bits>=24){
        b->ptr[3]=(u8)(value>>(b->endbit));
        if(bits>=32){
          if(b->endbit)
            b->ptr[4]=(u8)(value<<(8-b->endbit));
          else
            b->ptr[4]=0;
        }
      }
    }
  }

  b->endbyte+=bits/8;
  b->ptr+=bits/8;
  b->endbit=bits&7;
  return;
 err:
  oggpack_writeclear(b);
}

z0 oggpack_writealign(oggpack_buffer *b){
  i32 bits=8-b->endbit;
  if(bits<8)
    oggpack_write(b,0,bits);
}

z0 oggpackB_writealign(oggpack_buffer *b){
  i32 bits=8-b->endbit;
  if(bits<8)
    oggpackB_write(b,0,bits);
}

static z0 oggpack_writecopy_helper(oggpack_buffer *b,
                                     uk source,
                                     i64 bits,
                                     z0 (*w)(oggpack_buffer *,
                                               u64,
                                               i32),
                                     i32 msb){
  u8 *ptr=(u8 *)source;

  i64 bytes=bits/8;
  i64 pbytes=(b->endbit+bits)/8;
  bits-=bytes*8;

  /* expand storage up-front */
  if(b->endbyte+pbytes>=b->storage){
    uk ret;
    if(!b->ptr) goto err;
    if(b->storage>b->endbyte+pbytes+BUFFER_INCREMENT) goto err;
    b->storage=b->endbyte+pbytes+BUFFER_INCREMENT;
    ret=_ogg_realloc(b->buffer,b->storage);
    if(!ret) goto err;
    b->buffer=(u8*)ret;
    b->ptr=b->buffer+b->endbyte;
  }

  /* copy whole octets */
  if(b->endbit){
    i32 i;
    /* unaligned copy.  Do it the hard way. */
    for(i=0;i<bytes;i++)
      w(b,(u64)(ptr[i]),8);
  }else{
    /* aligned block copy */
    memmove(b->ptr,source,bytes);
    b->ptr+=bytes;
    b->endbyte+=bytes;
    *b->ptr=0;
  }

  /* copy trailing bits */
  if(bits){
    if(msb)
      w(b,(u64)(ptr[bytes]>>(8-bits)),bits);
    else
      w(b,(u64)(ptr[bytes]),bits);
  }
  return;
 err:
  oggpack_writeclear(b);
}

z0 oggpack_writecopy(oggpack_buffer *b,uk source,i64 bits){
  oggpack_writecopy_helper(b,source,bits,oggpack_write,0);
}

z0 oggpackB_writecopy(oggpack_buffer *b,uk source,i64 bits){
  oggpack_writecopy_helper(b,source,bits,oggpackB_write,1);
}

z0 oggpack_reset(oggpack_buffer *b){
  if(!b->ptr)return;
  b->ptr=b->buffer;
  b->buffer[0]=0;
  b->endbit=b->endbyte=0;
}

z0 oggpackB_reset(oggpack_buffer *b){
  oggpack_reset(b);
}

z0 oggpack_writeclear(oggpack_buffer *b){
  if(b->buffer)_ogg_free(b->buffer);
  memset(b,0,sizeof(*b));
}

z0 oggpackB_writeclear(oggpack_buffer *b){
  oggpack_writeclear(b);
}

z0 oggpack_readinit(oggpack_buffer *b,u8 *buf,i32 bytes){
  memset(b,0,sizeof(*b));
  b->buffer=b->ptr=buf;
  b->storage=bytes;
}

z0 oggpackB_readinit(oggpack_buffer *b,u8 *buf,i32 bytes){
  oggpack_readinit(b,buf,bytes);
}

/* Read in bits without advancing the bitptr; bits <= 32 */
i64 oggpack_look(oggpack_buffer *b,i32 bits){
  u64 ret;
  u64 m;

  if(bits<0 || bits>32) return -1;
  m=mask[bits];
  bits+=b->endbit;

  if(b->endbyte >= b->storage-4){
    /* not the main path */
    if(b->endbyte > b->storage-((bits+7)>>3)) return -1;
    /* special case to avoid reading b->ptr[0], which might be past the end of
        the buffer; also skips some useless accounting */
    else if(!bits)return(0L);
  }

  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);
      if(bits>24){
        ret|=b->ptr[3]<<(24-b->endbit);
        if(bits>32 && b->endbit)
          ret|=b->ptr[4]<<(32-b->endbit);
      }
    }
  }
  return(m&ret);
}

/* Read in bits without advancing the bitptr; bits <= 32 */
i64 oggpackB_look(oggpack_buffer *b,i32 bits){
  u64 ret;
  i32 m=32-bits;

  if(m<0 || m>32) return -1;
  bits+=b->endbit;

  if(b->endbyte >= b->storage-4){
    /* not the main path */
    if(b->endbyte > b->storage-((bits+7)>>3)) return -1;
    /* special case to avoid reading b->ptr[0], which might be past the end of
        the buffer; also skips some useless accounting */
    else if(!bits)return(0L);
  }

  ret=b->ptr[0]<<(24+b->endbit);
  if(bits>8){
    ret|=b->ptr[1]<<(16+b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(8+b->endbit);
      if(bits>24){
        ret|=b->ptr[3]<<(b->endbit);
        if(bits>32 && b->endbit)
          ret|=b->ptr[4]>>(8-b->endbit);
      }
    }
  }
  return ((ret&0xffffffff)>>(m>>1))>>((m+1)>>1);
}

i64 oggpack_look1(oggpack_buffer *b){
  if(b->endbyte>=b->storage)return(-1);
  return((b->ptr[0]>>b->endbit)&1);
}

i64 oggpackB_look1(oggpack_buffer *b){
  if(b->endbyte>=b->storage)return(-1);
  return((b->ptr[0]>>(7-b->endbit))&1);
}

z0 oggpack_adv(oggpack_buffer *b,i32 bits){
  bits+=b->endbit;

  if(b->endbyte > b->storage-((bits+7)>>3)) goto overflow;

  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return;

 overflow:
  b->ptr=NULL;
  b->endbyte=b->storage;
  b->endbit=1;
}

z0 oggpackB_adv(oggpack_buffer *b,i32 bits){
  oggpack_adv(b,bits);
}

z0 oggpack_adv1(oggpack_buffer *b){
  if(++(b->endbit)>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
}

z0 oggpackB_adv1(oggpack_buffer *b){
  oggpack_adv1(b);
}

/* bits <= 32 */
i64 oggpack_read(oggpack_buffer *b,i32 bits){
  i64 ret;
  u64 m;

  if(bits<0 || bits>32) goto err;
  m=mask[bits];
  bits+=b->endbit;

  if(b->endbyte >= b->storage-4){
    /* not the main path */
    if(b->endbyte > b->storage-((bits+7)>>3)) goto overflow;
    /* special case to avoid reading b->ptr[0], which might be past the end of
        the buffer; also skips some useless accounting */
    else if(!bits)return(0L);
  }

  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);
      if(bits>24){
        ret|=b->ptr[3]<<(24-b->endbit);
        if(bits>32 && b->endbit){
          ret|=b->ptr[4]<<(32-b->endbit);
        }
      }
    }
  }
  ret&=m;
  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return ret;

 overflow:
 err:
  b->ptr=NULL;
  b->endbyte=b->storage;
  b->endbit=1;
  return -1L;
}

/* bits <= 32 */
i64 oggpackB_read(oggpack_buffer *b,i32 bits){
  i64 ret;
  i64 m=32-bits;

  if(m<0 || m>32) goto err;
  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    if(b->endbyte > b->storage-((bits+7)>>3)) goto overflow;
    /* special case to avoid reading b->ptr[0], which might be past the end of
        the buffer; also skips some useless accounting */
    else if(!bits)return(0L);
  }

  ret=b->ptr[0]<<(24+b->endbit);
  if(bits>8){
    ret|=b->ptr[1]<<(16+b->endbit);
    if(bits>16){
      ret|=b->ptr[2]<<(8+b->endbit);
      if(bits>24){
        ret|=b->ptr[3]<<(b->endbit);
        if(bits>32 && b->endbit)
          ret|=b->ptr[4]>>(8-b->endbit);
      }
    }
  }
  ret=((ret&0xffffffffUL)>>(m>>1))>>((m+1)>>1);

  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return ret;

 overflow:
 err:
  b->ptr=NULL;
  b->endbyte=b->storage;
  b->endbit=1;
  return -1L;
}

i64 oggpack_read1(oggpack_buffer *b){
  i64 ret;

  if(b->endbyte >= b->storage) goto overflow;
  ret=(b->ptr[0]>>b->endbit)&1;

  b->endbit++;
  if(b->endbit>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
  return ret;

 overflow:
  b->ptr=NULL;
  b->endbyte=b->storage;
  b->endbit=1;
  return -1L;
}

i64 oggpackB_read1(oggpack_buffer *b){
  i64 ret;

  if(b->endbyte >= b->storage) goto overflow;
  ret=(b->ptr[0]>>(7-b->endbit))&1;

  b->endbit++;
  if(b->endbit>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
  return ret;

 overflow:
  b->ptr=NULL;
  b->endbyte=b->storage;
  b->endbit=1;
  return -1L;
}

i64 oggpack_bytes(oggpack_buffer *b){
  return(b->endbyte+(b->endbit+7)/8);
}

i64 oggpack_bits(oggpack_buffer *b){
  return(b->endbyte*8+b->endbit);
}

i64 oggpackB_bytes(oggpack_buffer *b){
  return oggpack_bytes(b);
}

i64 oggpackB_bits(oggpack_buffer *b){
  return oggpack_bits(b);
}

u8 *oggpack_get_buffer(oggpack_buffer *b){
  return(b->buffer);
}

u8 *oggpackB_get_buffer(oggpack_buffer *b){
  return oggpack_get_buffer(b);
}

/* Self test of the bitwise routines; everything else is based on
   them, so they damned well better be solid. */

#ifdef _V_SELFTEST
#include <stdio.h>

static i32 ilog(u32 v){
  i32 ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

oggpack_buffer o;
oggpack_buffer r;

z0 report(t8 *in){
  fprintf(stderr,"%s",in);
  exit(1);
}

z0 cliptest(u64 *b,i32 vals,i32 bits,i32 *comp,i32 compsize){
  i64 bytes,i;
  u8 *buffer;

  oggpack_reset(&o);
  for(i=0;i<vals;i++)
    oggpack_write(&o,b[i],bits?bits:ilog(b[i]));
  buffer=oggpack_get_buffer(&o);
  bytes=oggpack_bytes(&o);
  if(bytes!=compsize)report("wrong number of bytes!\n");
  for(i=0;i<bytes;i++)if(buffer[i]!=comp[i]){
    for(i=0;i<bytes;i++)fprintf(stderr,"%x %x\n",(i32)buffer[i],(i32)comp[i]);
    report("wrote incorrect value!\n");
  }
  oggpack_readinit(&r,buffer,bytes);
  for(i=0;i<vals;i++){
    i32 tbit=bits?bits:ilog(b[i]);
    if(oggpack_look(&r,tbit)==-1)
      report("out of data!\n");
    if(oggpack_look(&r,tbit)!=(b[i]&mask[tbit]))
      report("looked at incorrect value!\n");
    if(tbit==1)
      if(oggpack_look1(&r)!=(b[i]&mask[tbit]))
        report("looked at single bit incorrect value!\n");
    if(tbit==1){
      if(oggpack_read1(&r)!=(b[i]&mask[tbit]))
        report("read incorrect single bit value!\n");
    }else{
    if(oggpack_read(&r,tbit)!=(b[i]&mask[tbit]))
      report("read incorrect value!\n");
    }
  }
  if(oggpack_bytes(&r)!=bytes)report("leftover bytes after read!\n");
}

z0 cliptestB(u64 *b,i32 vals,i32 bits,i32 *comp,i32 compsize){
  i64 bytes,i;
  u8 *buffer;

  oggpackB_reset(&o);
  for(i=0;i<vals;i++)
    oggpackB_write(&o,b[i],bits?bits:ilog(b[i]));
  buffer=oggpackB_get_buffer(&o);
  bytes=oggpackB_bytes(&o);
  if(bytes!=compsize)report("wrong number of bytes!\n");
  for(i=0;i<bytes;i++)if(buffer[i]!=comp[i]){
    for(i=0;i<bytes;i++)fprintf(stderr,"%x %x\n",(i32)buffer[i],(i32)comp[i]);
    report("wrote incorrect value!\n");
  }
  oggpackB_readinit(&r,buffer,bytes);
  for(i=0;i<vals;i++){
    i32 tbit=bits?bits:ilog(b[i]);
    if(oggpackB_look(&r,tbit)==-1)
      report("out of data!\n");
    if(oggpackB_look(&r,tbit)!=(b[i]&mask[tbit]))
      report("looked at incorrect value!\n");
    if(tbit==1)
      if(oggpackB_look1(&r)!=(b[i]&mask[tbit]))
        report("looked at single bit incorrect value!\n");
    if(tbit==1){
      if(oggpackB_read1(&r)!=(b[i]&mask[tbit]))
        report("read incorrect single bit value!\n");
    }else{
    if(oggpackB_read(&r,tbit)!=(b[i]&mask[tbit]))
      report("read incorrect value!\n");
    }
  }
  if(oggpackB_bytes(&r)!=bytes)report("leftover bytes after read!\n");
}

z0 copytest(i32 prefill, i32 copy){
  oggpack_buffer source_write;
  oggpack_buffer dest_write;
  oggpack_buffer source_read;
  oggpack_buffer dest_read;
  u8 *source;
  u8 *dest;
  i64 source_bytes,dest_bytes;
  i32 i;

  oggpack_writeinit(&source_write);
  oggpack_writeinit(&dest_write);

  for(i=0;i<(prefill+copy+7)/8;i++)
    oggpack_write(&source_write,(i^0x5a)&0xff,8);
  source=oggpack_get_buffer(&source_write);
  source_bytes=oggpack_bytes(&source_write);

  /* prefill */
  oggpack_writecopy(&dest_write,source,prefill);

  /* check buffers; verify end byte masking */
  dest=oggpack_get_buffer(&dest_write);
  dest_bytes=oggpack_bytes(&dest_write);
  if(dest_bytes!=(prefill+7)/8){
    fprintf(stderr,"wrong number of bytes after prefill! %ld!=%d\n",dest_bytes,(prefill+7)/8);
    exit(1);
  }
  oggpack_readinit(&source_read,source,source_bytes);
  oggpack_readinit(&dest_read,dest,dest_bytes);

  for(i=0;i<prefill;i+=8){
    i32 s=oggpack_read(&source_read,prefill-i<8?prefill-i:8);
    i32 d=oggpack_read(&dest_read,prefill-i<8?prefill-i:8);
    if(s!=d){
      fprintf(stderr,"prefill=%d mismatch! byte %d, %x!=%x\n",prefill,i/8,s,d);
      exit(1);
    }
  }
  if(prefill<dest_bytes){
    if(oggpack_read(&dest_read,dest_bytes-prefill)!=0){
      fprintf(stderr,"prefill=%d mismatch! trailing bits not zero\n",prefill);
      exit(1);
    }
  }

  /* second copy */
  oggpack_writecopy(&dest_write,source,copy);

  /* check buffers; verify end byte masking */
  dest=oggpack_get_buffer(&dest_write);
  dest_bytes=oggpack_bytes(&dest_write);
  if(dest_bytes!=(copy+prefill+7)/8){
    fprintf(stderr,"wrong number of bytes after prefill+copy! %ld!=%d\n",dest_bytes,(copy+prefill+7)/8);
    exit(1);
  }
  oggpack_readinit(&source_read,source,source_bytes);
  oggpack_readinit(&dest_read,dest,dest_bytes);

  for(i=0;i<prefill;i+=8){
    i32 s=oggpack_read(&source_read,prefill-i<8?prefill-i:8);
    i32 d=oggpack_read(&dest_read,prefill-i<8?prefill-i:8);
    if(s!=d){
      fprintf(stderr,"prefill=%d mismatch! byte %d, %x!=%x\n",prefill,i/8,s,d);
      exit(1);
    }
  }

  oggpack_readinit(&source_read,source,source_bytes);
  for(i=0;i<copy;i+=8){
    i32 s=oggpack_read(&source_read,copy-i<8?copy-i:8);
    i32 d=oggpack_read(&dest_read,copy-i<8?copy-i:8);
    if(s!=d){
      fprintf(stderr,"prefill=%d copy=%d mismatch! byte %d, %x!=%x\n",prefill,copy,i/8,s,d);
      exit(1);
    }
  }

  if(copy+prefill<dest_bytes){
    if(oggpack_read(&dest_read,dest_bytes-copy-prefill)!=0){
      fprintf(stderr,"prefill=%d copy=%d mismatch! trailing bits not zero\n",prefill,copy);
      exit(1);
    }
  }

  oggpack_writeclear(&source_write);
  oggpack_writeclear(&dest_write);


}

z0 copytestB(i32 prefill, i32 copy){
  oggpack_buffer source_write;
  oggpack_buffer dest_write;
  oggpack_buffer source_read;
  oggpack_buffer dest_read;
  u8 *source;
  u8 *dest;
  i64 source_bytes,dest_bytes;
  i32 i;

  oggpackB_writeinit(&source_write);
  oggpackB_writeinit(&dest_write);

  for(i=0;i<(prefill+copy+7)/8;i++)
    oggpackB_write(&source_write,(i^0x5a)&0xff,8);
  source=oggpackB_get_buffer(&source_write);
  source_bytes=oggpackB_bytes(&source_write);

  /* prefill */
  oggpackB_writecopy(&dest_write,source,prefill);

  /* check buffers; verify end byte masking */
  dest=oggpackB_get_buffer(&dest_write);
  dest_bytes=oggpackB_bytes(&dest_write);
  if(dest_bytes!=(prefill+7)/8){
    fprintf(stderr,"wrong number of bytes after prefill! %ld!=%d\n",dest_bytes,(prefill+7)/8);
    exit(1);
  }
  oggpackB_readinit(&source_read,source,source_bytes);
  oggpackB_readinit(&dest_read,dest,dest_bytes);

  for(i=0;i<prefill;i+=8){
    i32 s=oggpackB_read(&source_read,prefill-i<8?prefill-i:8);
    i32 d=oggpackB_read(&dest_read,prefill-i<8?prefill-i:8);
    if(s!=d){
      fprintf(stderr,"prefill=%d mismatch! byte %d, %x!=%x\n",prefill,i/8,s,d);
      exit(1);
    }
  }
  if(prefill<dest_bytes){
    if(oggpackB_read(&dest_read,dest_bytes-prefill)!=0){
      fprintf(stderr,"prefill=%d mismatch! trailing bits not zero\n",prefill);
      exit(1);
    }
  }

  /* second copy */
  oggpackB_writecopy(&dest_write,source,copy);

  /* check buffers; verify end byte masking */
  dest=oggpackB_get_buffer(&dest_write);
  dest_bytes=oggpackB_bytes(&dest_write);
  if(dest_bytes!=(copy+prefill+7)/8){
    fprintf(stderr,"wrong number of bytes after prefill+copy! %ld!=%d\n",dest_bytes,(copy+prefill+7)/8);
    exit(1);
  }
  oggpackB_readinit(&source_read,source,source_bytes);
  oggpackB_readinit(&dest_read,dest,dest_bytes);

  for(i=0;i<prefill;i+=8){
    i32 s=oggpackB_read(&source_read,prefill-i<8?prefill-i:8);
    i32 d=oggpackB_read(&dest_read,prefill-i<8?prefill-i:8);
    if(s!=d){
      fprintf(stderr,"prefill=%d mismatch! byte %d, %x!=%x\n",prefill,i/8,s,d);
      exit(1);
    }
  }

  oggpackB_readinit(&source_read,source,source_bytes);
  for(i=0;i<copy;i+=8){
    i32 s=oggpackB_read(&source_read,copy-i<8?copy-i:8);
    i32 d=oggpackB_read(&dest_read,copy-i<8?copy-i:8);
    if(s!=d){
      fprintf(stderr,"prefill=%d copy=%d mismatch! byte %d, %x!=%x\n",prefill,copy,i/8,s,d);
      exit(1);
    }
  }

  if(copy+prefill<dest_bytes){
    if(oggpackB_read(&dest_read,dest_bytes-copy-prefill)!=0){
      fprintf(stderr,"prefill=%d copy=%d mismatch! trailing bits not zero\n",prefill,copy);
      exit(1);
    }
  }

  oggpackB_writeclear(&source_write);
  oggpackB_writeclear(&dest_write);

}

i32 main(z0){
  u8 *buffer;
  i64 bytes,i,j;
  static u64 testbuffer1[]=
    {18,12,103948,4325,543,76,432,52,3,65,4,56,32,42,34,21,1,23,32,546,456,7,
       567,56,8,8,55,3,52,342,341,4,265,7,67,86,2199,21,7,1,5,1,4};
  i32 test1size=43;

  static u64 testbuffer2[]=
    {216531625L,1237861823,56732452,131,3212421,12325343,34547562,12313212,
       1233432,534,5,346435231,14436467,7869299,76326614,167548585,
       85525151,0,12321,1,349528352};
  i32 test2size=21;

  static u64 testbuffer3[]=
    {1,0,14,0,1,0,12,0,1,0,0,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,1,1,1,0,0,1,
       0,1,30,1,1,1,0,0,1,0,0,0,12,0,11,0,1,0,0,1};
  i32 test3size=56;

  static u64 large[]=
    {2136531625L,2137861823,56732452,131,3212421,12325343,34547562,12313212,
       1233432,534,5,2146435231,14436467,7869299,76326614,167548585,
       85525151,0,12321,1,2146528352};

  i32 onesize=33;
  static i32 one[33]={146,25,44,151,195,15,153,176,233,131,196,65,85,172,47,40,
                    34,242,223,136,35,222,211,86,171,50,225,135,214,75,172,
                    223,4};
  static i32 oneB[33]={150,101,131,33,203,15,204,216,105,193,156,65,84,85,222,
                       8,139,145,227,126,34,55,244,171,85,100,39,195,173,18,
                       245,251,128};

  i32 twosize=6;
  static i32 two[6]={61,255,255,251,231,29};
  static i32 twoB[6]={247,63,255,253,249,120};

  i32 threesize=54;
  static i32 three[54]={169,2,232,252,91,132,156,36,89,13,123,176,144,32,254,
                      142,224,85,59,121,144,79,124,23,67,90,90,216,79,23,83,
                      58,135,196,61,55,129,183,54,101,100,170,37,127,126,10,
                      100,52,4,14,18,86,77,1};
  static i32 threeB[54]={206,128,42,153,57,8,183,251,13,89,36,30,32,144,183,
                         130,59,240,121,59,85,223,19,228,180,134,33,107,74,98,
                         233,253,196,135,63,2,110,114,50,155,90,127,37,170,104,
                         200,20,254,4,58,106,176,144,0};

  i32 foursize=38;
  static i32 four[38]={18,6,163,252,97,194,104,131,32,1,7,82,137,42,129,11,72,
                     132,60,220,112,8,196,109,64,179,86,9,137,195,208,122,169,
                     28,2,133,0,1};
  static i32 fourB[38]={36,48,102,83,243,24,52,7,4,35,132,10,145,21,2,93,2,41,
                        1,219,184,16,33,184,54,149,170,132,18,30,29,98,229,67,
                        129,10,4,32};

  i32 fivesize=45;
  static i32 five[45]={169,2,126,139,144,172,30,4,80,72,240,59,130,218,73,62,
                     241,24,210,44,4,20,0,248,116,49,135,100,110,130,181,169,
                     84,75,159,2,1,0,132,192,8,0,0,18,22};
  static i32 fiveB[45]={1,84,145,111,245,100,128,8,56,36,40,71,126,78,213,226,
                        124,105,12,0,133,128,0,162,233,242,67,152,77,205,77,
                        172,150,169,129,79,128,0,6,4,32,0,27,9,0};

  i32 sixsize=7;
  static i32 six[7]={17,177,170,242,169,19,148};
  static i32 sixB[7]={136,141,85,79,149,200,41};

  /* Test read/write together */
  /* Later we test against pregenerated bitstreams */
  oggpack_writeinit(&o);

  fprintf(stderr,"\nSmall preclipped packing (LSb): ");
  cliptest(testbuffer1,test1size,0,one,onesize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nNull bit call (LSb): ");
  cliptest(testbuffer3,test3size,0,two,twosize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nLarge preclipped packing (LSb): ");
  cliptest(testbuffer2,test2size,0,three,threesize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\n32 bit preclipped packing (LSb): ");
  oggpack_reset(&o);
  for(i=0;i<test2size;i++)
    oggpack_write(&o,large[i],32);
  buffer=oggpack_get_buffer(&o);
  bytes=oggpack_bytes(&o);
  oggpack_readinit(&r,buffer,bytes);
  for(i=0;i<test2size;i++){
    if(oggpack_look(&r,32)==-1)report("out of data. failed!");
    if(oggpack_look(&r,32)!=large[i]){
      fprintf(stderr,"%ld != %lu (%lx!=%lx):",oggpack_look(&r,32),large[i],
              oggpack_look(&r,32),large[i]);
      report("read incorrect value!\n");
    }
    oggpack_adv(&r,32);
  }
  if(oggpack_bytes(&r)!=bytes)report("leftover bytes after read!\n");
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nSmall unclipped packing (LSb): ");
  cliptest(testbuffer1,test1size,7,four,foursize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nLarge unclipped packing (LSb): ");
  cliptest(testbuffer2,test2size,17,five,fivesize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nSingle bit unclipped packing (LSb): ");
  cliptest(testbuffer3,test3size,1,six,sixsize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nTesting read past end (LSb): ");
  oggpack_readinit(&r,(u8 *)"\0\0\0\0\0\0\0\0",8);
  for(i=0;i<64;i++){
    if(oggpack_read(&r,1)!=0){
      fprintf(stderr,"failed; got -1 prematurely.\n");
      exit(1);
    }
  }
  if(oggpack_look(&r,1)!=-1 ||
     oggpack_read(&r,1)!=-1){
      fprintf(stderr,"failed; read past end without -1.\n");
      exit(1);
  }
  oggpack_readinit(&r,(u8 *)"\0\0\0\0\0\0\0\0",8);
  if(oggpack_read(&r,30)!=0 || oggpack_read(&r,16)!=0){
      fprintf(stderr,"failed 2; got -1 prematurely.\n");
      exit(1);
  }

  if(oggpack_look(&r,18)!=0 ||
     oggpack_look(&r,18)!=0){
    fprintf(stderr,"failed 3; got -1 prematurely.\n");
      exit(1);
  }
  if(oggpack_look(&r,19)!=-1 ||
     oggpack_look(&r,19)!=-1){
    fprintf(stderr,"failed; read past end without -1.\n");
      exit(1);
  }
  if(oggpack_look(&r,32)!=-1 ||
     oggpack_look(&r,32)!=-1){
    fprintf(stderr,"failed; read past end without -1.\n");
      exit(1);
  }
  oggpack_writeclear(&o);
  fprintf(stderr,"ok.");

  /* this is partly glassbox; we're mostly concerned about the allocation boundaries */

  fprintf(stderr,"\nTesting aligned writecopies (LSb): ");
  for(i=0;i<71;i++)
    for(j=0;j<5;j++)
      copytest(j*8,i);
  for(i=BUFFER_INCREMENT*8-71;i<BUFFER_INCREMENT*8+71;i++)
    for(j=0;j<5;j++)
      copytest(j*8,i);
  fprintf(stderr,"ok.      ");

  fprintf(stderr,"\nTesting unaligned writecopies (LSb): ");
  for(i=0;i<71;i++)
    for(j=1;j<40;j++)
      if(j&0x7)
        copytest(j,i);
  for(i=BUFFER_INCREMENT*8-71;i<BUFFER_INCREMENT*8+71;i++)
    for(j=1;j<40;j++)
      if(j&0x7)
        copytest(j,i);
  
  fprintf(stderr,"ok.      \n");


  /********** lazy, cut-n-paste retest with MSb packing ***********/

  /* Test read/write together */
  /* Later we test against pregenerated bitstreams */
  oggpackB_writeinit(&o);

  fprintf(stderr,"\nSmall preclipped packing (MSb): ");
  cliptestB(testbuffer1,test1size,0,oneB,onesize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nNull bit call (MSb): ");
  cliptestB(testbuffer3,test3size,0,twoB,twosize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nLarge preclipped packing (MSb): ");
  cliptestB(testbuffer2,test2size,0,threeB,threesize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\n32 bit preclipped packing (MSb): ");
  oggpackB_reset(&o);
  for(i=0;i<test2size;i++)
    oggpackB_write(&o,large[i],32);
  buffer=oggpackB_get_buffer(&o);
  bytes=oggpackB_bytes(&o);
  oggpackB_readinit(&r,buffer,bytes);
  for(i=0;i<test2size;i++){
    if(oggpackB_look(&r,32)==-1)report("out of data. failed!");
    if(oggpackB_look(&r,32)!=large[i]){
      fprintf(stderr,"%ld != %lu (%lx!=%lx):",oggpackB_look(&r,32),large[i],
              oggpackB_look(&r,32),large[i]);
      report("read incorrect value!\n");
    }
    oggpackB_adv(&r,32);
  }
  if(oggpackB_bytes(&r)!=bytes)report("leftover bytes after read!\n");
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nSmall unclipped packing (MSb): ");
  cliptestB(testbuffer1,test1size,7,fourB,foursize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nLarge unclipped packing (MSb): ");
  cliptestB(testbuffer2,test2size,17,fiveB,fivesize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nSingle bit unclipped packing (MSb): ");
  cliptestB(testbuffer3,test3size,1,sixB,sixsize);
  fprintf(stderr,"ok.");

  fprintf(stderr,"\nTesting read past end (MSb): ");
  oggpackB_readinit(&r,(u8 *)"\0\0\0\0\0\0\0\0",8);
  for(i=0;i<64;i++){
    if(oggpackB_read(&r,1)!=0){
      fprintf(stderr,"failed; got -1 prematurely.\n");
      exit(1);
    }
  }
  if(oggpackB_look(&r,1)!=-1 ||
     oggpackB_read(&r,1)!=-1){
      fprintf(stderr,"failed; read past end without -1.\n");
      exit(1);
  }
  oggpackB_readinit(&r,(u8 *)"\0\0\0\0\0\0\0\0",8);
  if(oggpackB_read(&r,30)!=0 || oggpackB_read(&r,16)!=0){
      fprintf(stderr,"failed 2; got -1 prematurely.\n");
      exit(1);
  }

  if(oggpackB_look(&r,18)!=0 ||
     oggpackB_look(&r,18)!=0){
    fprintf(stderr,"failed 3; got -1 prematurely.\n");
      exit(1);
  }
  if(oggpackB_look(&r,19)!=-1 ||
     oggpackB_look(&r,19)!=-1){
    fprintf(stderr,"failed; read past end without -1.\n");
      exit(1);
  }
  if(oggpackB_look(&r,32)!=-1 ||
     oggpackB_look(&r,32)!=-1){
    fprintf(stderr,"failed; read past end without -1.\n");
      exit(1);
  }
  fprintf(stderr,"ok.");
  oggpackB_writeclear(&o);

  /* this is partly glassbox; we're mostly concerned about the allocation boundaries */

  fprintf(stderr,"\nTesting aligned writecopies (MSb): ");
  for(i=0;i<71;i++)
    for(j=0;j<5;j++)
      copytestB(j*8,i);
  for(i=BUFFER_INCREMENT*8-71;i<BUFFER_INCREMENT*8+71;i++)
    for(j=0;j<5;j++)
      copytestB(j*8,i);
  fprintf(stderr,"ok.      ");

  fprintf(stderr,"\nTesting unaligned writecopies (MSb): ");
  for(i=0;i<71;i++)
    for(j=1;j<40;j++)
      if(j&0x7)
        copytestB(j,i);
  for(i=BUFFER_INCREMENT*8-71;i<BUFFER_INCREMENT*8+71;i++)
    for(j=1;j<40;j++)
      if(j&0x7)
        copytestB(j,i);
  
  fprintf(stderr,"ok.      \n\n");

  return(0);
}
#endif  /* _V_SELFTEST */

#undef BUFFER_INCREMENT
