#include "thresh.h"

inline int min(int a,int b)
{
  return((a < b)? a : b);
}

inline int max(int a,int b)
{
  return((a > b)? a : b);
}

int bound(int low,int high,int n)
{
  if(n < low ) n = low;
  if(n > high) n = high;
  return(n);
}

/*====================================================================
    Main program
  ====================================================================*/

windows win;
appstate app;
bool capture_frame;
bool captured_image = false;
bool white_background; 
extern bool button_press;
extern int mouse_x;
extern int mouse_y;

typedef void*(*pthread_start_routine)(void *);
bool run_frame_daemon;

class capture cap;
class CMVision vision;
int frame = 0;
int uframe = 0;

int permute[HIST_SIZE];
int rx,ry;
image_pixel *c_img;

void vision_draw_crosshairs(int color, int val);

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

#define SET_HIST_X(A,I,J,Y,U,V) \
  A[(I)*256 + (J)] = A[(I)*256 + (J) + 1] = yuv_to_rgb(Y,U,V)
#define SET_HIST_Y(A,I,J,Y,U,V) \
  A[(I)*256 + (J)] = A[((I)+1)*256 + (J)] = yuv_to_rgb(Y,U,V)

void init_histograms()
{
  int i,t,r;

  /*memset(app.uv_hist,0,HIST_SIZE*HIST_SIZE*4);
  memset(app.uy_hist,0,HIST_SIZE*HIST_SIZE*4);
  memset(app.yv_hist,0,HIST_SIZE*HIST_SIZE*4);*/

  memset(app.uv_hist,0,HIST_SIZE*HIST_SIZE*3); // set background color of histograms here
  memset(app.uy_hist,0,HIST_SIZE*HIST_SIZE*3); 
  memset(app.yv_hist,0,HIST_SIZE*HIST_SIZE*3);

  for(i=1; i<HIST_SIZE-1; i++){
    SET_HIST_Y(app.uv_hist, 1,i, 128,0,i);
    SET_HIST_X(app.uv_hist, i,1, 128,i,0);
    SET_HIST_Y(app.uv_hist, 253,i, 128,255,i);
    SET_HIST_X(app.uv_hist, i,253, 128,i,255);

    SET_HIST_Y(app.uy_hist, 1,i, i,0,128);
    SET_HIST_X(app.uy_hist, i,1, 0,i,128);
    SET_HIST_Y(app.uy_hist, 253,i, i,255,128);
    SET_HIST_X(app.uy_hist, i,253, 255,i,128);

    SET_HIST_Y(app.yv_hist, 1,i, 0,128,i);
    SET_HIST_X(app.yv_hist, i,1, i,128,0);
    SET_HIST_Y(app.yv_hist, 253,i, 255,128,i);
    SET_HIST_X(app.yv_hist, i,253, i,128,255);
  }

  // Generate randomly ordered index array
  for(i=0; i<HIST_SIZE-6; i++) permute[i] = i;

  for(i=0; i<HIST_SIZE-7; i++){
    r = rand() % (HIST_SIZE - 6 - i);
    t = permute[r];
    permute[r] = permute[i];
    permute[i] = t;
  }
}

void add_to_hist(image_pixel *src,int w,int h)
{
  int i,j;
  int l,x  ;
  int y,u,v;
  rgb c;
  image_pixel p;

  //c.alpha = 0;

  for(i=0; i<w*h/2; i++){
    p = src[i];

    y = (p.y1 + p.y2) / 2;
    v = 2*p.u - 256;
    u = 2*p.v - 256;

    c.red   = bound(0,255,y + u);
    c.green = bound(0,255,(int)(y - 0.51*u - 0.19*v));
    c.blue  = bound(0,255,y + v);
    // c.alpha = 0;

    app.uv_hist[256*p.u + p.v] = c;
    app.uy_hist[256*p.u +   y] = c;
    app.yv_hist[256*  y + p.v] = c;
  }

  // set background pixels here
  if(white_background) // make background white if user requests it
    c.red = c.green = c.blue = 255;
  else
    c.red = c.green = c.blue = 0;

  // erase a pseudo-random set of pixels.
  for(i=0; i<4; i++){
    for(j=0; j<HIST_SIZE-6; j++){
      rx = (rx + 1) % (HIST_SIZE - 6);
      ry = (ry + HIST_SIZE - 7) % (HIST_SIZE - 6);

      x = permute[rx] + 3;
      y = permute[ry] + 3;

      l = y*HIST_SIZE + x;
      app.uv_hist[l] = c;
      app.uy_hist[l] = c;
      app.yv_hist[l] = c;
    }
    ry++; // skip to next row sequence
  }

  // prevent threshold boxes from erasure
  if(white_background){
    vision_draw_thresholds(app.color,0);
  }else{
    vision_draw_thresholds(app.color,255);
  }
}

void draw_hist_rect(rgb *img,int x1,int y1,int x2,int y2,rgb c)
{
  int x,y;

  x1 = bound(4,HIST_SIZE-4,x1);
  y1 = bound(4,HIST_SIZE-4,y1);
  x2 = bound(4,HIST_SIZE-4,x2);
  y2 = bound(4,HIST_SIZE-4,y2);

  for(x=x1; x<x2; x++){
    img[HIST_SIZE*y1 + x] = c;
    img[HIST_SIZE*y2 + x] = c;
  }

  for(y=y1; y<y2; y++){
    img[HIST_SIZE*y + x1] = c;
    img[HIST_SIZE*y + x2] = c;
  }
}

rgb yuv_to_rgb(image_pixel p)
{
  int y,u,v;
  rgb c;

  y = (p.y1 + p.y2) / 2;
  v = 2*p.u - 256;
  u = 2*p.v - 256;

  c.red   = bound(0,255,y + u);
  c.green = bound(0,255,(int)(y - 0.51*u - 0.19*v));
  c.blue  = bound(0,255,y + v);

  return(c);
}


unsigned int matrix_coefficients = 6;
const signed int Inverse_Table_6_9[8][4] =
{
    {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
    {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
    {104597, 132201, 25675, 53279}, /* unspecified */
    {104597, 132201, 25675, 53279}, /* reserved */
    {104448, 132798, 24759, 53109}, /* FCC */
    {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
    {104597, 132201, 25675, 53279}, /* SMPTE 170M */
    {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
};


unsigned char * table_rV[256];
unsigned char * table_gU[256];
int table_gV[256];
unsigned char * table_bU[256];

static int div_round (int dividend, int divisor){
  if (dividend > 0)
    return (dividend + (divisor>>1)) / divisor;
  else
    return -((-dividend + (divisor>>1)) / divisor);
}

static void yuv_to_rgb_init()
{
  int i;
  unsigned char table_Y[1024];
  unsigned char  *table_8 = 0;
  unsigned int entry_size = 0;
  unsigned char  *table_r = 0, *table_g = 0, *table_b = 0;

  int crv = Inverse_Table_6_9[matrix_coefficients][0];
  int cbu = Inverse_Table_6_9[matrix_coefficients][1];
  int cgu = -Inverse_Table_6_9[matrix_coefficients][2];
  int cgv = -Inverse_Table_6_9[matrix_coefficients][3];

  for (i = 0; i < 1024; i++){
    int j;
    j = (76309 * (i - 384 - 16) + 32768) >> 16;
    j = (j < 0) ? 0 : ((j > 255) ? 255 : j);
    table_Y[i] = j;
  }

  table_8 = (unsigned char*) malloc ((256 + 2*232) * sizeof (unsigned char));

  entry_size = sizeof (unsigned char);
  table_r = table_g = table_b = table_8 + 232;

  for (i = -232; i < 256+232; i++)
    ((unsigned char * )table_b)[i] = table_Y[i+384];

  for (i = 0; i < 256; i++){
    table_rV[i] = table_r + entry_size * div_round (crv * (i-128), 76309);
    table_gU[i] = table_g + entry_size * div_round (cgu * (i-128), 76309);
    table_gV[i] = entry_size * div_round (cgv * (i-128), 76309);
    table_bU[i] = table_b + entry_size * div_round (cbu * (i-128), 76309);
  }
}


void yuv_to_rgb(rgb *dest , image_pixel *src, int w,int h)
{
  int i,s;
  int U, V, Y;
  image_pixel p;
  rgb c;
  unsigned char * r, * g, * b;
  s = w * h;
  for(i=0; i<s; i++){
    p = src[i / 2];

    U = p.u;				
    V = p.v;

    r = table_rV[V];
    g = table_gU[U] + table_gV[V];
    b = table_bU[U];

    Y = p.y1;
    c.red = r[Y];
    c.green = g[Y];
    c.blue = b[Y];
    dest[i] = c;

    Y = p.y2;
    c.red = r[Y];
    c.green = g[Y];
    c.blue = b[Y];
    dest[i+1] = c;
  }
}


void frame_daemon(void *data)
{
  while(run_frame_daemon){
    if(capture_frame){
      c_img = (image_pixel*)cap.captureFrame();
      captured_image = true;
      frame++;
    }

    if(app.show_hist /* && sem_trywait(&app.hist_lock)!=EAGAIN*/){
    	sem_wait(&app.hist_lock);
      add_to_hist(c_img,VIEW_WIDTH,VIEW_HEIGHT);
  		sem_post(&app.hist_lock);
    }

    yuv_to_rgb((rgb*)app.video_buf,c_img,VIEW_WIDTH,VIEW_HEIGHT);
    vision.testClassify((rgb*)app.output_buf,c_img);

    //memcpy(app.video_buf,img,320*240*4);

    /*
    printf(".");
    if(frame%30 == 0) printf("\n");
    fflush(stdout);
    */

    // update_windows(win);
  }
}

int update_daemon(void *data)
{
  uframe++;
  update_windows(win,uframe);
  return(run_frame_daemon);
}

bool vision_load(char *filename)
{
  return(vision.loadOptions(filename));
}

bool vision_save(char *filename)
{
  return(vision.saveOptions(filename));
}

#define THRESH_VAL(T) \
  (int)(GTK_ADJUSTMENT(win.control.T.adj)->value)
#define THRESH_SLIDER(T) \
  gtk_adjustment_set_value(GTK_ADJUSTMENT(win.control.T.adj),T)


void vision_draw_thresholds(int color,int val)
{
  rgb c;

  int y_min,y_max;
  int u_min,u_max;
  int v_min,v_max;

  vision.getThreshold(app.color,
    y_min,y_max,
    u_min,u_max,
    v_min,v_max);

  c.red = c.green = c.blue = val; //c.alpha = val;
  draw_hist_rect(app.uv_hist,v_min,u_min,v_max,u_max,c);
  draw_hist_rect(app.uy_hist,y_min,u_min,y_max,u_max,c);
  draw_hist_rect(app.yv_hist,v_min,y_min,v_max,y_max,c);
}

void vision_set_thresholds(int color)
{
  int y_min,y_max;
  int u_min,u_max;
  int v_min,v_max;

  // erase old thresholds
  if(white_background){
    vision_draw_thresholds(app.color,255);
  }else{
    vision_draw_thresholds(app.color,0);
  }

  // set new thresholds
  y_min = THRESH_VAL(y_min); y_max = THRESH_VAL(y_max);
  u_min = THRESH_VAL(u_min); u_max = THRESH_VAL(u_max);
  v_min = THRESH_VAL(v_min); v_max = THRESH_VAL(v_max);

  vision.setThreshold(color,
    y_min,y_max,
    u_min,u_max,
    v_min,v_max);

  // draw new thresholds
  if(white_background){
    vision_draw_thresholds(color,0);
  }else{
    vision_draw_thresholds(color,255);
  }
}

void vision_get_thresholds(int color)
{
  int y_min,y_max;
  int u_min,u_max;
  int v_min,v_max;

  vision.getThreshold(app.color,
    y_min,y_max,
    u_min,u_max,
    v_min,v_max);

  THRESH_SLIDER(y_min);  THRESH_SLIDER(y_max);
  THRESH_SLIDER(u_min);  THRESH_SLIDER(u_max);
  THRESH_SLIDER(v_min);  THRESH_SLIDER(v_max);

  // draw new thresholds
  if(white_background)
    vision_draw_thresholds(app.color,0);
  else
    vision_draw_thresholds(app.color,255);

}

char *vision_color_name(int color)
{
  return(vision.getColorName(color));
}

int main(int argc,char **argv)
{
  pthread_t thr;
  guint timer;

  app.video_buf = new unsigned char[VIEW_WIDTH * VIEW_HEIGHT * 4];
  app.output_buf = new unsigned char[VIEW_WIDTH * VIEW_HEIGHT * 4];

  app.uv_hist = new rgb[256 * 256];
  app.uy_hist = new rgb[256 * 256];
  app.yv_hist = new rgb[256 * 256];

  app.cross_x = 5;
  app.cross_y = 5;
  strcpy(app.filename,"colors.txt");

  // initialize capture
  if(!cap.initialize("/dev/video",VIEW_WIDTH,VIEW_HEIGHT,
		     DEFAULT_VIDEO_FORMAT)){
    printf("Capture initialization failed!\n");
    exit(1);
  }
  if(!vision.initialize(VIEW_WIDTH,VIEW_HEIGHT)
     || !vision.loadOptions(app.filename)){
    printf("Vision initialization failed!\n");
    exit(2);
  }

  init_histograms();

  // initialize gtk
  //g_thread_init(NULL);

  gtk_init(&argc,&argv);
  gdk_rgb_init();
  create_windows(win);

  // initialize partial look yuv to rgb routine
  yuv_to_rgb_init();

  // spawn refresh thread
  run_frame_daemon = true;
  capture_frame = true;
  sem_init(&app.hist_lock,0,1);
  pthread_create(&thr,NULL,(pthread_start_routine)frame_daemon,NULL);
  timer = gtk_timeout_add(1000/5,(GtkFunction)update_daemon,NULL);

  // black threshold background by default
  white_background = false;
  app.show_hist = true;

  // Main Function
  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();

  // shutwown
  run_frame_daemon = false;
  pthread_join(thr,NULL);
  sem_destroy(&app.hist_lock);

  cap.close();
  vision.close();

  return(0);
}
