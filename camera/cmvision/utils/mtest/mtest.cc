// A simple program demonstrating X output and use of density merging.

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "cmvision.h"
#include "capture.h"
#include "win.h"

#define WIDTH  160
#define HEIGHT 120

int main()
{
  xwindows xw;
  xwin win;
  xpixmap back;

  XEvent xev;
  KeySym key;

  capture cap;
  CMVision vision;
  image_pixel *img;

  CMVision::region *reg;
  CMVision::rectangle rect;
  CMVision::line line;

  int c;
  bool run;

  xw.initialize();
  win = xw.createWindow(WIDTH,HEIGHT,"Density Merging Test");
  back = win.createPixmap(WIDTH,HEIGHT);

  if(!cap.initialize(WIDTH,HEIGHT) ||
     !vision.initialize(WIDTH,HEIGHT)){
    printf("Vision init failed.\n");
  }

  vision.loadOptions("colors.txt");
  vision.enable(CMV_DENSITY_MERGE);

  run = true;

  while(run){
    img = (image_pixel*)cap.captureFrame();
    vision.processFrame(img);

    back.setGray(0);
    back.fillRectangle(0,0,WIDTH,HEIGHT);

    for(c=0; c<4; c++){
      reg = vision.getRegions(c);
      back.setColor(vision.getColorVisual(c));

      while(reg && reg->area>16){
	rect.x = reg->x1;
	rect.y = reg->y1;
	rect.w = reg->x2 - reg->x1 + 1;
	rect.h = reg->y2 - reg->y1 + 1;

	back.drawRectangle(rect.x,rect.y,rect.w,rect.h);

	reg = reg->next;
      }
    }

    // copy back buffer to front
    win.copyArea(back,0,0,WIDTH,HEIGHT,0,0);

    while(xw.checkEvent(xev)){
      switch(xev.type){
        case KeyPress:
	  key = XLookupKeysym(&xev.xkey,0);

	  switch(key){
            case XK_space: break;

            case(XK_Left):  break;
            case(XK_Up):    break;
            case(XK_Right): break;
            case(XK_Down):  break;

            case(XK_Escape):
	      run = false;
	      break;
	  }
	  break;
      }
    }
  }

  cap.close();
  vision.close();

  win.close();
  xw.close();
}
