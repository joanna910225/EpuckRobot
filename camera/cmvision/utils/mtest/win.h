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

#ifndef __WIN_H__
#define __WIN_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <stdio.h>

#ifndef RGB_STRUCT
#define RGB_STRUCT
struct rgb{
  unsigned char red,green,blue;
};
#endif

//==== Graphics/Display Routines =====================================//

class xwindows{
  friend class xdrawable;
  friend class xpixmap;
  friend class xwin;

  Display *disp;
  int screen;
  int depth;
  Window root;
  Visual *vis;

  Window win;
  GC gc;

  int red_shift,green_shift,blue_shift;

public:
  bool initialize();
  void close();
  xwin createWindow(int width,int height,char *title);
  // xpixmap createPixmap(int width,int height);

  bool checkEvent(XEvent &xev);
  void getEvent(XEvent &xev);
  void flush();
};

class xdrawable{
protected:
  xwindows *xw;
  union{
    Drawable draw;
    Window win;
    Pixmap pix;
  };
  Colormap cmap;
  GC gc;
public:
  void setColor(rgb color);
  void setColor(int red,int green,int blue);
  void setGray(int gray);

  void fillRectangle(int x,int y,int w,int h);
  void fillRectangle(XRectangle r);
  void fillCircle(int x,int y,int r);
  void fillPolygon(XPoint *pts,int num);

  void drawRectangle(int x,int y,int w,int h);
  void drawRectangle(XRectangle r);
  void drawCircle(int x,int y,int r);
  void drawLine(int x1,int y1,int x2,int y2);
  void drawLines(XPoint *pts,int num);

  void copyArea(xdrawable &src,int src_x,int src_y,
		int w,int h,int dest_x,int dest_y);
  void print(int x,int y,char *str);

  bool loadImage(char *filename);

  void flush()
    {xw->flush();}
};

class xpixmap : public xdrawable{
  friend class xwin;
public:
  void initialize(xwin *nxw,int width,int height);
  void close();
};

class xwin : public xdrawable{
  friend class xpixmap;
public:
  void initialize(xwindows *nxw,int width,int height,const char *title);
  void close();

  void setName(const char *name)
    {XStoreName(xw->disp,win,name);}
  void clearArea(int x,int y,int w,int h,bool exposures)
    {XClearArea(xw->disp,win,x,y,w,h,exposures);}

  xpixmap createPixmap(int width,int height);
  void setBackground(xpixmap &p);

  /*
  XImage *newImage(int w,int h);
  void deleteImage(XImage *img)
    {XDestroyImage(img);}
  void putImage(XImage *img,int x,int y)
    {XPutImage(xw->disp,win,gc,img,0,0,x,y,img->width,img->height);}
  */
};

#endif
// __WIN_H__
