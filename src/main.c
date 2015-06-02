/*

  Polar Clock Lite DP watch (SDK 3.0)

  Thanks to Team Pebble's Segment Six watchface...it was a big help!

 */

#include "pebble.h"

static char time_text[] = "00:00";
static char date_text[] = "00 Xxx";

Window *window;

Layer *minute_display_layer;

TextLayer *text_time_layer;
TextLayer *text_date_layer;
bool time_layer_exists = false;
bool date_layer_exists = false;

const GPathInfo MINUTE_SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-7, -70}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
    {7,  -70},
  }
};

static GPath *minute_segment_path;

static void minute_display_layer_update_callback(Layer *layer, GContext* ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned int angle = t->tm_min * 6;

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_circle(ctx, center, 65);

  graphics_context_set_fill_color(ctx, GColorBlack);

  for(; angle < 355; angle += 6) {

    gpath_rotate_to(minute_segment_path, (TRIG_MAX_ANGLE / 360) * angle);

    gpath_draw_filled(ctx, minute_segment_path);

  }

  graphics_fill_circle(ctx, center, 60);

}


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  
  layer_mark_dirty(minute_display_layer);

  if (time_layer_exists)
  {  
    char *time_format;
  
    if (clock_is_24h_style()) {
      time_format = "%R";
    } else {
      time_format = "%I:%M";
    }
  
    strftime(time_text, sizeof(time_text), time_format, tick_time);
    // Kludge to handle lack of non-padded hour format string
    // for twelve hour clock.
    if (time_text[0] == '0') {
      memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }
  
    text_layer_set_text(text_time_layer, time_text);
  }

  if (date_layer_exists)
  {
    //static char date_text[] = "00 Xxx";
    //date_text = "Xxx xx";
    strftime(date_text, sizeof(date_text), "%b %d", tick_time);
    text_layer_set_text(text_date_layer, date_text);
    //text_layer_set_text(text_date_layer, date_text);
  }
}

static void setup_time_date_layers() {
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  if(time_layer_exists) {
    text_layer_destroy(text_time_layer);
    time_layer_exists = false;  
  }
  
  if(date_layer_exists) {
    text_layer_destroy(text_date_layer);
    date_layer_exists = false;    
  }
  
  text_time_layer = text_layer_create(bounds);
  time_layer_exists = true;
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
    
  layer_set_frame(text_layer_get_layer(text_time_layer), GRect(0, 57, 144, 168-57));


  text_date_layer = text_layer_create(bounds);
  date_layer_exists = true;
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
  
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  layer_set_frame(text_layer_get_layer(text_date_layer), GRect(0, 100, 144, 168-100));
}




static void init(void) {  
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, true);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Init the layer for the minute display
  minute_display_layer = layer_create(bounds);
  layer_set_update_proc(minute_display_layer, minute_display_layer_update_callback);
  layer_add_child(window_layer, minute_display_layer);

  // Init the minute segment path
  minute_segment_path = gpath_create(&MINUTE_SEGMENT_PATH_POINTS);
  gpath_move_to(minute_segment_path, grect_center_point(&bounds));

  setup_time_date_layers();
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
  gpath_destroy(minute_segment_path);

  tick_timer_service_unsubscribe();
  window_destroy(window);
  layer_destroy(minute_display_layer);
  
  if (time_layer_exists) text_layer_destroy(text_time_layer);
  if (date_layer_exists) text_layer_destroy(text_date_layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
