#include <pebble.h>  
#include "gbitmap_color_palette_manipulator.h"

  
#define KEY_DATE_FORMAT 0
#define KEY_BT_TOGGLE 1
#define KEY_LZ_TOGGLE 2
#define KEY_LA_TOGGLE 3
  
static bool is24hrFormat = true;

Window* _window;
static Layer* window_layer;

TextLayer* date_text_layer;
static char date_text[] = "\0\0\0\0\0";
static char dateFormat[] = "%d%m";
const char DDMMdateFormat[] = "%d%m";
const char DDMM_FORMAT_KEY[] = "DDMM";
const char MMDDdateFormat[] = "%m%d";
const char MMDD_FORMAT_KEY[] = "MMDD";
const char OFF_FORMAT_KEY[] = "OFF";
static char PERSISTED_FORMAT[] = "DDMM";
static GFont s_font_teko_sb_20;

static BitmapLayer* hours_tens_layer = NULL;
static BitmapLayer* hours_ones_layer = NULL;
static GBitmap* hours_tens = NULL;
static GBitmap* hours_ones = NULL;

static BitmapLayer* minutes_tens_layer = NULL;
static BitmapLayer* minutes_ones_layer = NULL;
static GBitmap* minutes_tens = NULL;
static GBitmap* minutes_ones = NULL;

static int top_y = 2;
const int TOP_Y_WITH_DATE = 2;
const int TOP_Y_WITHOUT_DATE = 10;

static int mins_spacing_y = 75;
const int MINS_SPACING_Y_WITH_DATE = 75;
const int MINS_SPACING_Y_WITHOUT_DATE = 79;

static bool show_leading_zero = true;
static bool left_align_hour = false;


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

static void force_tick();
  
  
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
  unsigned short hoursOnesX = 48;
  if(hours < 10 || hours == 24) // && show leading zero
  {
    hoursTensSpacing = 5;
    
    if(show_leading_zero){
      load_bitmap(0,&hours_tens_layer, &hours_tens, GColorCyan, 2, top_y); //0 
    }
    else{
      unload_bitmap(&hours_tens_layer, &hours_tens);
      if(left_align_hour){
        hoursOnesX = 2;
      }
    }      
    
  }
  else
  {    
    int hourTens = hours/10;
    if(hourTens == 1)
    {
      load_bitmap(hourTens,&hours_tens_layer, &hours_tens, GColorCyan, 22, top_y); //1
    }
    else // 2
    {
      load_bitmap(hourTens,&hours_tens_layer, &hours_tens, GColorCyan, 2, top_y); //2       
      hoursTensSpacing = 14;
    }    
  }
  
  // hours ones
  if(hours == 24) // or 12 and 12hourtime
  {
    if(show_leading_zero == true || (show_leading_zero == false && left_align_hour == false)) {
      hoursOnesX += 9;
    }
    load_bitmap(0,&hours_ones_layer, &hours_ones, GColorCyan, hoursOnesX, top_y);   
  }
  else
  {    
    int hourOnes = hours%10;    
    if(show_leading_zero == true || (show_leading_zero == false && left_align_hour == false)) {
      hoursOnesX = hoursOnesX + ONES_SPACINGS[hourOnes] + hoursTensSpacing;
    }
    else{
      if(hourOnes == 1){
        hoursOnesX += 20;
      }
      hoursOnesX = hoursOnesX + ONES_SPACINGS[hourOnes];
    }
    load_bitmap(hourOnes,&hours_ones_layer, &hours_ones, GColorCyan, hoursOnesX, top_y); 
  }    
}

static void update_minutes(unsigned short minutes){
  unsigned short minutesTensSpacing = 0;
  
  //tens
  if(minutes<10){    
    load_bitmap(0,&minutes_tens_layer, &minutes_tens, GColorMagenta, 5, top_y + mins_spacing_y) ;
  }
  else{
    unsigned short minuteTens = minutes/10;
    if(minuteTens == 1){
      load_bitmap(minuteTens,&minutes_tens_layer, &minutes_tens, GColorMagenta, 31, top_y + mins_spacing_y) ;  
    }
    else{
      load_bitmap(minuteTens,&minutes_tens_layer, &minutes_tens, GColorMagenta, 5, top_y + mins_spacing_y) ;
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
  load_bitmap(minuteOnes,&minutes_ones_layer, &minutes_ones, GColorMagenta, 57 + ONES_SPACINGS[minuteOnes] + minutesTensSpacing, top_y + mins_spacing_y) ;
  
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

static void set_date_format(char* dateFormatOption)
{
  if(strcmp(OFF_FORMAT_KEY, dateFormatOption) == 0)   {
    strcpy(dateFormat, "\0");
    layer_set_hidden(text_layer_get_layer(date_text_layer), true);    
    top_y = TOP_Y_WITHOUT_DATE;
    mins_spacing_y = MINS_SPACING_Y_WITHOUT_DATE;
    force_tick();
    return;
  }
  
  if(strcmp(DDMM_FORMAT_KEY, dateFormatOption) == 0){
    strcpy(dateFormat, DDMMdateFormat);
    layer_set_hidden(text_layer_get_layer(date_text_layer), false);
    top_y = TOP_Y_WITH_DATE;
    mins_spacing_y = MINS_SPACING_Y_WITH_DATE;
    force_tick();
    return;
  }
  
  if(strcmp(MMDD_FORMAT_KEY, dateFormatOption) == 0){
    strcpy(dateFormat, MMDDdateFormat);
    layer_set_hidden(text_layer_get_layer(date_text_layer), false);
    top_y = TOP_Y_WITH_DATE;
    mins_spacing_y = MINS_SPACING_Y_WITH_DATE;
    force_tick();
    return;
  }
}

static void force_tick(){
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);
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
  
  force_tick();
}


static void window_unload(Window *window) {
  text_layer_destroy(date_text_layer);
  fonts_unload_custom_font(s_font_teko_sb_20);
  unload_bitmap(&hours_tens_layer, &hours_tens);
  unload_bitmap(&hours_ones_layer, &hours_ones);
  unload_bitmap(&minutes_tens_layer, &minutes_tens);
  unload_bitmap(&minutes_ones_layer, &minutes_ones);
}

// thanks to @faquin :)
// https://github.com/faquin/miami-nights-pebble
static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
    vibes_long_pulse(); // vibrate long pulse when connection is back
  } else {
    vibes_double_pulse(); // vibrate two short pulses when connection is lost
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  
  Tuple* bluetooth_toggle_t = dict_find(iter, KEY_BT_TOGGLE);
  if(bluetooth_toggle_t && bluetooth_toggle_t->value->int32 > 0) {
    
    // Enable bluetooth notification
    bluetooth_connection_service_subscribe(bt_handler);
    bt_handler(true);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "BT Toggle: TRUE");
    
    // Persist value
    persist_write_bool(KEY_BT_TOGGLE, true);    
  } else {
    bluetooth_connection_service_unsubscribe();
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "BT Toggle: FALSE");
    
    // persist
    persist_write_bool(KEY_BT_TOGGLE, false);
  }
  
  Tuple* date_format_t = dict_find(iter, KEY_DATE_FORMAT);  
  if(date_format_t) {
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Date Format: %s", date_format_t->value->cstring);
    
    set_date_format(date_format_t->value->cstring);
    
    // Persist value
    persist_write_string(KEY_DATE_FORMAT, date_format_t->value->cstring);
  }
  
  Tuple* leading_zero_toggle_t = dict_find(iter, KEY_LZ_TOGGLE);
  if(leading_zero_toggle_t && leading_zero_toggle_t->value->int32 > 0) {
    show_leading_zero = true;
    
    // Persist value
    persist_write_bool(KEY_LZ_TOGGLE, true);    
  } else {
    show_leading_zero = false;
    
    // persist
    persist_write_bool(KEY_LZ_TOGGLE, false);
  }
  
  Tuple* left_align_toggle_t = dict_find(iter, KEY_LA_TOGGLE);
  if(left_align_toggle_t && left_align_toggle_t->value->int32 > 0) {
    left_align_hour = true;
    
    // Persist value
    persist_write_bool(KEY_LA_TOGGLE, true);    
  } else {
    left_align_hour = false;
    
    // persist
    persist_write_bool(KEY_LA_TOGGLE, false);
  }
  
  force_tick();
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
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  if(persist_exists(KEY_DATE_FORMAT)){
    persist_read_string(KEY_DATE_FORMAT, PERSISTED_FORMAT, sizeof(PERSISTED_FORMAT));
    set_date_format(PERSISTED_FORMAT);
  }
  if(persist_exists(KEY_BT_TOGGLE)){
    if(persist_read_bool(KEY_BT_TOGGLE)){
      bluetooth_connection_service_subscribe(bt_handler);
    }    
  }
  else{ // default to BT notif enabled
    bluetooth_connection_service_subscribe(bt_handler); 
  }
  
  if(persist_exists(KEY_LZ_TOGGLE)){
    show_leading_zero = persist_read_bool(KEY_LZ_TOGGLE);    
  }
  else{ // default to show leading zero
    show_leading_zero = true;
  }
  
  if(persist_exists(KEY_LA_TOGGLE)){
    left_align_hour = persist_read_bool(KEY_LA_TOGGLE);    
  }
  else{ // default to disable leftalign hour
    left_align_hour = false;
  }    
}

void handle_deinit(void) {
  app_message_deregister_callbacks();
  window_destroy(_window);
}

int main(void) {  
  handle_init();
  app_event_loop();
  handle_deinit();
}
