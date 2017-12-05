/*=========================================================================
    Win.h
  -------------------------------------------------------------------------
    Simple C++ wrapper interface for XLib
  -------------------------------------------------------------------------
    Copyright 1999, 2000 James R. Bruce
    School of Computer Science, Carnegie Mellon University
  -------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  =========================================================================*/

#include "win.h"

int LeastSetBitNum(int n)
{
  int i = 1;

  if(!n) return(0);

  while(!(n&1)){
    n >>= 1;
    i++;
  }
  return(i);
}

//==== XWindows Class Implementation =================================//

bool xwindows::initialize()
{
  // General initialization of X
  disp = XOpenDisplay(NULL);
  if(!disp){
    printf("Cannot open display.\n");
    return(false);
  }

  screen = DefaultScreen(disp);
  depth = DefaultDepth(disp,screen);
  root = RootWindow(disp,screen);

  XVisualInfo xvi;
  if(!XMatchVisualInfo(disp,screen,24,TrueColor,&xvi) &&
     !XMatchVisualInfo(disp,screen,32,TrueColor,&xvi)){
    printf("This program requires 24 bit color (%d).\n",depth);
    exit(0);
  }
  red_shift   = LeastSetBitNum(xvi.red_mask  ) - 1;
  green_shift = LeastSetBitNum(xvi.green_mask) - 1;
  blue_shift  = LeastSetBitNum(xvi.blue_mask ) - 1;
  vis = xvi.visual;
  // printf("Shift: %d %d %d\n",red_shift,green_shift,blue_shift);

  return(true);
}

void xwindows::close()
{
  if(disp) XCloseDisplay(disp);
  disp = NULL;
}

xwin xwindows::createWindow(int width,int height,char *title)
{
  xwin xw;
  xw.initialize(this,width,height,title);
  return(xw);
}

/*
xpixmap xwindows::createPixmap(int width,int height)
{
  xpixmap xp;
  xp.initialize(this,width,height);
  return(xp);
}
*/

bool xwindows::checkEvent(XEvent &xev)
{
  bool r = (XPending(disp) != 0);
  if(r) XNextEvent(disp,&xev);
  return(r);
}

void xwindows::getEvent(XEvent &xev)
{
  XNextEvent(disp,&xev);
}

void xwindows::flush()
{
  XFlush(disp);
}

//==== XDrawable Class Implementation ================================//

void xdrawable::setColor(int red,int green,int blue)
{
  int id;

  id = (red   << xw->red_shift  ) |
       (green << xw->green_shift) |
       (blue  << xw->blue_shift );

  XSetForeground(xw->disp,gc,id);
}

void xdrawable::setColor(rgb color)
{
  int id;

  id = (color.red   << xw->red_shift  ) |
       (color.green << xw->green_shift) |
       (color.blue  << xw->blue_shift );

  XSetForeground(xw->disp,gc,id);
}

void xdrawable::setGray(int intensity)
{
  int id;

  id = (intensity << xw->red_shift  ) |
       (intensity << xw->green_shift) |
       (intensity << xw->blue_shift );

  XSetForeground(xw->disp,gc,id);
}

void xdrawable::fillRectangle(int x,int y,int w,int h)
{
  XFillRectangle(xw->disp,draw,gc,x,y,w,h);
}

void xdrawable::fillCircle(int x,int y,int r)
{
  XFillArc(xw->disp,draw,gc,x-r,y-r,2*r+1,2*r+1,0,360*64);
}

void xdrawable::fillPolygon(XPoint *pts,int num)
{
  XFillPolygon(xw->disp,draw,gc,pts,num,Convex,CoordModeOrigin);
}


void xdrawable::drawRectangle(int x,int y,int w,int h)
{
  XDrawRectangle(xw->disp,draw,gc,x,y,w,h);
}

void xdrawable::drawCircle(int x,int y,int r)
{
  XDrawArc(xw->disp,draw,gc,x-r,y-r,2*r+1,2*r+1,0,360*64);
}

void xdrawable::drawLine(int x1,int y1,int x2,int y2)
{
  XDrawLine(xw->disp,draw,gc,x1,y1,x2,y2);
}

void xdrawable::drawLines(XPoint *pts,int num)
{
  XDrawLines(xw->disp,draw,gc,pts,num,CoordModeOrigin);      
}

void xdrawable::copyArea(xdrawable &src,int src_x,int src_y,
			int w,int h,int dest_x,int dest_y)
{
  XCopyArea(xw->disp,src.draw,draw,gc,src_x,src_y,w,h,dest_x,dest_y);
}

void xdrawable::print(int x,int y,char *str)
{
  XTextItem t;
  int i;

  t.chars  = str;
  t.nchars = strlen(str);
  t.delta  = 0;
  t.font   = None;

  XDrawText(xw->disp,draw,gc,x,y,&t,1);
}

bool xdrawable::loadImage(char *filename)
{
  FILE *in;
  int w,h,m,x,y;
  rgb c;

  in = fopen(filename,"rb");
  if(!in) return(false);

  w = h = 0;
  fscanf(in,"P6\n%d %d\n%d\n",&w,&h,&m);
  // printf("w=%d h=%d max=%d\n",w,h,m);

  for(y=0; y<h; y++){
    printf("\rloading '%s' %2d%%",filename,(y*100)/h);
    fflush(stdout);
    for(x=0; x<w; x++){
      fread(&c,sizeof(rgb),1,in);
      setColor(c);
      XDrawPoint(xw->disp,draw,gc,x,y);
    }
  }
  fclose(in);
  printf("\rloading '%s' done.\n",filename);

  return(true);
}

/*
void printXImage(XImage *i)
{
  printf("width: %d\n",i->width);
  printf("height: %d\n",i->height);
  printf("xoffset: %d\n",i->xoffset);
  printf("format: %d\n",i->format);
  printf("data: 0x%X\n",i->data);
  printf("byte_order: %d\n",i->byte_order);
  printf("bitmap_unit: %d\n",i->bitmap_unit);
  printf("bitmap_bit_order: %d\n",i->bitmap_bit_order);
  printf("bitmap_pad: %d\n",i->bitmap_pad);
  printf("depth: %d\n",i->depth);
  printf("bytes_per_line: %d\n",i->bytes_per_line);
  printf("bits_per_pixel: %d\n",i->bits_per_pixel);
  printf("red_mask: 0x%08X\n",i->red_mask);
  printf("green_mask: 0x%08X\n",i->green_mask);
  printf("blue_mask: 0x%08X\n",i->blue_mask);
}

XImage *xdrawable::newImage(int w,int h)
{
  XImage *img;
  char *data;

  data = new char[w * h * 3];
  if(!data) return(NULL);

  img = new XImage;
  if(!img){
    delete(data);
    return(NULL);
  }

  img->width  = w;
  img->height = h;
  img->xoffset = 0;
  img->format = ZPixmap;
  img->data = data;
  img->byte_order = MSBFirst;
  img->bitmap_unit = 8;
  img->bitmap_bit_order = LSBFirst;
  img->bitmap_pad = 8;
  img->depth = 24;
  img->bytes_per_line = w * (24/8);
  img->bits_per_pixel = 24;
  img->red_mask   = 0xFF <<  0;
  img->green_mask = 0xFF <<  8;
  img->blue_mask  = 0xFF << 16;
  // there, wasn't that easy?
  XInitImage(img);

  // printf("Status = %d\n",XInitImage(img));
  // img = XGetImage(disp,root,0,0,width,height,AllPlanes,ZPixmap);
  // img = XCreateImage(disp,vis,24,ZPixmap,0,data,w,h,8,0);

  return(img);
}
*/

//==== XPixmap Class Implementation ==================================//

void xpixmap::initialize(xwin *nxw,int w,int h)
{
  xw = nxw->xw;

  // Create and initialize pixmap
  unsigned valuemask = 0;
  XGCValues values;

  pix = XCreatePixmap(xw->disp,xw->root,w,h,24);

  cmap = nxw->cmap; // XCreateColormap(xw->disp,win,xw->vis,AllocNone);
  gc = XCreateGC(xw->disp,nxw->win,valuemask,&values);
  // XCopyGC(xw->disp, src, valuemask, dest)nxw->gc);
}

void xpixmap::close()
{
  XFreePixmap(xw->disp,pix);
}

//==== XWin Class Implementation =====================================//

void xwin::initialize(xwindows *nxw,int width,int height,const char *title)
{
  xw = nxw;

  // Create and initialize window
  XSizeHints size_hints;
  unsigned valuemask = 0;
  XGCValues values;
  XEvent xev;

  win = XCreateSimpleWindow(xw->disp,xw->root,0,0,width,height,0,
    WhitePixel(xw->disp,xw->screen), BlackPixel(xw->disp,xw->screen));

  cmap = XCreateColormap(xw->disp,win,xw->vis,AllocNone);
  gc = XCreateGC(xw->disp,win,valuemask,&values);

  size_hints.flags = PSize | PMinSize | PMaxSize;
  size_hints.min_width = width;
  size_hints.max_width = width;
  size_hints.min_height = height;
  size_hints.max_height = height;
  XSetStandardProperties(xw->disp,win,title,title,None,0,0,&size_hints);

  XSelectInput(xw->disp,win,
    ButtonPressMask |
    ButtonMotionMask |
    Button1MotionMask | Button2MotionMask | Button3MotionMask |
    PointerMotionMask |
    KeyPressMask | KeyReleaseMask |
    ExposureMask);
  XMapWindow(xw->disp,win);

  // Block until window is mapped
  do{
    xw->getEvent(xev);
  }while(xev.type != Expose);
}

void xwin::close()
{
  XDestroyWindow(xw->disp,win);
}

xpixmap xwin::createPixmap(int width,int height)
{
  xpixmap xp;
  xp.initialize(this,width,height);
  return(xp);
}

void xwin::setBackground(xpixmap &p)
{
  XSetWindowBackgroundPixmap(xw->disp,win,p.pix);
}

//==== Event Handling ====//

/*
bool xwin::checkEvent(XEvent &xev)
{
  bool r = (XPending(disp) != 0);
  if(r) XNextEvent(disp,&xev);
  return(r);
}

void xwin::getEvent(XEvent &xev)
{
  XNextEvent(disp,&xev);
}

void xwin::flush()
{
  XFlush(disp);
}
*/
