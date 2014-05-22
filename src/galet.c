#include "galet.h"

static int SCREEN_WIDTH = 144;

static int SCREEN_HEIGHT = 168;

static Window *window;

static Layer *window_layer;

static TextLayer *clock_layer;

static TextLayer *date_layer;

static GRect battery_rect;

static bool wasCharging = false;

static bool isConnected = false;

void draw_battery_percent(GContext *ctx, BatteryChargeState state) {
    if (state.is_charging)
        graphics_fill_rect(ctx, GRect(1, 1, 10, 10), 0, GCornerNone);
    graphics_draw_rect(ctx, GRect(12, 1, 120, 10));
    graphics_fill_rect(ctx, GRect(14, 3, 1.08 * state.charge_percent, 6), 0, GCornerNone);
}

void draw_connectivity_icon(GContext *ctx) {
    if (!isConnected) {
        graphics_fill_rect(ctx, GRect(133, 1, 10, 10), 0, GCornerNone);
    }
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
    static char time_text[] = "00:00";
    char *time_format = clock_is_24h_style() ? "%R" : "%I:%M";
    strftime(time_text, sizeof(time_text), time_format, tick_time);
    text_layer_set_text(clock_layer, time_text);
    // Update date
    static char date_text[] = "Xxxxxxxxx 00";
    strftime(date_text, sizeof(date_text), "%B %e", tick_time);
    text_layer_set_text(date_layer, date_text);
}

static void handle_connectivity_changes(bool connected) {
    isConnected = connected;
    layer_mark_dirty(window_layer);
    // Warn the user in case of missing phone
    if (!connected) 
        vibes_short_pulse();
}

static void clean_screen(GContext *ctx) {
    GRect bounds = layer_get_bounds(window_layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void window_update_proc(Layer *layer, GContext *ctx) {
    clean_screen(ctx);
    // Fill with white
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    draw_battery(ctx);    
    draw_connectivity_icon(ctx);
    //draw_clock(ctx);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
    return 0;
}

void init(void) {
    window = window_create();
    window_set_fullscreen(window, true);
    window_stack_push(window, false /* animated */);
    window_layer = window_get_root_layer(window);
    layer_set_update_proc(window_layer, window_update_proc);
    // Subscribe for battery updates
    battery_state_service_subscribe(handle_battery_change);
    // Check connection and subscribe
    isConnected = bluetooth_connection_service_peek();
    bluetooth_connection_service_subscribe(handle_connectivity_changes);	
    // Init clock layer
    clock_layer = text_layer_create(GRect(10, 12, SCREEN_WIDTH - 10, 49));
    text_layer_set_text_color(clock_layer, GColorWhite);
    text_layer_set_background_color(clock_layer, GColorBlack);
    text_layer_set_font(clock_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    layer_add_child(window_layer, text_layer_get_layer(clock_layer));
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

    // Init date layer
    date_layer = text_layer_create(GRect(10, 68, SCREEN_WIDTH - 20, 75));
    text_layer_set_text_color(date_layer, GColorWhite);
    text_layer_set_background_color(date_layer, GColorBlack);
    text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
    text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

void deinit(void) {
    window_destroy(window);
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();	
    tick_timer_service_unsubscribe();
}

