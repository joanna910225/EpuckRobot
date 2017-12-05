#include "thresh.h"

extern appstate app;
extern windows win;
extern bool capture_frame;
extern bool white_background;
extern image_pixel *c_img;
extern bool captured_image;
bool crosshair_mode = false;

int mouse_x = 0;
int mouse_y = 0;
bool button_press = false;

void destroy_windows(windows &w);

void delete_event()
{
  destroy_windows(win);

  gtk_main_quit();
}

//==== File Selection Dialog =========================================//

void file_done(GtkWidget *selector,gpointer data)
{
  // call load_spline_file() / save_current_view()
  char *s;

  s = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  strncpy(app.filename,s,256);
  printf("Got filename:[%s]\n",app.filename);

  gtk_widget_hide(selector);

  switch(app.file_mode){
    case FILE_MODE_OPEN:
      vision_load(app.filename);
      break;
    case FILE_MODE_SAVE:
      vision_save(app.filename);
      break;
  }
}

void file_cancel(GtkWidget *selector,gpointer data)
{
  gtk_widget_destroy(selector);
}

/*
void file_show(GtkWidget *selector)
{
  gtk_widget_show(selector);
}
*/

void create_file_window(file_window &w,char *label)
{    
  w.file_selector = gtk_file_selection_new(label);

  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(w.file_selector)->
				       ok_button),
			    "clicked", GTK_SIGNAL_FUNC(file_done),
			    GTK_OBJECT(w.file_selector));
  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(w.file_selector)->
				       cancel_button),
			    "clicked", GTK_SIGNAL_FUNC(file_cancel),
			    GTK_OBJECT(w.file_selector));

  gtk_widget_show(w.file_selector);
}

//==== Toolbar Events ================================================//

// quit aplication
void toolbar_crosshair_event(GtkWidget *widget, gpointer data)
{
  // switch crosshair mode on and off
  printf("crosshair mode event\n");

  crosshair_mode = !crosshair_mode;
}

void toolbar_quit_event(GtkWidget *widget, gpointer data){
  printf("quit event\n");
  destroy_windows(win);
  gtk_main_quit();  
}

void toolbar_background_event(GtkWidget *widget, gpointer data){
  printf("toggle background event\n");
  if(white_background){
    // set background color of histograms here
    memset(app.uv_hist,0,HIST_SIZE*HIST_SIZE*3);
    memset(app.uy_hist,0,HIST_SIZE*HIST_SIZE*3);
    memset(app.yv_hist,0,HIST_SIZE*HIST_SIZE*3);
    white_background = false;
  }else{
    // set background color of histograms here
    memset(app.uv_hist,255,HIST_SIZE*HIST_SIZE*3); 
    memset(app.uy_hist,255,HIST_SIZE*HIST_SIZE*3);
    memset(app.yv_hist,255,HIST_SIZE*HIST_SIZE*3);
    white_background = true;
  }
}

void toolbar_pause_event(GtkWidget *widget, gpointer data)
{
  printf("pause event\n");
  if(capture_frame){
    capture_frame = false;
  }else{
    capture_frame = true;
  }
}

void toolbar_new_event(GtkWidget *widget,gpointer data)
{
  printf("new event.\n");
  // clear_image();
}

void toolbar_open_event(GtkWidget *widget,gpointer data)
{
  printf("open event.\n");
  app.file_mode = FILE_MODE_OPEN;
  create_file_window(win.file,"Choose a threshold file to open.");
}

void toolbar_reload_event(GtkWidget *widget,gpointer data)
{
  printf("reload event.\n");
  vision_load(app.filename);
}

void toolbar_save_event(GtkWidget *widget,gpointer data)
{
  printf("save event.\n");
  vision_save(app.filename);
}

void toolbar_saveas_event(GtkWidget *widget,gpointer data)
{
  printf("saveas event.\n");
  app.file_mode = FILE_MODE_SAVE;
  create_file_window(win.file,"Choose a file to save thresholds.");
}

//==== Control Window ================================================//

void control_thresh_set_event(GtkWidget *widget,GtkAdjustment *adj)
{
  //printf("set threhold %d.\n",(int)(adj->value));
  vision_set_thresholds(app.color);

  //update_view();
}

void control_select_color_event(GtkWidget *widget,int id)
{
  char *str;
  int i;

  if(id<0 || id>=CMV_MAX_COLORS) return;

  /*
  if(!str) return;

  i = 0;
  while(i<32 && strcmp(str,vision_color_name(i))) i++;
  if(i >= 32) return;
  */

  str = vision_color_name(id);
  printf("set color [%s]:%d.\n",str,id);

  vision_draw_thresholds(app.color,0);
  app.color = id;
  vision_get_thresholds(i);
}

void control_add_hseparator(GtkWidget *table,int &y)
{
  GtkWidget *sep;

  sep = gtk_hseparator_new();

  GtkAttachOptions opfe = (GtkAttachOptions)(GTK_FILL|GTK_EXPAND);
  int pad = 4;
  gtk_table_attach(GTK_TABLE(table),sep,0,2,y,y+1,opfe,opfe,pad,pad);
  y++;

  gtk_widget_show(sep);
}

void control_add_hslider(GtkWidget *table,int &y,slider &s,char *label,double val)
{
  // create widgets
  s.label = gtk_label_new(label);
  s.adj   = gtk_adjustment_new(val,0,256,1,16,1);
  s.scale = gtk_hscale_new(GTK_ADJUSTMENT(s.adj));

  gtk_scale_set_digits(GTK_SCALE(s.scale),0);
  gtk_scale_set_value_pos(GTK_SCALE(s.scale),GTK_POS_TOP);
  // gtk_range_set_update_policy(GTK_RANGE(s.scale),GTK_UPDATE_DELAYED);

  // attach to table
  GtkAttachOptions opfe = (GtkAttachOptions)(GTK_FILL|GTK_EXPAND);
  GtkAttachOptions opf  = (GtkAttachOptions)(GTK_FILL);
  int pad = 4;
  gtk_table_attach(GTK_TABLE(table),s.label,0,1,y,y+1,opf ,opfe,pad,pad);
  gtk_table_attach(GTK_TABLE(table),s.scale,1,2,y,y+1,opfe,opfe,pad,pad);
  y++;

  // Set up signal
  gtk_signal_connect(GTK_OBJECT(s.adj),"value_changed",
      GTK_SIGNAL_FUNC(control_thresh_set_event),(gpointer)s.adj);

  // Show widgets
  gtk_widget_show(s.label);
  gtk_widget_show(s.scale);
}

GtkWidget *control_add_menu(GtkWidget *table,int &y,char *label)
{
  // create widgets
  GtkWidget *lab,*menu,*option_menu;

  lab = gtk_label_new(label);
  option_menu = gtk_option_menu_new();

  // attach to table
  GtkAttachOptions opfe = (GtkAttachOptions)(GTK_FILL|GTK_EXPAND);
  GtkAttachOptions opf  = (GtkAttachOptions)(GTK_FILL);
  int pad = 4;
  gtk_table_attach(GTK_TABLE(table),lab ,0,1,y,y+1,opf ,opfe,pad,pad);
  gtk_table_attach(GTK_TABLE(table),option_menu,1,2,y,y+1,opfe,opfe,pad,pad);
  y++;

  // Show widgets
  gtk_widget_show(lab);
  gtk_widget_show(option_menu);

  return(option_menu);
}

#define LOAD_ICON(str) \
  gdk_pixmap_colormap_create_from_xpm(w.window->window, \
      colormap,&mask,NULL,str)

void create_control_window(control_window &w)
{
  GdkPixmap *icon;
  GdkBitmap *mask;
  GtkWidget *iconw;
  GdkColormap *colormap;

  // Create widgets
  w.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(w.window),"CMV Threshold Editor");
  gtk_container_set_border_width(GTK_CONTAINER(w.window),4);

  w.vbox = gtk_vbox_new(FALSE,0);
  w.table = gtk_table_new(11,2,FALSE);

  // Create toolbar and add icons
  w.toolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,GTK_TOOLBAR_BOTH);
  gtk_box_pack_start(GTK_BOX(w.vbox),w.toolbar,TRUE,TRUE,0);

  colormap = gdk_colormap_get_system();
  icon = LOAD_ICON("img/new.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.new_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "New","Initialize new threshold file",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_new_event),NULL);

  icon = LOAD_ICON("img/open.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.open_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Open","Opens existing threshold file",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_open_event),NULL);

  icon = LOAD_ICON("img/reload.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.reload_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Reload","Reloads current threshold file",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_reload_event),NULL);

  icon = LOAD_ICON("img/save.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.save_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Save","Saves thresholds to current file",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_save_event),NULL);

  icon = LOAD_ICON("img/saveas.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.saveas_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Save As","Saves thresholds to a new file",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_saveas_event),NULL);

  icon = LOAD_ICON("img/pause.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.saveas_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Pause","Pauses CMVision",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_pause_event),NULL);

  icon = LOAD_ICON("img/background.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.saveas_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Background","Toggle Background Color",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_background_event),NULL);

  icon = LOAD_ICON("img/crosshair.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.saveas_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Crosshair","Toggle into crosshair mode",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_crosshair_event),NULL);

  icon = LOAD_ICON("img/quit.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.saveas_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Quit","Quit program",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_quit_event),NULL);

  /*
  gtk_toolbar_append_space(GTK_TOOLBAR(w.toolbar));

  icon = LOAD_ICON("img/paint.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.paint_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Paint","Sets current mode to paint",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_paint_event),NULL);

  icon = LOAD_ICON("img/blur.xpm");
  iconw = gtk_pixmap_new(icon,mask);
  w.blur_button = gtk_toolbar_append_item(GTK_TOOLBAR(w.toolbar),
      "Blur","Sets current mode to blur",NULL,
      iconw,GTK_SIGNAL_FUNC(toolbar_blur_event),NULL);
  */

  GtkWidget *l;
  char *str;
  int i,y;

  w.color_menu = gtk_menu_new();

  for(i=0; i<32; i++){
    str = vision_color_name(i);
    if(str){
      l = gtk_menu_item_new_with_label(str);
      gtk_signal_connect(GTK_OBJECT(l),"activate",
        GTK_SIGNAL_FUNC(control_select_color_event),(gpointer)i);
      gtk_widget_show(l);
      // printf("Added [%s]\n",vision_color_name(i));
      gtk_menu_append(GTK_MENU(w.color_menu),l);
    }
  }

  /*
  // Add color dropdown
  GList *l = w.colors;
  l = g_list_append(l,(void*)"First Item");
  l = g_list_append(l,(void*)"Second Item");
  l = g_list_append(l,(void*)"Third Item");
  l = g_list_append(l,(void*)"Fourth Item");
  l = g_list_append(l,(void*)"Fifth Item");
  w.colors = l;


  w.color = control_add_hslider(w.table,y,"Color:");
  gtk_combo_set_popdown_strings(GTK_COMBO(w.color),w.colors);
  control_add_hseparator(w.table,y);
  */

  y = 0;

  w.color = control_add_menu(w.table,y,"Color:");
  gtk_option_menu_set_menu(GTK_OPTION_MENU(w.color),w.color_menu);
  control_add_hseparator(w.table,y);

  // Add sliders
  control_add_hslider(w.table,y,w.y_min,"Y Min",0);
  control_add_hslider(w.table,y,w.y_max,"Y Max",255);
  control_add_hseparator(w.table,y);

  control_add_hslider(w.table,y,w.u_min,"U Min",0);
  control_add_hslider(w.table,y,w.u_max,"U Max",255);
  control_add_hseparator(w.table,y);

  control_add_hslider(w.table,y,w.v_min,"V Min",0);
  control_add_hslider(w.table,y,w.v_max,"V Max",255);
  control_add_hseparator(w.table,y);

  // set initial values
  control_select_color_event(NULL,0);

  // Pack widgets
  gtk_container_add(GTK_CONTAINER(w.window),w.vbox);
  gtk_box_pack_start(GTK_BOX(w.vbox),w.table,FALSE,FALSE,0);

  // Add destroy-on-close behavior
  gtk_signal_connect(GTK_OBJECT(w.window), "delete_event",
		     GTK_SIGNAL_FUNC(delete_event), NULL);
  gtk_quit_add_destroy(1, GTK_OBJECT(w.window));

  // Set up signals

  // Show widgets
  gtk_widget_show(w.toolbar);

  gtk_widget_show(w.table);
  //gtk_widget_show(w.hbox);
  gtk_widget_show(w.vbox);
  gtk_widget_show(w.window);
}


//==== Video Window ==================================================//

int cal_id = 0;

double cal[13][3] = {
  -137.00,  76.25,   0.00,
     0.00,  76.25,   0.00,
   137.00,  76.25,   0.00,

  -114.50,  50.00,   0.00,
   114.50,  50.00,   0.00,

  -137.00,   0.00,   0.00,
     0.00,   0.00,   0.00,
   137.00,   0.00,   0.00,

  -114.50, -50.00,   0.00,
   114.50, -50.00,   0.00,

  -137.00, -76.25,   0.00,
     0.00, -76.25,   0.00,
   137.00, -76.25,   0.00
};

gboolean crosshair_button_press(GtkWidget *widget,GdkEventButton *event,
				gpointer user_data)
{
  //printf("crosshair button press event (%f, %f)\n", event->x, event->y);
  printf("(%8.2f,%8.2f,%8.2f) (%8.2f,%8.2f)\n",
    cal[cal_id][0],cal[cal_id][1],cal[cal_id][2],
    event->x, event->y);
  cal_id++;

  app.cross_x = (int)event->x;
  app.cross_y = (int)event->y;
  button_press = true;

  return(TRUE);
}

void draw_crosshair(GdkDrawable *output, int x, int y)
{
  GdkGC *gc_output = gdk_gc_new(output);
  GdkColor color;
  //b | (g << 8) | (r << 16);
  color.pixel = 0 | ( 255 << 8) | (0 << 16);
  color.red = 0;
  color.blue = 0;
  color.green = 255;
  gdk_gc_set_foreground(gc_output, &color);
  gdk_gc_set_line_attributes(gc_output, 2, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);
  gdk_draw_line(output,gc_output,x - 2, y, x+2, y);
  gdk_draw_line(output,gc_output,x, y - 2, x, y+2);
  gdk_gc_unref(gc_output);
}

gboolean video_video_expose(GtkWidget *widget,GdkEventExpose *event)
{
  // widget = win.video.video;
  //gdk_gc_set_clip_rectangle(widget->style->fg_gc[widget->state],
  // NULL);
  gdk_draw_rgb_image(widget->window,widget->style->fg_gc[GTK_STATE_NORMAL],
			0,0,VIEW_WIDTH,VIEW_HEIGHT,GDK_RGB_DITHER_NONE,
			(guchar*)app.video_buf,VIEW_WIDTH*3);
  if(crosshair_mode)
    draw_crosshair(widget->window, app.cross_x, app.cross_y);

  return(TRUE);
}

gboolean video_output_expose(GtkWidget *widget,GdkEventExpose *event)
{
  // gdk_gc_set_clip_rectangle(widget->style->fg_gc[widget->state],
  // NULL);
  gdk_draw_rgb_image(widget->window,widget->style->fg_gc[GTK_STATE_NORMAL],
			0,0,VIEW_WIDTH,VIEW_HEIGHT,GDK_RGB_DITHER_NONE,
			(guchar*)app.output_buf,VIEW_WIDTH*3);
  return(TRUE);
}

gboolean video_button_press_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  printf("video_button_press\n");
  if(!capture_frame){
    mouse_x = (int)event->x;
    mouse_y = (int)event->y;
    if(mouse_x < VIEW_WIDTH && mouse_y < VIEW_HEIGHT){
        button_press = true;
    }
  }
}


// void create_output_window()
// create a output window and store information in output_window
void create_output_window(output_window &w){
  // create the window
  w.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(w.window),"CMV Threshold Editor: Output");  
  gtk_widget_set_usize (w.window, VIEW_WIDTH, VIEW_HEIGHT); 
  gtk_window_set_policy (GTK_WINDOW (w.window), FALSE, FALSE, FALSE); // not resizable

  // contstruct output drawing area
  w.output = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(w.output),VIEW_WIDTH,VIEW_HEIGHT);
  gtk_container_add(GTK_CONTAINER(w.window),w.output);
  gtk_signal_connect(GTK_OBJECT(w.window), "delete_event",
		     GTK_SIGNAL_FUNC(delete_event), NULL);

  // don't know
  gtk_quit_add_destroy(1, GTK_OBJECT(w.window));

  // Show widgets
  gtk_widget_show(w.output);

  // show output window
  gtk_widget_show(w.window);

  w.can_update = true;
  gtk_signal_connect(GTK_OBJECT(w.output), "expose_event",
  	     GTK_SIGNAL_FUNC(video_output_expose),NULL);

}

void create_video_window(vid_window &w)
{
  // Create widgets
  w.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_events (w.window, GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  gtk_window_set_title(GTK_WINDOW(w.window),"CMV Threshold Editor: Video");
  gtk_widget_set_usize (w.window, VIEW_WIDTH, VIEW_HEIGHT); 
  gtk_window_set_policy (GTK_WINDOW (w.window), FALSE, FALSE, FALSE); // not resizable

  //gtk_widget_set_uposition (window1, 0, 0);



  // gtk_container_set_border_width(GTK_CONTAINER(w.window),4); don't need this

  //w.vbox = gtk_vbox_new(FALSE,4);

  w.video = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(w.video),VIEW_WIDTH,VIEW_HEIGHT);
  //gtk_widget_set_events (w.video, GDK_BUTTON_PRESS_MASK);

  //w.output = gtk_drawing_area_new();
  //gtk_drawing_area_size(GTK_DRAWING_AREA(w.output),VIEW_WIDTH,VIEW_HEIGHT);

  // Pack widgets
  //gtk_container_add(GTK_CONTAINER(w.window),w.vbox);
  gtk_container_add(GTK_CONTAINER(w.window),w.video);

  gtk_signal_connect(GTK_OBJECT(w.video), "expose_event",
  	     GTK_SIGNAL_FUNC(video_video_expose),NULL);

  //gtk_box_pack_start(GTK_BOX(w.vbox),w.video,FALSE,FALSE,0);
  //gtk_box_pack_start(GTK_BOX(w.vbox),w.output,FALSE,FALSE,0);

  // Set up signals



  // Add destroy-on-close behavior
  gtk_signal_connect(GTK_OBJECT(w.window), "delete_event",
		     GTK_SIGNAL_FUNC(delete_event), NULL);

  /*gtk_signal_connect (GTK_OBJECT (w.window), "button_press_event",
                      GTK_SIGNAL_FUNC (video_button_press_event),
                      NULL);*/

  gtk_quit_add_destroy(1, GTK_OBJECT(w.window));

  // Show widgets
  gtk_widget_show(w.video);
  //gtk_widget_show(w.output);

  //gtk_widget_show(w.vbox);
  gtk_widget_show(w.window);

  w.can_update = true;
  gtk_signal_connect (GTK_OBJECT (w.window), "button_press_event",
                      GTK_SIGNAL_FUNC (crosshair_button_press),
                      NULL);
  // gtk_widget_set_events(w.video, GDK_BUTTON_PRESS_MASK);
}


//==== Histogram Window ==============================================//

gboolean hist_uv_expose(GtkWidget *widget,GdkEventExpose *event)
{
  gdk_draw_rgb_image(widget->window,widget->style->fg_gc[GTK_STATE_NORMAL],
		     0,0,HIST_SIZE,HIST_SIZE,GDK_RGB_DITHER_NONE,
		     (guchar*)app.uv_hist,HIST_SIZE*3);
  if(captured_image && crosshair_mode){
    image_pixel here = c_img[(app.cross_y * VIEW_WIDTH + app.cross_x)/2];

    draw_crosshair(widget->window, here.v, here.u);
  }
  return(TRUE);
}

gboolean hist_uy_expose(GtkWidget *widget,GdkEventExpose *event)
{
  gdk_draw_rgb_image(widget->window,widget->style->fg_gc[GTK_STATE_NORMAL],
			0,0,HIST_SIZE,HIST_SIZE,GDK_RGB_DITHER_NONE,
			(guchar*)app.uy_hist,HIST_SIZE*3);
  if(captured_image && crosshair_mode){
    image_pixel here = c_img[(app.cross_y * VIEW_WIDTH + app.cross_x)/2];
    int y = (here.y1 + here.y2)/2;
    //printf("y=%d, u=%d, v=%d\n", here.y1, here.u, here.v);
    draw_crosshair(widget->window, y, here.u);
  }
  return(TRUE);
}

gboolean hist_yv_expose(GtkWidget *widget,GdkEventExpose *event)
{
  gdk_draw_rgb_image(widget->window,widget->style->fg_gc[GTK_STATE_NORMAL],
			0,0,HIST_SIZE,HIST_SIZE,GDK_RGB_DITHER_NONE,
			(guchar*)app.yv_hist,HIST_SIZE*3);
  if(captured_image && crosshair_mode){
    image_pixel here = c_img[(app.cross_y * VIEW_WIDTH + app.cross_x)/2];
    int y = (here.y1 + here.y2)/2;
    draw_crosshair(widget->window, here.v, y);
  }
  return(TRUE);
}

void create_hist_window(hist_window &w)
{
  // Create widgets
  w.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(w.window),"CMV Threshold Editor");
  gtk_container_set_border_width(GTK_CONTAINER(w.window),2);

  w.table = gtk_table_new(2,2,FALSE);

  w.uv_hist = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(w.uv_hist),HIST_SIZE,HIST_SIZE);

  w.uy_hist = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(w.uy_hist),HIST_SIZE,HIST_SIZE);

  w.yv_hist = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(w.yv_hist),HIST_SIZE,HIST_SIZE);


  // Pack widgets
  gtk_container_add(GTK_CONTAINER(w.window),w.table);

  //GtkAttachOptions opfe = (GtkAttachOptions)(GTK_FILL|GTK_EXPAND);
  GtkAttachOptions opf  = (GtkAttachOptions)(GTK_FILL);
  int pad = 2;
  gtk_table_attach(GTK_TABLE(w.table),w.uv_hist,0,1,0,1,opf,opf,pad,pad);
  gtk_table_attach(GTK_TABLE(w.table),w.uy_hist,1,2,0,1,opf,opf,pad,pad);
  gtk_table_attach(GTK_TABLE(w.table),w.yv_hist,0,1,1,2,opf,opf,pad,pad);

  // Set up signals
  gtk_signal_connect(GTK_OBJECT(w.uv_hist), "expose_event",
		     GTK_SIGNAL_FUNC(hist_uv_expose),NULL);
  gtk_signal_connect(GTK_OBJECT(w.uy_hist), "expose_event",
		     GTK_SIGNAL_FUNC(hist_uy_expose),NULL);
  gtk_signal_connect(GTK_OBJECT(w.yv_hist), "expose_event",
		     GTK_SIGNAL_FUNC(hist_yv_expose),NULL);

  // Add destroy-on-close behavior
  gtk_signal_connect(GTK_OBJECT(w.window), "delete_event",
		     GTK_SIGNAL_FUNC(delete_event), NULL);
  gtk_quit_add_destroy(1, GTK_OBJECT(w.window));

  // Show widgets
  gtk_widget_show(w.uv_hist);
  gtk_widget_show(w.uy_hist);
  gtk_widget_show(w.yv_hist);

  gtk_widget_show(w.table);
  gtk_widget_show(w.window);

  w.can_update = true;
}


//==== Main Window Functions =========================================//

void create_windows(windows &w)
{
  // create_file_window(w.file,"");
  create_control_window(w.control);
  create_video_window(w.video);
  create_output_window(w.output);
  create_hist_window(w.hist);
}

void update_windows(windows &w,int frame)
{
  // expose video, blob, and hist windows
  if(!w.video.can_update && !w.output.can_update) return; // maybe check can_update for output, don't know, ZK

  if(frame % 2){
  	if(app.show_hist){
  		sem_wait(&app.hist_lock);
      hist_uv_expose(w.hist.uv_hist,NULL);
      hist_uy_expose(w.hist.uy_hist,NULL);
      hist_yv_expose(w.hist.yv_hist,NULL);
  		sem_post(&app.hist_lock);
    }
  }else{
    video_video_expose(w.video.video,NULL);
    video_output_expose(w.output.output,NULL);
  }
}

void destroy_windows(windows &w)
{
  // printf("DESTROY!\n");
  w.video.can_update = false;
  w.output.can_update = false;
  w.hist.can_update = false;
  // add any garbage collection needed
}
