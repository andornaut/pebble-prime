#include <pebble.h>

#define CORNER_RADIUS 8
#define TEXT_LAYER_HEIGHT 76
#define WARN_CHARGE_PERCENTAGE 15

static uint8_t s_charge_percentage = 100;
static TextLayer *s_day_of_week_layer, *s_day_of_month_layer, *s_time_layer;
static bool s_is_connected = true;
static Layer *s_text_layer;
static Window *s_window;

static void update_time(struct tm *t) {
  static char s_day_of_week_buffer[4], s_day_of_month_buffer[3], s_time_buffer[5];

  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%k%M" : "%l%M", t);
  text_layer_set_text(s_time_layer, s_time_buffer);

  strftime(s_day_of_week_buffer, sizeof(s_day_of_week_buffer), "%a", t);
  text_layer_set_text(s_day_of_week_layer, s_day_of_week_buffer);

  strftime(s_day_of_month_buffer, sizeof(s_day_of_month_buffer), "%d", t);
  text_layer_set_text(s_day_of_month_layer, s_day_of_month_buffer);
}

static void update_window(Layer *layer, GContext *ctx) {
  const GColor
    inner_bg_color = s_is_connected ?
      GColorWhite : PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray),
    divider_bg_color = (s_is_connected && s_charge_percentage > WARN_CHARGE_PERCENTAGE) ?
      GColorWhite : PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray),
    window_bg_color = s_is_connected ?
      GColorBlack : PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray);

  const GRect window_bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, window_bg_color);
  graphics_fill_rect(ctx, window_bounds, 0, GCornerNone);

  const uint8_t
    max_vert_padding = (window_bounds.size.h - TEXT_LAYER_HEIGHT) / 2,
    vert_padding = (100 - s_charge_percentage) / 100.0 * max_vert_padding;
  const GRect divider_bounds = GRect(
    window_bounds.origin.x, window_bounds.origin.y + vert_padding,
    window_bounds.size.w, window_bounds.size.h - vert_padding * 2);
  graphics_context_set_fill_color(ctx, divider_bg_color);
  graphics_fill_rect(ctx,  divider_bounds, CORNER_RADIUS, GCornersAll);

  const uint8_t border_padding = 4;
  const GRect inner_bounds = GRect(
    divider_bounds.origin.x + border_padding, divider_bounds.origin.y + border_padding,
    divider_bounds.size.w - border_padding * 2, divider_bounds.size.h - border_padding * 2);
  graphics_context_set_fill_color(ctx, inner_bg_color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_fill_rect(ctx,  inner_bounds, CORNER_RADIUS, GCornersAll);
  graphics_draw_round_rect(ctx,  inner_bounds, CORNER_RADIUS);
}

static void handle_battery_event(BatteryChargeState charge_state) {
  s_charge_percentage = charge_state.charge_percent;
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void handle_connection_event(bool is_connected) {
  s_is_connected = is_connected;
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void handle_tick_event(struct tm *t, TimeUnits units_changed) {
  update_time(t);
}

static void load_date_layers(GRect bounds) {
  const GFont font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_BOLD_22));
  const uint8_t font_height = 22, top_padding = 2;

  s_day_of_week_layer = text_layer_create(
    GRect(
      bounds.size.w / 5 * 2, top_padding,
      bounds.size.w / 5 * 2, font_height));
  text_layer_set_background_color(s_day_of_week_layer, GColorClear);
  text_layer_set_font(s_day_of_week_layer, font);
  text_layer_set_text_alignment(s_day_of_week_layer, GTextAlignmentCenter);
  layer_add_child(s_text_layer, text_layer_get_layer(s_day_of_week_layer));

  s_day_of_month_layer = text_layer_create(
    GRect(
      bounds.size.w / 5 * 4, top_padding,
      bounds.size.w / 5, font_height));
  text_layer_set_background_color(s_day_of_month_layer, GColorClear);
  text_layer_set_font(s_day_of_month_layer, font);
  text_layer_set_text_alignment(s_day_of_month_layer, GTextAlignmentCenter);
  layer_add_child(s_text_layer, text_layer_get_layer(s_day_of_month_layer));
}

static void load_time_layer(GRect bounds) {
  const uint8_t font_height = 52, top_padding = 15;
  s_time_layer = text_layer_create(GRect(0, top_padding, bounds.size.w, font_height));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(
    s_time_layer,
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_REGULAR_52)));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(s_text_layer, text_layer_get_layer(s_time_layer));
}

static void load_window(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  layer_set_update_proc(window_layer, update_window);

  const GRect window_bounds = layer_get_bounds(window_layer);
  const uint8_t horiz_padding = 6;
  s_text_layer = layer_create(
    GRect(
      horiz_padding, window_bounds.size.h / 2 - TEXT_LAYER_HEIGHT / 2,
      window_bounds.size.w - horiz_padding * 2, TEXT_LAYER_HEIGHT));
  layer_add_child(window_layer, s_text_layer);

  const GRect text_bounds = layer_get_bounds(s_text_layer);
  load_time_layer(text_bounds);
  load_date_layers(text_bounds);

  handle_battery_event(battery_state_service_peek());
  handle_connection_event(connection_service_peek_pebble_app_connection());
  battery_state_service_subscribe(handle_battery_event);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_connection_event,
  });
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick_event);
}

static void unload_window(Window *window) {
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  tick_timer_service_unsubscribe();

  text_layer_destroy(s_day_of_week_layer);
  text_layer_destroy(s_day_of_month_layer);
  text_layer_destroy(s_time_layer);
  layer_destroy(s_text_layer);
}

static void init() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = load_window,
    .unload = unload_window,
  });
  const bool is_animated = true;
  window_stack_push(s_window, is_animated);
}

static void deinit() {
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
