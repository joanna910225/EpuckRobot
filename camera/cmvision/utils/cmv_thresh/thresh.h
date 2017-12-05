#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

// CMVision headers
#include <capture.h>
#include <cmvision.h>



/*====================================================================
    Windowing definitions
  ====================================================================*/

#define VIEW_WIDTH  640
#define VIEW_HEIGHT 480

#define HIST_SIZE 256

struct file_window{
  GtkWidget *file_selector;
};

struct slider{
  GtkWidget *label;
  GtkWidget *scale;
  GtkObject *adj;
};

struct control_window{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *table;
  // GtkWidget *hbox;

  GtkWidget *toolbar;
  GtkWidget *new_button;
  GtkWidget *open_button;
  GtkWidget *reload_button;
  GtkWidget *save_button;
  GtkWidget *saveas_button;

  GtkWidget *color;
  GtkWidget *color_menu;

  slider y_min,y_max;
  slider u_min,u_max;
  slider v_min,v_max;
};

struct vid_window{
  bool can_update;

  GtkWidget *window;

  //GtkWidget *vbox;
  GtkWidget *video;
};

struct output_window{
  bool can_update;
  GtkWidget *window;
  GtkWidget *output; // drawing area
};

struct hist_window{
  bool can_update;

  GtkWidget *window;

  GtkWidget *table;
  GtkWidget *uv_hist;
  GtkWidget *uy_hist;
  GtkWidget *yv_hist;
};

struct windows{
  struct file_window    file;
  struct control_window control;
  struct vid_window     video;
  struct hist_window    hist;
  struct output_window  output;
};

void create_windows(windows &w);
void destroy_windows(windows &w);

/*====================================================================
    File IO definitions
  ====================================================================*/

// bool save_screen(char *filename);

/*====================================================================
    Global data definitions
  ====================================================================*/

#define FILE_MODE_OPEN 1
#define FILE_MODE_SAVE 2

struct appstate{
  int file_mode;
  char filename[256];

  int color;

  unsigned char *video_buf;
  unsigned char *output_buf;

  sem_t hist_lock;
  rgb *uv_hist,*uy_hist,*yv_hist;

  int frame;

  int cross_x;
  int cross_y;

  int framerate;
  bool show_hist;
};

/*====================================================================
    Main functions
  ====================================================================*/

void update_windows(windows &w,int frame);
bool vision_load(char *filename);
bool vision_save(char *filename);
void vision_set_thresholds(int color);
void vision_get_thresholds(int color);
char *vision_color_name(int color);
void vision_draw_thresholds(int color,int val);
