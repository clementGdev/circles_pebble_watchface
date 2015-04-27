/************************************************************************/
/* This is a Pebble Watchface Made By Clement Guillou					*/
/* Date : 02/03/15														*/
/*																		*/
/* Version : 1.1														*/
/* Version note : 	first release, app icon								*/
/* 																		*/
/* TODO : Animations, Settings, color support for pebble time			*/
/* COMMENT THE CODE !!!!!												*/
/*																		*/
/* 							3.0 SDK										*/
/************************************************************************/




// TODO :
//			Get rid of the text layer (App crashes without it ?)
// 			Animations (Almost done)
// 			Color selection (Settings with js)
// 			MACRO for aplite and basalt : Done
//			Define constants for easier changes in the config.h file
//				All values must be replaced by a const in the code
//		IMPORTANT
//			Take care of the rotation issue during the animation !!
//			This is fucking ugly now that the animation is slower
//
//			Take care of the first image that shouldn't be here (before animation_startup ends)

//		It might come from the value of the angle.
//		To test this out, I should draw another GPath in the middle and rotate it with different values.
//
//		Start thinking about the settings
//		Use a tool like the one one the resources page of pebble to choose  color would be nice

/*
 Animations :
 
 On Startup
 __________
 Reverse anim is called on startup...
 Both circles will grow from the middle, swirling once
 
 
 On Time change
 ______________
 Other possibility : Swirl and shrink then swirl and grow.
 
 Minute Circle swirls once
 
 
 On Shutdown
 ___________
 If it's possible, do a reverse startup animation when the app is shut down
 NOT POSSIBLE AT THE MOMENT
 A push on a button exits the loop and go into the deinit() proc, where it is not possible to do timed things...
 Maybe I could use those .pdc mentioned in the blog post from April 3rd. Wait and see
 */

//////////////////////////////////////////////////////////////


#include <pebble.h>
#include "color_definition.h"
#include "config.h"

	
#define KEY_BACKGROUND_COLOR 	20
#define KEY_CIRCLES_COLOR 		21

static Window *s_main_window;

static TextLayer *s_time_layer;

static Layer *outter_hour_layer;
static Layer *inner_hour_layer;

static Layer *outter_min_layer;
static Layer *inner_min_layer;


/********************************************************/
/*					HARDWARE SUPPORT					*/
/********************************************************/
#ifdef PBL_COLOR
static GColor back_color = (GColor) GColorBlueMoonARGB8;
static GColor circle_color = (GColor) GColorWhiteARGB8;
#else
static GColor back_color = GColorBlack;
static GColor circle_color = GColorWhite;
#endif
/********************************************************/
/********************************************************/

static GPath *s_my_path_ptr_hour = NULL;
static GPath *s_my_path_ptr_min = NULL;


static const GPathInfo BOLT_PATH_INFO = {
	.num_points = 4,
	.points = (GPoint []) {{-7, -7}, {7, -7}, {7, 7} , {-7, 7}}
};



int get_time(int unit) {
	time_t now = time(NULL);
	struct tm * t = localtime(&now);
	
	int second = t->tm_sec;
	int minute = t->tm_min;
	int hour = t->tm_hour;
	switch (unit) {
		case 1:
			return second;
		case 2:
			return minute;
		case 3:
			return hour;
		default:
			return 0;
	}
}


#ifdef ANIMATION
/********************************************************/
/*						ANIMATIONS 						*/
/********************************************************/


//TO DO :
// 			Startup animation needs to move minute path too
//			It doesn't

// Debugging : Reverse anim and anim...	
	
static Animation* anim;
static Animation* startup_anim;
static Animation* shutdown_anim;

static int animation_flag = 0;
static int animation_on_startup_flag = 1;
static int animation_startup_flag = 0;

static int animation_variable = 0;
static int animation_startup_variable = 0;

void update_implementation (Animation *animation, const uint32_t distance_normalized) {
	
	animation_variable = (int)distance_normalized;
	animation_flag = 1;
	layer_mark_dirty(outter_hour_layer);
	
	return;
}

const AnimationImplementation anim_implementation = {
	.update = (AnimationUpdateImplementation) update_implementation,
};


// ANIMATION DESTROY
void on_animation_stop(Animation * animation, bool finished, void *ctx) {
	animation_flag = 0;
	layer_mark_dirty(outter_hour_layer);
	
#ifdef PBL_PLATFORM_APLITE
	animation_destroy(animation);
#endif
	
}

void trigger_animation (int duration, int delay) {
	anim = animation_create();
	animation_set_duration((Animation*) anim, duration);
	animation_set_delay((Animation*) anim, delay);
	animation_set_curve((Animation *) anim, AnimationCurveEaseOut);
	
	AnimationHandlers anim_handlers = {
		.stopped = (AnimationStoppedHandler) on_animation_stop
	};
	
	animation_set_handlers (anim, anim_handlers, NULL);
	
	animation_set_implementation(anim, &anim_implementation);
	
	animation_schedule((Animation *) anim);
	
}



////////////////////////////////////////////////////////////////


void update_startup_implementation (Animation *animation, const uint32_t distance_normalized) {

	animation_startup_variable = (int)distance_normalized;
	animation_startup_flag = 1;
	
	layer_mark_dirty(outter_min_layer);
	
	return;
}

const AnimationImplementation anim_startup_implementation = {
	.update = (AnimationUpdateImplementation) update_startup_implementation,
};



// ANIMATION DESTROY
void on_animation_startup_stop(Animation * animation, bool finished, void *ctx) {
	animation_on_startup_flag = 0;
	animation_startup_flag = 0;
	APP_LOG(APP_LOG_LEVEL_INFO, "Animation stopped");
	layer_mark_dirty(outter_min_layer);		// Re render all the layers
	
	/* ANIMATION_DESTROY MAKES THE APP CRASH ON BASALT... */
	/* It works without it but it's not very safe for the memory */
	// Actually, on the basalt platform, the Pebble_OS frees the memory on its own
	// No need to call animation_destroy on Basalt
#ifdef PBL_PLATFORM_APLITE
	animation_destroy(animation);
#endif
	
}


void trigger_startup_animation (int duration, int delay) {
	startup_anim = animation_create();
	
	animation_set_duration((Animation*) startup_anim, duration);
	animation_set_delay((Animation*) startup_anim, delay);
	animation_set_curve((Animation *) startup_anim, AnimationCurveEaseInOut);
	
	AnimationHandlers startup_anim_handlers = {
		.stopped = (AnimationStoppedHandler) on_animation_startup_stop
	};
	
	animation_set_handlers (startup_anim, startup_anim_handlers, NULL);
	
	animation_set_implementation(startup_anim, &anim_startup_implementation);
	
	animation_schedule((Animation *) startup_anim);
	
}

////////////////////////////////////////////////////////////

// ANIMATION DESTROY
void on_animation_shutdown_stop(Animation * animation, bool finished, void *ctx) {
	animation_on_startup_flag = 0;
	animation_startup_flag = 0;
	APP_LOG(APP_LOG_LEVEL_INFO, "Animation stopped");
	layer_mark_dirty(outter_min_layer);		// Re render all the layers
	
	/* ANIMATION_DESTROY MAKES THE APP CRASH ON BASALT... */
	/* It works without it but it's not very safe for the memory */
	// Actually, on the basalt platform, the Pebble_OS frees the memory on its own
	// No need to call animation_destroy on Basalt
	trigger_startup_animation(ANIMATION_DURATION, 0);
	//trigger_animation(ANIMATION_DURATION, 0);
#ifdef PBL_PLATFORM_APLITE
	animation_destroy(animation);
#endif
	
}


#ifdef PBL_PLATFORM_BASALT
void trigger_shutdown_animation (int duration, int delay) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Animation triggered");
	shutdown_anim = animation_create();
	
	animation_set_duration((Animation*) shutdown_anim, duration);
	animation_set_delay((Animation*) shutdown_anim, delay);
	animation_set_curve((Animation *) shutdown_anim, AnimationCurveEaseInOut);
	
	AnimationHandlers startup_anim_handlers = {
		.stopped = (AnimationStoppedHandler) on_animation_shutdown_stop
	};
	
	animation_set_handlers (shutdown_anim, startup_anim_handlers, NULL);
	
	animation_set_implementation(shutdown_anim, &anim_startup_implementation);
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Animation reversed");
	animation_set_reverse(shutdown_anim, true);

	APP_LOG(APP_LOG_LEVEL_INFO, "Scheduling animation");
	animation_schedule((Animation *) shutdown_anim);
	APP_LOG(APP_LOG_LEVEL_INFO, "Animation scheduled");
	
}
#endif

#endif	// ANIMATIONS

/********************************************************/
/*						HOUR PATH					*/
/********************************************************/
void setup_my_path_hour(int hour, int minute, GContext * ctx) {
	
	double test;
	
	gpath_destroy(s_my_path_ptr_hour);
	s_my_path_ptr_hour = gpath_create(&BOLT_PATH_INFO);
	
	if (animation_startup_flag == 1) {
		
		gpath_rotate_to(s_my_path_ptr_hour, animation_startup_variable * 2 + hour *  0x01555 + minute * 0x0005B);
		
		test = (double)(0.104 * 0.000686 * (animation_startup_variable) / 6848);
		
		int move_sin = (double)(test * sin_lookup(animation_startup_variable * 2 + hour *  0x01555 + minute * 0x0005B));		//round
		int move_cos = (double)(test * cos_lookup(animation_startup_variable * 2 + hour *  0x01555 + minute * 0x0005B));		//round
		
		gpath_move_to(s_my_path_ptr_hour, GPoint(72 + move_sin, 84 - move_cos));
		// Fill the path:
		graphics_context_set_fill_color(ctx, back_color);
		gpath_draw_filled(ctx, s_my_path_ptr_hour);
	}
	else {
		gpath_rotate_to(s_my_path_ptr_hour, hour *  0x01555 + minute * 0x0005B);
		
		test = 4.704 / 6848;										// 45 * sin(1) / 6848
		
		int move_sin = (double)(test * sin_lookup(hour *  0x01555 + minute * 0x0005B));		//round
		int move_cos = (double)(test * cos_lookup(hour *  0x01555 + minute * 0x0005B));		//round
		
		gpath_move_to(s_my_path_ptr_hour, GPoint(72 + move_sin, 84 - move_cos));
		// Fill the path:
		graphics_context_set_fill_color(ctx, back_color);
		gpath_draw_filled(ctx, s_my_path_ptr_hour);
	}
	
}
/********************************************************/
/********************************************************/



/********************************************************/
/*						MINUTE PATH						*/
/********************************************************/
void setup_my_path_min(int minute, GContext * ctx) {
	double test;
	int move_sin;
	int move_cos;
	
	gpath_destroy(s_my_path_ptr_min);
	
	if (animation_flag == 1) {
		// Animation routine
		
		
		s_my_path_ptr_min = gpath_create(&BOLT_PATH_INFO);
		
		if (animation_startup_flag == 1) {
			gpath_rotate_to(s_my_path_ptr_min, animation_startup_variable + minute * 0x00444 );
			test = (double)(0.105 * 0.000915 * animation_startup_variable / 6848);
			
			move_sin = (double)(test * sin_lookup(animation_startup_variable + minute * 0x00444 ));		//round
			move_cos = (double)(test * cos_lookup(animation_startup_variable + minute * 0x00444));		//round
		}
		else {
			gpath_rotate_to(s_my_path_ptr_min, animation_variable + minute * 0x00444 );
			test = 6.272 / 6848;				// 45 * sin(1) / 6848
			move_sin = (double)(test * sin_lookup(animation_variable + minute * 0x00444 ));		//round
			move_cos = (double)(test * cos_lookup(animation_variable + minute * 0x00444));		//round
		}
		
		gpath_move_to(s_my_path_ptr_min, GPoint(72 + move_sin, 84 - move_cos));
		// Fill the path:
		graphics_context_set_fill_color(ctx, back_color);
		gpath_draw_filled(ctx, s_my_path_ptr_min);
	}
	else {
		s_my_path_ptr_min = gpath_create(&BOLT_PATH_INFO);
		
		gpath_rotate_to(s_my_path_ptr_min, TRIG_MAX_ANGLE / 360 * minute * 6);
		
		double test = 6.272 / 6848;										// 60 * sin(6) / 6848
		
		int move_sin = (double)(test * sin_lookup(minute * 0x00444));		//round
		int move_cos = (double)(test * cos_lookup(minute * 0x00444));		//round
		
		gpath_move_to(s_my_path_ptr_min, GPoint(72 + move_sin, 84 - move_cos));
		// Fill the path:
		graphics_context_set_fill_color(ctx, back_color);
		gpath_draw_filled(ctx, s_my_path_ptr_min);
	}
}
/********************************************************/
/********************************************************/



/************************************************************/
/*						CIRCLES SETUP						*/
/************************************************************/
static void outter_min_layer_update_callback(Layer *me, GContext *ctx) {
	GPoint p = {72, 84};
	graphics_context_set_fill_color(ctx, circle_color);
	
	if (animation_startup_flag == 1) {
		double test_value = (double)(0.000915 * animation_startup_variable);
#ifdef DEBUG
		//APP_LOG(APP_LOG_LEVEL_INFO, "outter_min_startup_variable = %d, %d",animation_startup_variable, animation_on_startup_flag);
#endif
		graphics_fill_circle(ctx, p, test_value);
	}
	else if (animation_on_startup_flag == 0){
		graphics_fill_circle(ctx, p, 60);
	}
	
#ifdef DEBUG
	int minute = get_time(1);
#else
	int minute = get_time(2);
#endif
	setup_my_path_min(minute, ctx);
	
}

static void inner_min_layer_update_callback(Layer *me, GContext *ctx) {
	GPoint p = {72, 84};
	
	graphics_context_set_fill_color(ctx, back_color);
	
	if (animation_startup_flag == 1) {
		double test_value = (double)(0.000839 * animation_startup_variable);
#ifdef DEBUG
		//APP_LOG(APP_LOG_LEVEL_INFO, "inner_min_startup_variable = %d, %d",animation_startup_variable, animation_on_startup_flag);
#endif
		graphics_fill_circle(ctx, p, test_value);
	}
	else if (animation_on_startup_flag == 0) {
		graphics_fill_circle(ctx, p, 55);
	}
}

static void outter_hour_layer_update_callback(Layer *me, GContext *ctx) {
	GPoint p = {72, 84};
	
	graphics_context_set_fill_color(ctx, circle_color);
	
	if (animation_startup_flag == 1) {
		double test_value = (double)(0.000687 * animation_startup_variable);
		graphics_fill_circle(ctx, p, test_value);
#ifdef DEBUG
		//APP_LOG(APP_LOG_LEVEL_INFO, "outter_hour_startup_variable = %d, %d", (int)test_value, animation_on_startup_flag);
#endif
	}
	else if (animation_on_startup_flag == 0){
		graphics_fill_circle(ctx, p, 45);
	}
	
#ifdef DEBUG
	int minute = get_time(1);
	int hour = get_time(2);
#else
	int minute = get_time(2);
	int hour = get_time(3);
#endif
	
	setup_my_path_hour(hour, minute, ctx);
}

static void inner_hour_layer_update_callback(Layer *me, GContext *ctx) {
	GPoint p = {72, 84};
	
	graphics_context_set_fill_color(ctx, back_color);
	if (animation_startup_flag == 1) {
		
		double test_value = (double)(0.00061 * animation_startup_variable);
		graphics_fill_circle(ctx, p, test_value);
#ifdef DEBUG
		//APP_LOG(APP_LOG_LEVEL_INFO, "inner_hour_startup_variable = %d, %d", (int)test_value, animation_on_startup_flag);
#endif
	}
	else if (animation_on_startup_flag == 0) {
		graphics_fill_circle(ctx, p, 40);
	}
	
}
/************************************************************/
/************************************************************/



/************************************************************/
/*						TICK HANDLER						*/
/************************************************************/
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
//#ifdef DEBUG
	APP_LOG(APP_LOG_LEVEL_INFO, "TEST = %d : %d : %d", t->tm_hour, t->tm_min, t->tm_sec);
//#endif
	
	if (animation_on_startup_flag == 1) {
		trigger_startup_animation(STARTUP_DURATION, 0);
		animation_on_startup_flag = 0;
	}
	
#ifdef PBL_PLATFORM_BASALT
	//trigger_shutdown_animation(ANIMATION_DURATION, 0);
#endif
	//trigger_startup_animation(ANIMATION_DURATION, 0);
	trigger_animation(ANIMATION_DURATION, 0);
	
}

// CONFIGURATION COMMUNICATION
static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
	//Get Tuple
	Tuple *t = dict_read_first(iterator);
	
	while(t)
	{
		switch(t->key)
		{
			case KEY_BACKGROUND_COLOR:
				//It's the KEY_INVERT key
				if(strcmp(t->value->cstring, "blue") == 0)
				{
					#ifdef PBL_COLOR
						back_color = (GColor) GColorBlueMoonARGB8;
						circle_color = (GColor) GColorWhiteARGB8;
					#endif
					persist_write_int(KEY_BACKGROUND_COLOR, 1);
				}
				else if(strcmp(t->value->cstring, "green") == 0)
				{
					#ifdef PBL_COLOR
						back_color = (GColor) GColorJaegerGreenARGB8;
						circle_color = (GColor) GColorWhiteARGB8;
					#endif

					persist_write_int(KEY_BACKGROUND_COLOR, 2);
				}
				
				break;
			case KEY_CIRCLES_COLOR:
				//It's the KEY_BATTERY key
				if(strcmp(t->value->cstring, "on") == 0)
				{

				}
				else if(strcmp(t->value->cstring, "low") == 0)
				{

				}
				
				else if(strcmp(t->value->cstring, "off") == 0)
				{

				}
				break;

		}
		t = dict_read_next(iterator);
	}
}

static void main_window_load(Window *window) {
	// Create time TextLayer
	APP_LOG(APP_LOG_LEVEL_INFO, "Window_load");
	s_time_layer = text_layer_create(GRect(0, 0, 144, 168));
	text_layer_set_background_color(s_time_layer, back_color);
	text_layer_set_text_color(s_time_layer, GColorClear);
	text_layer_set_text(s_time_layer, " ");
	
	// Improve the layout to be more like a watchface
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	// Added by me
	
	outter_min_layer = layer_create(GRect(0, 0, 144, 168));
	layer_add_child(text_layer_get_layer(s_time_layer), outter_min_layer);
	layer_set_update_proc(outter_min_layer, outter_min_layer_update_callback);
	
	layer_add_child(text_layer_get_layer(s_time_layer), outter_min_layer);

	inner_min_layer = layer_create(GRect(0, 0, 144, 168));
	layer_add_child(outter_min_layer, inner_min_layer);
	layer_set_update_proc(inner_min_layer, inner_min_layer_update_callback);
	
	outter_hour_layer = layer_create(GRect(0, 0, 144, 168));
	layer_add_child(inner_min_layer, outter_hour_layer);
	layer_set_update_proc(outter_hour_layer, outter_hour_layer_update_callback);
	
	inner_hour_layer = layer_create(GRect(0, 0, 144, 168));

	layer_add_child(outter_hour_layer, inner_hour_layer);

	layer_set_update_proc(inner_hour_layer, inner_hour_layer_update_callback);
	
}

static void main_window_unload(Window *window) {
	// Destroy TextLayer
	APP_LOG(APP_LOG_LEVEL_INFO, "Window_unload");
	//animation_set_reverse(startup_anim, true);
#ifdef PBL_PLATFORM_BASALT
	APP_LOG(APP_LOG_LEVEL_INFO, "Triggering animation");
	trigger_shutdown_animation(100, 0);
#endif
	APP_LOG(APP_LOG_LEVEL_INFO, "Destroying everything");
	window_stack_remove(s_main_window, true);
	gpath_destroy(s_my_path_ptr_hour);
	gpath_destroy(s_my_path_ptr_min);
	text_layer_destroy(s_time_layer);
	layer_destroy(outter_hour_layer);
	layer_destroy(inner_hour_layer);
	layer_destroy(outter_min_layer);
	layer_destroy(inner_min_layer);

}

static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	
	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
		
	});
	
	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	
	TimeUnits unit;
#ifdef DEBUG
	unit = SECOND_UNIT;
#else
	unit = MINUTE_UNIT;
#endif
	
	tick_timer_service_subscribe(unit, &handle_minute_tick);
}

static void deinit() {
	APP_LOG(APP_LOG_LEVEL_INFO, "Deinit here");
	tick_timer_service_unsubscribe();
	// Destroy Window
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}