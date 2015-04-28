#ifndef CIRCLES_LOCAL_H
#define CIRCLES_LOCAL_H
	
#include <pebble.h>
	
int get_time(int unit);
void update_implementation (Animation *animation, const uint32_t distance_normalized);
void update_startup_implementation (Animation *animation, const uint32_t distance_normalized);
void on_animation_stop(Animation * animation, bool finished, void *ctx);
void on_animation_startup_stop(Animation * animation, bool finished, void *ctx);
void on_animation_shutdown_stop(Animation * animation, bool finished, void *ctx);
void trigger_animation (int duration, int delay);
void trigger_startup_animation (int duration, int delay);
//void trigger_shutdown_animation (int duration, int delay);
void setup_my_path_hour(int hour, int minute, GContext * ctx);
void setup_my_path_min(int minute, GContext * ctx);
static void background_layer_update_callback(Layer* me, GContext* ctx);
static void outter_min_layer_update_callback(Layer *me, GContext *ctx);
static void inner_min_layer_update_callback(Layer *me, GContext *ctx);
static void outter_hour_layer_update_callback(Layer *me, GContext *ctx);
static void inner_hour_layer_update_callback(Layer *me, GContext *ctx);
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed);
static void in_recv_handler(DictionaryIterator *iterator, void *context);
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void init();
static void deinit();
