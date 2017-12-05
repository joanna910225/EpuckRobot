/*=========================================================================
    Copyright 2000 James Bruce
    Example program to capture and save images using the capture library
  -------------------------------------------------------------------------
    REVISION HISTORY:
      2000-05-20:  Initial release version - ugly, adapted from
                   something I needed really quickly at some point (JRB)
  =========================================================================*/

// NOTE: this file is messy and requires modification to be useful,
// though hopefully it can serve as example code for using the capture
// library.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// CMVision capture library
#include <capture.h>

// The printf format string for output filenames
#define FILE_NAME_STYLE "images/image%03d.ppm"


struct rgb{
  unsigned char red,green,blue;
};

struct uyvy{
  unsigned char u,y1,v,y2;
};

rgb black = {0,0,0};


// These should really be parameters, but aren't yet.
int width  = 320;
int height = 240;
int fmt = V4L2_PIX_FMT_UYVY;
// int fmt = V4L2_PIX_FMT_BGR24;
// int fmt = V4L2_PIX_FMT_YUYV;
int num = 120;

rgb *img;
uyvy *buf,*cur;
char file[32];


int bound(int low,int high,int n)
{
  if(n < low ) n = low;
  if(n > high) n = high;
  return(n);
}

rgb yuv_to_rgb(int y,int u,int v)
{
  rgb c;

  u = 2*u - 256;
  v = 2*v - 256;

  c.red   = bound(0,255,y + v);
  c.green = bound(0,255,(int)(y - 0.51*v - 0.19*u));
  c.blue  = bound(0,255,y + u);
  //c.alpha = 0;

  return(c);
}

void swap_rgb(rgb *buf,int len)
{
  int i;
  rgb r;

  for(i=0; i<len; i++){
    r = buf[i];
    buf[i].red  = r.blue;
    buf[i].blue = r.red;
  }
}

void convert(rgb *dest,uyvy *src,int size)
{
  int i;
  uyvy p;

  for(i=0; i<size; i++){
    p = src[i];
    dest[2*i + 0] = yuv_to_rgb(p.y1,p.u,p.v);
    dest[2*i + 1] = yuv_to_rgb(p.y2,p.u,p.v);
  }
}

int main()
{
  capture cap;
  int i,size;

  char filename[32];
  FILE *file;

  timeval t1,t2;
  double sec;
  bool ok;

  ok = cap.initialize("/dev/video",width,height,fmt);
  if(!ok) printf("Capture init failed.\n");

  size = width*height/2;
  img = new rgb[size*2];
  buf = new uyvy[size*num];

  if(!img || !buf){
    printf("Not enough memory");
    exit(1);
  }

  // Initialize mem to bring into core memory
  // Could use mlock, but its the simple 1-thread method for now
  memset(buf,0,size*num*sizeof(uyvy));

  /*
  // Wait a bit
  printf("Waiting four seconds"); fflush(stdout);
  for(i=0; i<4; i++){
    sleep(1);
    printf("."); fflush(stdout);
  }
  printf("\n");
  */

  // Capture some initial frames
  for(i=0; i<10; i++){
    cur = (uyvy*)cap.captureFrame();
  }

  // Capture the frames
  printf("Capturing\n");
  gettimeofday(&t1,NULL);

  for(i=0; i<num; i++){
    cur = (uyvy*)cap.captureFrame();
    memcpy(buf + size*i,cur,size*sizeof(uyvy));
    // memset(buf + size*i,i*4,size*sizeof(rgb));

    printf("."); fflush(stdout);
    if(i % 30 == 0) printf("\n");
  }

  gettimeofday(&t2,NULL);
  printf("done.\n");

  sec = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1E6;
  printf("Elapsed time: %4.3fs (%ffps)\n",sec,num / sec);

  // Most capture cards are BGR for some reason
  // swap_rgb(buf,size*num);

  // Save files to image dir as raw PPMs
  printf("Writing\n");
  for(i=0; i<num; i++){
    convert(img,buf + size*i,size);

    sprintf(filename,FILE_NAME_STYLE,i);
    if(file = fopen(filename,"w")){
      fprintf(file,"P6\n%d %d\n255\n",width,height);
      fwrite(img,sizeof(rgb),size*2,file);
      fclose(file);
      printf("."); fflush(stdout);
    }
  }
  printf("done.\n");

  return(0);
}
