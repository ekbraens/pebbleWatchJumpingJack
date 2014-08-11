//
//  main.c
//  jumpingJackCounter
//
//  Created by New on 8/03/14.
//  Copyright (c) 2014 New. All rights reserved.
//

#include "pebble.h"

// This is a custom defined key for saving our count field
#define TARGET_JACKS_PKEY 1

// You can define defaults for values in persistent storage
// so you do not have to set how many jumping jacks you want to do each time
#define TARGET_JACKS_DEFAULT 0

static Window *window;

// the integer that keeps track of how many jumping jacks you have completed
int doneJacks = 0;

// icons
static GBitmap *action_icon_plus;
static GBitmap *action_icon_minus;

static ActionBarLayer *action_bar;

static TextLayer *header_text_layer;
static TextLayer *body_text_layer;

static AppTimer *timer;

// We'll save the count in memory from persistent storage
static int target_jacks = TARGET_JACKS_DEFAULT;

// shows you how many jumping jacks you want to complete on the watch face
static void update_text() {
    static char body_text[50];
    snprintf(body_text, sizeof(body_text), "%u Jacks", target_jacks);
    text_layer_set_text(body_text_layer, body_text);
}

// once you have started jumping, the watch face shows you how many you have done
static void jumping_text(){
    static char body_text[50];
    snprintf(body_text, sizeof(body_text), "%u DONE", doneJacks);
    text_layer_set_text(body_text_layer, body_text);
}

// clicking the right buttons, you can increment or decrement
// the target jumping jacks you want to accomplish
static void increment_click_handler(ClickRecognizerRef recognizer, void *context) {
    target_jacks++;
    update_text();
}
static void decrement_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (target_jacks <= 0) {
        // Keep the counter at zero
        return;
    }
    target_jacks--;
    update_text();
}

// if you hold down the left buttons, it will increment/decrement faster
static void click_config_provider(void *context) {
    const uint16_t repeat_interval_ms = 50;
    window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, (ClickHandler) increment_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, (ClickHandler) decrement_click_handler);
}

// the accelerometer handles when we wearer does one jumping jack
void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    // Process tap on ACCEL_AXIS_X, ACCEL_AXIS_Y or ACCEL_AXIS_Z
    // Direction is 1 or -1
    
    ++doneJacks;
    jumping_text();
    
    // if you have done the target number of jumping jacks or beyond,
    // the watch lets you know by vibrating
    if(doneJacks >= target_jacks) {
        vibes_long_pulse();
    }
}

// creating the text you see on the watch face
static void window_load(Window *me) {
    action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(action_bar, me);
    action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
    
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_minus);
    
    Layer *layer = window_get_root_layer(me);
    const int16_t width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 3;
    
    header_text_layer = text_layer_create(GRect(4, 0, width, 60));
    text_layer_set_font(header_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_background_color(header_text_layer, GColorClear);
    text_layer_set_text(header_text_layer, "Jumping Jacks");
    layer_add_child(layer, text_layer_get_layer(header_text_layer));
    
    body_text_layer = text_layer_create(GRect(4, 44, width, 60));
    text_layer_set_font(body_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(body_text_layer, GColorClear);
    layer_add_child(layer, text_layer_get_layer(body_text_layer));
    
    update_text();
}

// when we leave the application, release all the memory
static void window_unload(Window *window) {
    text_layer_destroy(header_text_layer);
    text_layer_destroy(body_text_layer);
    
    action_bar_layer_destroy(action_bar);
}

static void init(void) {
    accel_tap_service_subscribe(accel_tap_handler);
    
    action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
    action_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);
    
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    
    // Get the count from persistent storage for use if it exists, otherwise use the default
    target_jacks = persist_exists(TARGET_JACKS_PKEY) ? persist_read_int(TARGET_JACKS_PKEY) : TARGET_JACKS_DEFAULT;
    
    window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
    // Save the count into persistent storage on app exit
    accel_tap_service_unsubscribe();
    
    persist_write_int(TARGET_JACKS_PKEY, target_jacks);
    
    window_destroy(window);
    
    gbitmap_destroy(action_icon_plus);
    gbitmap_destroy(action_icon_minus);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
