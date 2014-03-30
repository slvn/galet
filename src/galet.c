#include "galet.h"

static Window *window;

static Layer *window_layer;

static TextLayer *text_layer;

static GRect battery_rect;

static bool wasCharging = false;

void draw_battery_percent(GContext *ctx, BatteryChargeState state) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorWhite);
    if (state.is_charging)
        graphics_fill_rect(ctx, GRect(1, 1, 10, 10), 0, GCornerNone);
    graphics_draw_rect(ctx, GRect(12, 1, 120, 10));
    graphics_fill_rect(ctx, GRect(14, 3, 1.08 * state.charge_percent, 6), 0, GCornerNone);
}

void draw_battery(GContext *ctx) {
    draw_battery_percent(ctx, battery_state_service_peek());
}

void handle_battery_change(BatteryChargeState state) {
    if (state.is_charging != wasCharging) {
	wasCharging = state.is_charging;
        layer_mark_dirty(window_layer);
    }
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(window_layer);
}

static void clean_screen(GContext *ctx) {
  GRect bounds = layer_get_bounds(window_layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void window_update_proc(Layer *layer, GContext *ctx) {
    clean_screen(ctx);
    draw_battery(ctx);    
}

int main(void) {
    init();
    app_event_loop();
    deinit();
    return 0;
}

void init(void) {
    window = window_create();
    window_stack_push(window, false /* animated */);
    window_layer = window_get_root_layer(window);
    layer_set_update_proc(window_layer, window_update_proc);
    battery_state_service_subscribe(handle_battery_change);
}

void deinit(void) {
    window_destroy(window);
    battery_state_service_subscribe(handle_battery_change);
}

