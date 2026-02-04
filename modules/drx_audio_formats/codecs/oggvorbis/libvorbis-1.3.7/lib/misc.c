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
 ********************************************************************/

#define HEAD_ALIGN 32
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vorbis/codec.h"
#define MISC_C
#include "misc.h"
#include <sys/time.h>

static pthread_mutex_t memlock=PTHREAD_MUTEX_INITIALIZER;
static uk *pointers=NULL;
static i64 *insertlist=NULL; /* We can't embed this in the pointer list;
                          a pointer can have any value... */

static t8 **files=NULL;
static i64 *file_bytes=NULL;
static i32  filecount=0;

static i32 ptop=0;
static i32 palloced=0;
static i32 pinsert=0;

typedef struct {
  t8 *file;
  i64 line;
  i64 ptr;
  i64 bytes;
} head;

i64 global_bytes=0;
i64 start_time=-1;

static uk _insert(uk ptr,i64 bytes,t8 *file,i64 line){
  ((head *)ptr)->file=file;
  ((head *)ptr)->line=line;
  ((head *)ptr)->ptr=pinsert;
  ((head *)ptr)->bytes=bytes-HEAD_ALIGN;

  pthread_mutex_lock(&memlock);
  if(pinsert>=palloced){
    palloced+=64;
    if(pointers){
      pointers=(uk *)realloc(pointers,sizeof(uk *)*palloced);
      insertlist=(i64 *)realloc(insertlist,sizeof(i64 *)*palloced);
    }else{
      pointers=(uk *)malloc(sizeof(uk *)*palloced);
      insertlist=(i64 *)malloc(sizeof(i64 *)*palloced);
    }
  }

  pointers[pinsert]=ptr;

  if(pinsert==ptop)
    pinsert=++ptop;
  else
    pinsert=insertlist[pinsert];

#ifdef _VDBG_GRAPHFILE
  {
    FILE *out;
    struct timeval tv;
    static struct timezone tz;
    i32 i;
    t8 buffer[80];
    gettimeofday(&tv,&tz);

    for(i=0;i<filecount;i++)
      if(!strcmp(file,files[i]))break;

    if(i==filecount){
      filecount++;
      if(!files){
        files=malloc(filecount*sizeof(*files));
        file_bytes=malloc(filecount*sizeof(*file_bytes));
      }else{
        files=realloc(files,filecount*sizeof(*files));
        file_bytes=realloc(file_bytes,filecount*sizeof(*file_bytes));
      }
      files[i]=strdup(file);
      file_bytes[i]=0;
    }

    file_bytes[i]+=bytes-HEAD_ALIGN;

    if(start_time==-1)start_time=(tv.tv_sec*1000)+(tv.tv_usec/1000);

    snprintf(buffer,80,"%s%s",file,_VDBG_GRAPHFILE);
    out=fopen(buffer,"a");
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            file_bytes[i]-(bytes-HEAD_ALIGN));
    fprintf(out,"%ld, %ld # FILE %s LINE %ld\n",
            -start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            file_bytes[i],file,line);
    fclose(out);

    out=fopen(_VDBG_GRAPHFILE,"a");
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            global_bytes);
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            global_bytes+(bytes-HEAD_ALIGN));
    fclose(out);
  }
#endif

  global_bytes+=(bytes-HEAD_ALIGN);

  pthread_mutex_unlock(&memlock);
  return(ptr+HEAD_ALIGN);
}

static z0 _ripremove(uk ptr){
  i32 insert;
  pthread_mutex_lock(&memlock);

#ifdef _VDBG_GRAPHFILE
  {
    FILE *out=fopen(_VDBG_GRAPHFILE,"a");
    struct timeval tv;
    static struct timezone tz;
    t8 buffer[80];
    t8 *file =((head *)ptr)->file;
    i64 bytes =((head *)ptr)->bytes;
    i32 i;

    gettimeofday(&tv,&tz);
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            global_bytes);
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            global_bytes-((head *)ptr)->bytes);
    fclose(out);

    for(i=0;i<filecount;i++)
      if(!strcmp(file,files[i]))break;

    snprintf(buffer,80,"%s%s",file,_VDBG_GRAPHFILE);
    out=fopen(buffer,"a");
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            file_bytes[i]);
    fprintf(out,"%ld, %ld\n",-start_time+(tv.tv_sec*1000)+(tv.tv_usec/1000),
            file_bytes[i]-bytes);
    fclose(out);

    file_bytes[i]-=bytes;

  }
#endif

  global_bytes-=((head *)ptr)->bytes;

  insert=((head *)ptr)->ptr;
  insertlist[insert]=pinsert;
  pinsert=insert;

  if(pointers[insert]==NULL){
    fprintf(stderr,"DEBUGGING MALLOC ERROR: freeing previously freed memory\n");
    fprintf(stderr,"\t%s %ld\n",((head *)ptr)->file,((head *)ptr)->line);
  }

  if(global_bytes<0){
    fprintf(stderr,"DEBUGGING MALLOC ERROR: freeing unmalloced memory\n");
  }

  pointers[insert]=NULL;
  pthread_mutex_unlock(&memlock);
}

z0 _VDBG_dump(z0){
  i32 i;
  pthread_mutex_lock(&memlock);
  for(i=0;i<ptop;i++){
    head *ptr=pointers[i];
    if(ptr)
      fprintf(stderr,"unfreed bytes from %s:%ld\n",
              ptr->file,ptr->line);
  }

  pthread_mutex_unlock(&memlock);
}

uk _VDBG_malloc(uk ptr,i64 bytes,t8 *file,i64 line){
  if(bytes<=0)
    fprintf(stderr,"bad malloc request (%ld bytes) from %s:%ld\n",bytes,file,line);

  bytes+=HEAD_ALIGN;
  if(ptr){
    ptr-=HEAD_ALIGN;
    _ripremove(ptr);
    ptr=realloc(ptr,bytes);
  }else{
    ptr=malloc(bytes);
    memset(ptr,0,bytes);
  }
  return _insert(ptr,bytes,file,line);
}

z0 _VDBG_free(uk ptr,t8 *file,i64 line){
  if(ptr){
    ptr-=HEAD_ALIGN;
    _ripremove(ptr);
    free(ptr);
  }
}

