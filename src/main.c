#include <pebble.h>  
#include "gbitmap_color_palette_manipulator.h"
  
static bool is24hrFormat = true;

Window* _window;
static Layer* window_layer;

TextLayer* date_text_layer;
static char date_text[] = "\0\0\0\0\0";
const char dateFormat[] = "%d%m";
static GFont s_font_teko_sb_20;

static BitmapLayer* hours_tens_layer = NULL;
static BitmapLayer* hours_ones_layer = NULL;
static GBitmap* hours_tens = NULL;
static GBitmap* hours_ones = NULL;

static BitmapLayer* minutes_tens_layer = NULL;
static BitmapLayer* minutes_ones_layer = NULL;
static GBitmap* minutes_tens = NULL;
static GBitmap* minutes_ones = NULL;

// Font is "True Lies" by Jonathan S. Harris
// http://tattoowoo.com/index.php?main_page=product_info&cPath=72&products_id=1678
// exported digits to PNG to be able to use large-size fonts (some characters are too big
// for the pebble font rasterizer even at 48 point).
const int IMAGE_RESOURCE_IDS[10] = {
  RESOURCE_ID_IMAGE_B_TL_0, RESOURCE_ID_IMAGE_B_TL_1, RESOURCE_ID_IMAGE_B_TL_2,
  RESOURCE_ID_IMAGE_B_TL_3, RESOURCE_ID_IMAGE_B_TL_4, RESOURCE_ID_IMAGE_B_TL_5,
  RESOURCE_ID_IMAGE_B_TL_6, RESOURCE_ID_IMAGE_B_TL_7, RESOURCE_ID_IMAGE_B_TL_8,
  RESOURCE_ID_IMAGE_B_TL_9
};

const short ONES_SPACINGS[10] = 
{
  7, //0
  0, //1
  0, //2
  5, //3
  7, //4
  0, //5
  5, //6
  3, //7
  0, //8
  17, //9
};

//#define DEBUGTIME

#ifdef DEBUGTIME
static int debugHours = 1;
static int debugMinutes = 0;
#endif

static void unload_bitmap(BitmapLayer** layer, GBitmap** bitmap){
  if(*layer){
    layer_remove_from_parent(bitmap_layer_get_layer(*layer));
    bitmap_layer_destroy(*layer);
    *layer = NULL;    
  }
  
  if(*bitmap){
    gbitmap_destroy(*bitmap);
    *bitmap = NULL;    
  }
}
  
static void load_bitmap(unsigned short image_number, BitmapLayer** layer, GBitmap** bitmap, GColor color, int x, int y)  {  
  unload_bitmap(layer, bitmap);
    
  *bitmap = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[image_number]);
  replace_gbitmap_color(GColorBlack, color, *bitmap, NULL);
  
#ifdef PBL_PLATFORM_BASALT
  GRect bounds = gbitmap_get_bounds(*bitmap);
#else
  GRect bounds = *bitmap->bounds;
#endif
  
  *layer = bitmap_layer_create(GRect(x,y, bounds.size.w, bounds.size.h));
  
  bitmap_layer_set_bitmap(*layer, *bitmap);
  bitmap_layer_set_compositing_mode(*layer, GCompOpSet);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(*layer));  
}

static void update_hours(unsigned short hours){  
  unsigned short hoursTensSpacing = 0;
  if(hours < 10 || hours == 24) // && show leading zero
  {
    load_bitmap(0,&hours_tens_layer, &hours_tens, GColorCyan, 2, 2); //0
    hoursTensSpacing = 5;
  }
  else
  {    
    int hourTens = hours/10;
    if(hourTens == 1)
    {
      load_bitmap(hourTens,&hours_tens_layer, &hours_tens, GColorCyan, 22, 2); //1
    }
    else // 2
    {
      load_bitmap(hourTens,&hours_tens_layer, &hours_tens, GColorCyan, 2, 2); //2       
      hoursTensSpacing = 14;
    }    
  }
  
  // hours ones
  if(hours == 24) // or 12 and 12hourtime
  {
    load_bitmap(0,&hours_ones_layer, &hours_ones, GColorCyan, 57, 2);   
  }
  else
  {    
    int hourOnes = hours%10;    
    load_bitmap(hourOnes,&hours_ones_layer, &hours_ones, GColorCyan, 48 + ONES_SPACINGS[hourOnes] + hoursTensSpacing, 2); 
  }    
}

static void update_minutes(unsigned short minutes){
  unsigned short minutesTensSpacing = 0;
  
  //tens
  if(minutes<10){    
    load_bitmap(0,&minutes_tens_layer, &minutes_tens, GColorMagenta, 5, 77) ;
  }
  else{
    unsigned short minuteTens = minutes/10;
    if(minuteTens == 1){
      load_bitmap(minuteTens,&minutes_tens_layer, &minutes_tens, GColorMagenta, 31, 77) ;  
    }
    else{
      load_bitmap(minuteTens,&minutes_tens_layer, &minutes_tens, GColorMagenta, 5, 77) ;
      if(minuteTens == 5 && minutes != 55){
        minutesTensSpacing = 5;
      }
      if(minuteTens == 2 && minutes != 25){
        minutesTensSpacing = 7;
      }
    }
  }
  
  //ones
  unsigned short minuteOnes = minutes%10;
  load_bitmap(minuteOnes,&minutes_ones_layer, &minutes_ones, GColorMagenta, 57 + ONES_SPACINGS[minuteOnes] + minutesTensSpacing, 77) ;
  
}

static unsigned short get_display_hour(unsigned short hour) {
  if (is24hrFormat) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed){  
#ifdef DEBUGTIME
  int hours = get_display_hour(debugHours);
#else
  int hours = get_display_hour(tick_time->tm_hour);
#endif

#ifdef DEBUGTIME
  int minutes = debugMinutes;
#else
  int minutes = tick_time->tm_min;
#endif
  
  update_hours(hours);
  update_minutes(minutes);

#ifdef DEBUGTIME
  ++debugHours;
  if(debugHours > 24){
    debugHours = 1;
  }
  ++debugMinutes;
  if(debugMinutes > 59){
    debugMinutes = 0;
  }    
#endif
  
  strftime(date_text, sizeof(date_text), dateFormat, tick_time);
  text_layer_set_text(date_text_layer, date_text);
}

static void window_load(Window *window) {    
  window_layer = window_get_root_layer(window);
  
  s_font_teko_sb_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_SB_20));
  
  date_text_layer = text_layer_create(GRect(77, 145, 144, 50));
  text_layer_set_text_color(date_text_layer, GColorMalachite);
  text_layer_set_background_color(date_text_layer, GColorClear);
  text_layer_set_text(date_text_layer, date_text);
  text_layer_set_font(date_text_layer, s_font_teko_sb_20);
  layer_add_child(window_layer, text_layer_get_layer(date_text_layer));
  
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);
}


static void window_unload(Window *window) {
  text_layer_destroy(date_text_layer);
  fonts_unload_custom_font(s_font_teko_sb_20);
  unload_bitmap(&hours_tens_layer, &hours_tens);
  unload_bitmap(&hours_ones_layer, &hours_ones);
  unload_bitmap(&minutes_tens_layer, &minutes_tens);
  unload_bitmap(&minutes_ones_layer, &minutes_ones);
}

void handle_init(void) {  
  is24hrFormat = clock_is_24h_style();
  
  _window = window_create();
  window_set_background_color(_window, GColorBlack);
  window_set_window_handlers(_window, (WindowHandlers) {.load = window_load, .unload = window_unload});
    
  window_stack_push(_window, true);
  
  
  tick_timer_service_subscribe(MONTH_UNIT | DAY_UNIT | HOUR_UNIT | MINUTE_UNIT 
#ifdef DEBUGTIME                               
                               | SECOND_UNIT
#endif
                               , handle_tick);
}

void handle_deinit(void) {
  
  window_destroy(_window);
}

int main(void) {  
  handle_init();
  app_event_loop();
  handle_deinit();
}
