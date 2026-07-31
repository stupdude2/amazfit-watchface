#include <pebble.h>

// ── Layout constants (Basalt: 144 × 168 px) ──────────────────────────────────
#define HEADER_H        26    // day/date/month bar
#define STEPBAR_H        8    // step progress bar strip
#define FOOTER_H        44    // weather + hr + steps
#define CLOCK_Y         HEADER_H
#define CLOCK_H         (168 - HEADER_H - STEPBAR_H - FOOTER_H)  // ~90 px
#define STEPBAR_Y       (168 - FOOTER_H - STEPBAR_H)
#define FOOTER_Y        (168 - FOOTER_H)

// Date box dimensions (blue square behind date number)
#define DATEBOX_W       26
#define DATEBOX_H       22
#define DATEBOX_X       ((144 - DATEBOX_W) / 2)   // centred = 59
#define DATEBOX_Y        2

// Step goal — update to match user's daily goal
#define STEP_GOAL       10000

// ── Colors ────────────────────────────────────────────────────────────────────
#define COL_BG          GColorBlack
#define COL_BLUE        GColorCobaltBlue
#define COL_WHITE       GColorWhite
#define COL_SUNDAY      GColorCobaltBlue   // SUN text is blue on black
#define COL_WEEKDAY     GColorWhite        // other days white on black

// ── Layers ────────────────────────────────────────────────────────────────────
static Window        *s_window;

// Header
static Layer         *s_header_layer;
static TextLayer     *s_day_layer;
static TextLayer     *s_date_num_layer;
static TextLayer     *s_month_layer;

// Clock
static TextLayer     *s_time_layer;

// Step progress bar
static Layer         *s_stepbar_layer;

// Footer
static Layer         *s_footer_layer;
static TextLayer     *s_weather_label;
static TextLayer     *s_weather_val;
static TextLayer     *s_hr_label;
static TextLayer     *s_hr_val;
static TextLayer     *s_steps_label;
static TextLayer     *s_steps_val;

// ── Fonts ─────────────────────────────────────────────────────────────────────
static GFont s_font_header;    // day / month text
static GFont s_font_clock;     // large time digits — LECO 38 (tall, narrow)
static GFont s_font_label;     // tiny WEATHER / HR / STEPS labels
static GFont s_font_value;     // footer values (larger)

// ── State ─────────────────────────────────────────────────────────────────────
static char s_time_buf[8];
static char s_day_buf[4];
static char s_date_buf[3];
static char s_month_buf[4];
static char s_hr_buf[6];
static char s_steps_buf[8];
static char s_weather_buf[8];
static int  s_step_count = 0;
static bool s_is_sunday  = false;

// ── Uppercase helper ──────────────────────────────────────────────────────────
static void to_upper(char *s) {
  for (; *s; s++) {
    if (*s >= 'a' && *s <= 'z') *s -= 32;
  }
}

// ── Header draw callback ──────────────────────────────────────────────────────
// Black background, blue box behind date number only.
static void header_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Black background
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Blue box behind date number
  graphics_context_set_fill_color(ctx, COL_BLUE);
  graphics_fill_rect(ctx, GRect(DATEBOX_X, DATEBOX_Y, DATEBOX_W, DATEBOX_H), 0, GCornerNone);
}

// ── Step bar draw callback ────────────────────────────────────────────────────
// Dotted blue outline bar. White fill grows from center outward.
// When full, the whole bar turns solid blue.
static void stepbar_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;  // 144
  int h = bounds.size.h;  // 8
  int cx = w / 2;

  // Clamp progress
  int steps = s_step_count < 0 ? 0 : s_step_count;
  int goal  = STEP_GOAL > 0 ? STEP_GOAL : 1;
  int fill_half = (steps >= goal) ? cx : (steps * cx / goal);

  // Background black
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (steps >= goal) {
    // Goal reached — full solid blue bar
    graphics_context_set_fill_color(ctx, COL_BLUE);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  } else {
    // White fill growing from center outward
    if (fill_half > 0) {
      graphics_context_set_fill_color(ctx, COL_WHITE);
      graphics_fill_rect(ctx, GRect(cx - fill_half, 0, fill_half * 2, h), 0, GCornerNone);
    }

    // Dotted blue border — draw dots along top and bottom edges
    graphics_context_set_stroke_color(ctx, COL_BLUE);
    graphics_context_set_stroke_width(ctx, 1);
    for (int x = 0; x < w; x += 4) {
      // Top edge dot
      graphics_draw_pixel(ctx, GPoint(x,     0));
      graphics_draw_pixel(ctx, GPoint(x + 1, 0));
      // Bottom edge dot
      graphics_draw_pixel(ctx, GPoint(x,     h - 1));
      graphics_draw_pixel(ctx, GPoint(x + 1, h - 1));
    }
    // Left and right end caps (solid blue 1px)
    graphics_draw_line(ctx, GPoint(0, 0), GPoint(0, h - 1));
    graphics_draw_line(ctx, GPoint(w - 1, 0), GPoint(w - 1, h - 1));
  }
}

// ── Footer draw callback ──────────────────────────────────────────────────────
static void footer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Black background
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Blue box behind HR value — centred third, matches date box color
  // HR zone: x=48 to x=96 (48px wide)
  graphics_context_set_fill_color(ctx, COL_BLUE);
  graphics_fill_rect(ctx, GRect(48, 0, 48, bounds.size.h), 0, GCornerNone);

  // Vertical dividers between zones (subtle — 1px dark lines)
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(48, 0), GPoint(48, bounds.size.h));
  graphics_draw_line(ctx, GPoint(96, 0), GPoint(96, bounds.size.h));

  // Weather icon (cloud) — drawn in left zone
  int cx = 24, cy = bounds.size.h / 2 + 4;
  graphics_context_set_fill_color(ctx, COL_WHITE);
  graphics_fill_circle(ctx, GPoint(cx - 4, cy + 1), 4);
  graphics_fill_circle(ctx, GPoint(cx + 4, cy + 1), 4);
  graphics_fill_circle(ctx, GPoint(cx,     cy - 2), 5);
  graphics_fill_rect(ctx, GRect(cx - 8, cy + 1, 16, 5), 0, GCornerNone);
}

// ── Time / date update ────────────────────────────────────────────────────────
static void update_time(struct tm *tick_time) {
  // 12-hour, no leading zero
  strftime(s_time_buf, sizeof(s_time_buf), "%l:%M", tick_time);
  // %l can produce a leading space — trim it
  char *t = s_time_buf;
  while (*t == ' ') t++;
  text_layer_set_text(s_time_layer, t);

  // Day of week
  strftime(s_day_buf, sizeof(s_day_buf), "%a", tick_time);
  to_upper(s_day_buf);

  // Sunday = blue text, all others = white
  s_is_sunday = (tick_time->tm_wday == 0);
  text_layer_set_text_color(s_day_layer, s_is_sunday ? COL_SUNDAY : COL_WEEKDAY);
  text_layer_set_text(s_day_layer, s_day_buf);

  // Date number
  strftime(s_date_buf, sizeof(s_date_buf), "%e", tick_time);
  char *d = s_date_buf;
  while (*d == ' ') d++;
  text_layer_set_text(s_date_num_layer, d);

  // Month
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

  // Fonts
  s_font_header = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_font_clock  = fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS);
  s_font_label  = fonts_get_system_font(FONT_KEY_GOTHIC_09);
  s_font_value  = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);

  // ── HEADER (black bg, blue date box) ─────────────────────────────────────
  s_header_layer = layer_create(GRect(0, 0, 144, HEADER_H));
  layer_set_update_proc(s_header_layer, header_update_proc);
  layer_add_child(root, s_header_layer);

  // Day of week — left zone, text color set dynamically in update_time
  s_day_layer = text_layer_create(GRect(0, 3, 54, 20));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, COL_WEEKDAY);
  text_layer_set_font(s_day_layer, s_font_header);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_day_layer));

  // Date number — white text on blue box, centred
  s_date_num_layer = text_layer_create(GRect(DATEBOX_X, DATEBOX_Y, DATEBOX_W, DATEBOX_H));
  text_layer_set_background_color(s_date_num_layer, GColorClear);
  text_layer_set_text_color(s_date_num_layer, COL_WHITE);
  text_layer_set_font(s_date_num_layer, s_font_header);
  text_layer_set_text_alignment(s_date_num_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_date_num_layer));

  // Month — right zone
  s_month_layer = text_layer_create(GRect(90, 3, 54, 20));
  text_layer_set_background_color(s_month_layer, GColorClear);
  text_layer_set_text_color(s_month_layer, COL_WHITE);
  text_layer_set_font(s_month_layer, s_font_header);
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_month_layer));

  // ── CLOCK ─────────────────────────────────────────────────────────────────
  s_time_layer = text_layer_create(GRect(0, CLOCK_Y, 144, CLOCK_H));
  text_layer_set_background_color(s_time_layer, COL_BG);
  text_layer_set_text_color(s_time_layer, COL_WHITE);
  text_layer_set_font(s_time_layer, s_font_clock);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  // ── STEP BAR ──────────────────────────────────────────────────────────────
  s_stepbar_layer = layer_create(GRect(0, STEPBAR_Y, 144, STEPBAR_H));
  layer_set_update_proc(s_stepbar_layer, stepbar_update_proc);
  layer_add_child(root, s_stepbar_layer);

  // ── FOOTER ────────────────────────────────────────────────────────────────
  s_footer_layer = layer_create(GRect(0, FOOTER_Y, 144, FOOTER_H));
  layer_set_update_proc(s_footer_layer, footer_update_proc);
  layer_add_child(root, s_footer_layer);

  // WEATHER label (tiny, top of left zone)
  s_weather_label = text_layer_create(GRect(0, 2, 48, 11));
  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text_color(s_weather_label, COL_WHITE);
  text_layer_set_font(s_weather_label, s_font_label);
  text_layer_set_text_alignment(s_weather_label, GTextAlignmentCenter);
  text_layer_set_text(s_weather_label, "WEATHER");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_label));

  // Weather value (temperature placeholder)
  s_weather_val = text_layer_create(GRect(0, 12, 48, 28));
  text_layer_set_background_color(s_weather_val, GColorClear);
  text_layer_set_text_color(s_weather_val, COL_WHITE);
  text_layer_set_font(s_weather_val, s_font_value);
  text_layer_set_text_alignment(s_weather_val, GTextAlignmentCenter);
  snprintf(s_weather_buf, sizeof(s_weather_buf), "--");
  text_layer_set_text(s_weather_val, s_weather_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_val));

  // HR label (tiny, centre zone — on blue bg)
  s_hr_label = text_layer_create(GRect(48, 2, 48, 11));
  text_layer_set_background_color(s_hr_label, GColorClear);
  text_layer_set_text_color(s_hr_label, COL_WHITE);
  text_layer_set_font(s_hr_label, s_font_label);
  text_layer_set_text_alignment(s_hr_label, GTextAlignmentCenter);
  text_layer_set_text(s_hr_label, "HR");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_label));

  // HR value
  s_hr_val = text_layer_create(GRect(48, 12, 48, 28));
  text_layer_set_background_color(s_hr_val, GColorClear);
  text_layer_set_text_color(s_hr_val, COL_WHITE);
  text_layer_set_font(s_hr_val, s_font_value);
  text_layer_set_text_alignment(s_hr_val, GTextAlignmentCenter);
  snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
  text_layer_set_text(s_hr_val, s_hr_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_val));

  // STEPS label (tiny, right zone)
  s_steps_label = text_layer_create(GRect(96, 2, 48, 11));
  text_layer_set_background_color(s_steps_label, GColorClear);
  text_layer_set_text_color(s_steps_label, COL_WHITE);
  text_layer_set_font(s_steps_label, s_font_label);
  text_layer_set_text_alignment(s_steps_label, GTextAlignmentCenter);
  text_layer_set_text(s_steps_label, "STEPS");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_label));

  // STEPS value
  s_steps_val = text_layer_create(GRect(96, 12, 48, 28));
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
