#include <pebble.h>

// ── Layout constants (Emery: 200 × 228 px) ───────────────────────────────────
#define SCREEN_W        200
#define SCREEN_H        228
#define HEADER_H         34    // day/date/month bar
#define STEPBAR_H        10    // step progress bar strip
#define FOOTER_H         56    // weather + hr + steps
#define CLOCK_Y          HEADER_H
#define CLOCK_H          (SCREEN_H - HEADER_H - STEPBAR_H - FOOTER_H)  // 128 px
#define STEPBAR_Y        (SCREEN_H - FOOTER_H - STEPBAR_H)
#define FOOTER_Y         (SCREEN_H - FOOTER_H)

// Date box dimensions (blue square behind date number)
#define DATEBOX_W        34
#define DATEBOX_H        28
#define DATEBOX_X        ((SCREEN_W - DATEBOX_W) / 2)
#define DATEBOX_Y         3

// Step goal
#define STEP_GOAL        10000

// ── Colors ────────────────────────────────────────────────────────────────────
#define COL_BG           GColorBlack
#define COL_BLUE         GColorCobaltBlue
#define COL_WHITE        GColorWhite
#define COL_SUNDAY       GColorCobaltBlue
#define COL_WEEKDAY      GColorWhite

// ── Layers ────────────────────────────────────────────────────────────────────
static Window        *s_window;

static Layer         *s_header_layer;
static TextLayer     *s_day_layer;
static TextLayer     *s_date_num_layer;
static TextLayer     *s_month_layer;

static TextLayer     *s_time_layer;

static Layer         *s_stepbar_layer;

static Layer         *s_footer_layer;
static TextLayer     *s_weather_label;
static TextLayer     *s_weather_val;
static TextLayer     *s_hr_label;
static TextLayer     *s_hr_val;
static TextLayer     *s_steps_label;
static TextLayer     *s_steps_val;

// ── Fonts ─────────────────────────────────────────────────────────────────────
static GFont s_font_header;
static GFont s_font_clock;
static GFont s_font_label;
static GFont s_font_value;

// ── State ─────────────────────────────────────────────────────────────────────
static char s_time_buf[8];
static char s_day_buf[4];
static char s_date_buf[3];
static char s_month_buf[4];
static char s_hr_buf[6];
static char s_steps_buf[8];
static char s_weather_buf[8];
static int  s_step_count = 0;

// ── Uppercase helper ──────────────────────────────────────────────────────────
static void to_upper(char *s) {
  for (; *s; s++) {
    if (*s >= 'a' && *s <= 'z') *s -= 32;
  }
}

// ── Header draw callback ──────────────────────────────────────────────────────
static void header_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, COL_BLUE);
  graphics_fill_rect(ctx, GRect(DATEBOX_X, DATEBOX_Y, DATEBOX_W, DATEBOX_H), 0, GCornerNone);
}

// ── Step bar draw callback ────────────────────────────────────────────────────
static void stepbar_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;
  int h = bounds.size.h;
  int cx = w / 2;

  int steps = s_step_count < 0 ? 0 : s_step_count;
  int goal  = STEP_GOAL > 0 ? STEP_GOAL : 1;
  int fill_half = (steps >= goal) ? cx : (steps * cx / goal);

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (steps >= goal) {
    graphics_context_set_fill_color(ctx, COL_BLUE);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  } else {
    if (fill_half > 0) {
      graphics_context_set_fill_color(ctx, COL_WHITE);
      graphics_fill_rect(ctx, GRect(cx - fill_half, 0, fill_half * 2, h), 0, GCornerNone);
    }
    graphics_context_set_stroke_color(ctx, COL_BLUE);
    graphics_context_set_stroke_width(ctx, 1);
    for (int x = 0; x < w; x += 4) {
      graphics_draw_pixel(ctx, GPoint(x,     0));
      graphics_draw_pixel(ctx, GPoint(x + 1, 0));
      graphics_draw_pixel(ctx, GPoint(x,     h - 1));
      graphics_draw_pixel(ctx, GPoint(x + 1, h - 1));
    }
    graphics_draw_line(ctx, GPoint(0, 0),     GPoint(0, h - 1));
    graphics_draw_line(ctx, GPoint(w - 1, 0), GPoint(w - 1, h - 1));
  }
}

// ── Footer draw callback ──────────────────────────────────────────────────────
static void footer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int third = SCREEN_W / 3;  // 66px each zone

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Blue box behind HR (centre zone)
  graphics_context_set_fill_color(ctx, COL_BLUE);
  graphics_fill_rect(ctx, GRect(third, 0, third, bounds.size.h), 0, GCornerNone);

  // Zone dividers
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(third,     0), GPoint(third,     bounds.size.h));
  graphics_draw_line(ctx, GPoint(third * 2, 0), GPoint(third * 2, bounds.size.h));

  // Cloud icon in left zone
  int cx = third / 2, cy = bounds.size.h / 2 + 6;
  graphics_context_set_fill_color(ctx, COL_WHITE);
  graphics_fill_circle(ctx, GPoint(cx - 5, cy + 2), 5);
  graphics_fill_circle(ctx, GPoint(cx + 5, cy + 2), 5);
  graphics_fill_circle(ctx, GPoint(cx,     cy - 3), 6);
  graphics_fill_rect(ctx, GRect(cx - 10, cy + 2, 20, 6), 0, GCornerNone);
}

// ── Time / date update ────────────────────────────────────────────────────────
static void update_time(struct tm *tick_time) {
  strftime(s_time_buf, sizeof(s_time_buf), "%l:%M", tick_time);
  char *t = s_time_buf;
  while (*t == ' ') t++;
  text_layer_set_text(s_time_layer, t);

  strftime(s_day_buf, sizeof(s_day_buf), "%a", tick_time);
  to_upper(s_day_buf);
  text_layer_set_text_color(s_day_layer,
    (tick_time->tm_wday == 0) ? COL_SUNDAY : COL_WEEKDAY);
  text_layer_set_text(s_day_layer, s_day_buf);

  strftime(s_date_buf, sizeof(s_date_buf), "%e", tick_time);
  char *d = s_date_buf;
  while (*d == ' ') d++;
  text_layer_set_text(s_date_num_layer, d);

  strftime(s_month_buf, sizeof(s_month_buf), "%b", tick_time);
  to_upper(s_month_buf);
  text_layer_set_text(s_month_layer, s_month_buf);
}

// ── Tick handler ──────────────────────────────────────────────────────────────
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  update_time(tick_time);
}

// ── Health handler ────────────────────────────────────────────────────────────
#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
    snprintf(s_steps_buf, sizeof(s_steps_buf), "%d", s_step_count);
    text_layer_set_text(s_steps_val, s_steps_buf);
    layer_mark_dirty(s_stepbar_layer);
  }
  if (event == HealthEventHeartRateUpdate || event == HealthEventSignificantUpdate) {
    HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
    if (hr > 0) {
      snprintf(s_hr_buf, sizeof(s_hr_buf), "%d", (int)hr);
      text_layer_set_text(s_hr_val, s_hr_buf);
    }
  }
}
#endif

// ── Window load ───────────────────────────────────────────────────────────────
static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  int third = SCREEN_W / 3;

  s_font_header = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_font_clock  = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  s_font_label  = fonts_get_system_font(FONT_KEY_GOTHIC_09);
  s_font_value  = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);

  // ── HEADER ────────────────────────────────────────────────────────────────
  s_header_layer = layer_create(GRect(0, 0, SCREEN_W, HEADER_H));
  layer_set_update_proc(s_header_layer, header_update_proc);
  layer_add_child(root, s_header_layer);

  s_day_layer = text_layer_create(GRect(0, 4, DATEBOX_X, 26));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, COL_WEEKDAY);
  text_layer_set_font(s_day_layer, s_font_header);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_day_layer));

  s_date_num_layer = text_layer_create(GRect(DATEBOX_X, DATEBOX_Y, DATEBOX_W, DATEBOX_H));
  text_layer_set_background_color(s_date_num_layer, GColorClear);
  text_layer_set_text_color(s_date_num_layer, COL_WHITE);
  text_layer_set_font(s_date_num_layer, s_font_header);
  text_layer_set_text_alignment(s_date_num_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_date_num_layer));

  s_month_layer = text_layer_create(GRect(DATEBOX_X + DATEBOX_W, 4, DATEBOX_X, 26));
  text_layer_set_background_color(s_month_layer, GColorClear);
  text_layer_set_text_color(s_month_layer, COL_WHITE);
  text_layer_set_font(s_month_layer, s_font_header);
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_month_layer));

  // ── CLOCK ─────────────────────────────────────────────────────────────────
  s_time_layer = text_layer_create(GRect(4, CLOCK_Y, SCREEN_W - 8, CLOCK_H));
  text_layer_set_background_color(s_time_layer, COL_BG);
  text_layer_set_text_color(s_time_layer, COL_WHITE);
  text_layer_set_font(s_time_layer, s_font_clock);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  // ── STEP BAR ──────────────────────────────────────────────────────────────
  s_stepbar_layer = layer_create(GRect(0, STEPBAR_Y, SCREEN_W, STEPBAR_H));
  layer_set_update_proc(s_stepbar_layer, stepbar_update_proc);
  layer_add_child(root, s_stepbar_layer);

  // ── FOOTER ────────────────────────────────────────────────────────────────
  s_footer_layer = layer_create(GRect(0, FOOTER_Y, SCREEN_W, FOOTER_H));
  layer_set_update_proc(s_footer_layer, footer_update_proc);
  layer_add_child(root, s_footer_layer);

  // WEATHER label
  s_weather_label = text_layer_create(GRect(0, 2, third, 12));
  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text_color(s_weather_label, COL_WHITE);
  text_layer_set_font(s_weather_label, s_font_label);
  text_layer_set_text_alignment(s_weather_label, GTextAlignmentCenter);
  text_layer_set_text(s_weather_label, "WEATHER");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_label));

  s_weather_val = text_layer_create(GRect(0, 14, third, 38));
  text_layer_set_background_color(s_weather_val, GColorClear);
  text_layer_set_text_color(s_weather_val, COL_WHITE);
  text_layer_set_font(s_weather_val, s_font_value);
  text_layer_set_text_alignment(s_weather_val, GTextAlignmentCenter);
  snprintf(s_weather_buf, sizeof(s_weather_buf), "--");
  text_layer_set_text(s_weather_val, s_weather_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_val));

  // HR label + value (centre, blue bg)
  s_hr_label = text_layer_create(GRect(third, 2, third, 12));
  text_layer_set_background_color(s_hr_label, GColorClear);
  text_layer_set_text_color(s_hr_label, COL_WHITE);
  text_layer_set_font(s_hr_label, s_font_label);
  text_layer_set_text_alignment(s_hr_label, GTextAlignmentCenter);
  text_layer_set_text(s_hr_label, "HR");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_label));

  s_hr_val = text_layer_create(GRect(third, 14, third, 38));
  text_layer_set_background_color(s_hr_val, GColorClear);
  text_layer_set_text_color(s_hr_val, COL_WHITE);
  text_layer_set_font(s_hr_val, s_font_value);
  text_layer_set_text_alignment(s_hr_val, GTextAlignmentCenter);
  snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
  text_layer_set_text(s_hr_val, s_hr_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_val));

  // STEPS label + value
  s_steps_label = text_layer_create(GRect(third * 2, 2, third, 12));
  text_layer_set_background_color(s_steps_label, GColorClear);
  text_layer_set_text_color(s_steps_label, COL_WHITE);
  text_layer_set_font(s_steps_label, s_font_label);
  text_layer_set_text_alignment(s_steps_label, GTextAlignmentCenter);
  text_layer_set_text(s_steps_label, "STEPS");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_label));

  s_steps_val = text_layer_create(GRect(third * 2, 14, third, 38));
  text_layer_set_background_color(s_steps_val, GColorClear);
  text_layer_set_text_color(s_steps_val, COL_WHITE);
  text_layer_set_font(s_steps_val, s_font_value);
  text_layer_set_text_alignment(s_steps_val, GTextAlignmentCenter);
  snprintf(s_steps_buf, sizeof(s_steps_buf), "0");
  text_layer_set_text(s_steps_val, s_steps_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_val));

  // ── Seed initial values ───────────────────────────────────────────────────
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

#if defined(PBL_HEALTH)
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  snprintf(s_steps_buf, sizeof(s_steps_buf), "%d", s_step_count);
  text_layer_set_text(s_steps_val, s_steps_buf);

  HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
  if (hr > 0) {
    snprintf(s_hr_buf, sizeof(s_hr_buf), "%d", (int)hr);
    text_layer_set_text(s_hr_val, s_hr_buf);
  }
#endif
}

// ── Window unload ─────────────────────────────────────────────────────────────
static void window_unload(Window *window) {
  layer_destroy(s_header_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_num_layer);
  text_layer_destroy(s_month_layer);
  text_layer_destroy(s_time_layer);
  layer_destroy(s_stepbar_layer);
  layer_destroy(s_footer_layer);
  text_layer_destroy(s_weather_label);
  text_layer_destroy(s_weather_val);
  text_layer_destroy(s_hr_label);
  text_layer_destroy(s_hr_val);
  text_layer_destroy(s_steps_label);
  text_layer_destroy(s_steps_val);
}

// ── App lifecycle ─────────────────────────────────────────────────────────────
static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, COL_BG);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}