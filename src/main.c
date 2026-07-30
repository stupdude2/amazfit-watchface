#include <pebble.h>

// ── Layout constants (Basalt: 144 × 168 px) ──────────────────────────────────
#define HEADER_H       28
#define FOOTER_H       32
#define CLOCK_Y        (HEADER_H)
#define CLOCK_H        (168 - HEADER_H - FOOTER_H)  // 108 px

// ── Colors ────────────────────────────────────────────────────────────────────
#define COL_BG         GColorBlack
#define COL_HEADER_BG  GColorCobaltBlue        // royal blue header
#define COL_HEADER_FG  GColorWhite
#define COL_CLOCK_FG   GColorWhite
#define COL_FOOTER_BG  GColorBlack
#define COL_FOOTER_FG  GColorWhite
#define COL_ACCENT     GColorCobaltBlue        // blue highlights in footer

// ── Layers ────────────────────────────────────────────────────────────────────
static Window        *s_window;

// Header
static Layer         *s_header_layer;
static TextLayer     *s_day_layer;       // "SUN"
static TextLayer     *s_date_num_layer;  // "7"  (white box)
static TextLayer     *s_month_layer;     // "JAN"

// Clock
static TextLayer     *s_time_layer;      // "0:00"

// Footer
static Layer         *s_footer_layer;
static TextLayer     *s_weather_label;   // "WEATHER"
static TextLayer     *s_hr_label;        // "HR"
static TextLayer     *s_hr_val;          // "62"
static TextLayer     *s_steps_label;     // "STEPS"
static TextLayer     *s_steps_val;       // "1000"

// ── Fonts ─────────────────────────────────────────────────────────────────────
static GFont s_font_bold_18;   // header text
static GFont s_font_clock;     // large clock digits
static GFont s_font_label_14;  // footer labels
static GFont s_font_value_18;  // footer values

// ── State buffers ─────────────────────────────────────────────────────────────
static char s_time_buf[8];
static char s_day_buf[4];
static char s_date_buf[3];
static char s_month_buf[4];
static char s_hr_buf[6];
static char s_steps_buf[8];

// ── Header draw callback ──────────────────────────────────────────────────────
static void header_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Full blue background
  graphics_context_set_fill_color(ctx, COL_HEADER_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // White box around the date number — 24×24, vertically centred
  int box_w = 24, box_h = 24;
  int box_x = (bounds.size.w - box_w) / 2;
  int box_y = (bounds.size.h - box_h) / 2;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(box_x, box_y, box_w, box_h), 0, GCornerNone);
}

// ── Footer draw callback ──────────────────────────────────────────────────────
static void footer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Black background
  graphics_context_set_fill_color(ctx, COL_FOOTER_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Thin blue separator line at top of footer
  graphics_context_set_stroke_color(ctx, COL_ACCENT);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, 0), GPoint(bounds.size.w, 0));

  // Blue circle for weather icon (placeholder — replace with bitmap if desired)
  int cx = 18, cy = bounds.size.h / 2;
  graphics_context_set_fill_color(ctx, COL_ACCENT);
  graphics_fill_circle(ctx, GPoint(cx, cy), 8);

  // Cloud silhouette drawn simply in white
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(cx - 3, cy + 1), 4);
  graphics_fill_circle(ctx, GPoint(cx + 3, cy + 1), 4);
  graphics_fill_circle(ctx, GPoint(cx,     cy - 2), 5);
  graphics_fill_rect(ctx, GRect(cx - 7, cy + 1, 14, 5), 0, GCornerNone);
}

// ── Time / date update ────────────────────────────────────────────────────────
static void update_time(struct tm *tick_time) {
  // Clock — no leading zero on hour (matches design "0:00" style)
  strftime(s_time_buf,  sizeof(s_time_buf),  "%k:%M", tick_time);
  // Trim leading space that %k can produce
  char *t = s_time_buf;
  while (*t == ' ') t++;
  text_layer_set_text(s_time_layer, t);

  // Header date fields
  strftime(s_day_buf,   sizeof(s_day_buf),   "%a", tick_time);
  // Uppercase
  for (int i = 0; s_day_buf[i]; i++) s_day_buf[i] ^= 0x20 * (s_day_buf[i] >= 'a' && s_day_buf[i] <= 'z');
  strftime(s_date_buf,  sizeof(s_date_buf),  "%e", tick_time);
  // Trim
  char *d = s_date_buf;
  while (*d == ' ') d++;
  strftime(s_month_buf, sizeof(s_month_buf), "%b", tick_time);
  for (int i = 0; s_month_buf[i]; i++) s_month_buf[i] ^= 0x20 * (s_month_buf[i] >= 'a' && s_month_buf[i] <= 'z');

  text_layer_set_text(s_day_layer,      s_day_buf);
  text_layer_set_text(s_date_num_layer, d);
  text_layer_set_text(s_month_layer,    s_month_buf);
}

// ── Tick handler ──────────────────────────────────────────────────────────────
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  update_time(tick_time);
}

// ── Health / steps ────────────────────────────────────────────────────────────
#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    HealthValue steps = health_service_sum_today(HealthMetricStepCount);
    snprintf(s_steps_buf, sizeof(s_steps_buf), "%d", (int)steps);
    text_layer_set_text(s_steps_val, s_steps_buf);
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
  GRect full  = layer_get_bounds(root);

  // Fonts
  s_font_bold_18  = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_font_clock    = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_font_label_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_font_value_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  // ── HEADER ────────────────────────────────────────────────────────────────
  s_header_layer = layer_create(GRect(0, 0, full.size.w, HEADER_H));
  layer_set_update_proc(s_header_layer, header_update_proc);
  layer_add_child(root, s_header_layer);

  // "SUN" — left third of header
  s_day_layer = text_layer_create(GRect(0, 4, 48, 20));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, COL_HEADER_FG);
  text_layer_set_font(s_day_layer, s_font_bold_18);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_day_layer));

  // Date number centred over the white box — 24 px wide box starts at (60, 2)
  s_date_num_layer = text_layer_create(GRect(60, 3, 24, 22));
  text_layer_set_background_color(s_date_num_layer, GColorClear);
  text_layer_set_text_color(s_date_num_layer, COL_HEADER_BG);  // blue text on white box
  text_layer_set_font(s_date_num_layer, s_font_bold_18);
  text_layer_set_text_alignment(s_date_num_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_date_num_layer));

  // "JAN" — right third
  s_month_layer = text_layer_create(GRect(96, 4, 48, 20));
  text_layer_set_background_color(s_month_layer, GColorClear);
  text_layer_set_text_color(s_month_layer, COL_HEADER_FG);
  text_layer_set_font(s_month_layer, s_font_bold_18);
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_month_layer));

  // ── CLOCK ─────────────────────────────────────────────────────────────────
  s_time_layer = text_layer_create(GRect(0, CLOCK_Y, full.size.w, CLOCK_H));
  text_layer_set_background_color(s_time_layer, COL_BG);
  text_layer_set_text_color(s_time_layer, COL_CLOCK_FG);
  text_layer_set_font(s_time_layer, s_font_clock);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  // ── FOOTER ────────────────────────────────────────────────────────────────
  int footer_y = 168 - FOOTER_H;
  s_footer_layer = layer_create(GRect(0, footer_y, full.size.w, FOOTER_H));
  layer_set_update_proc(s_footer_layer, footer_update_proc);
  layer_add_child(root, s_footer_layer);

  // "WEATHER" label — left zone (x=0, after icon at x≈36)
  s_weather_label = text_layer_create(GRect(2, 2, 40, 14));
  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text_color(s_weather_label, COL_FOOTER_FG);
  text_layer_set_font(s_weather_label, s_font_label_14);
  text_layer_set_text(s_weather_label, "WEATHER");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_label));

  // HR label
  s_hr_label = text_layer_create(GRect(52, 2, 26, 14));
  text_layer_set_background_color(s_hr_label, GColorClear);
  text_layer_set_text_color(s_hr_label, COL_FOOTER_FG);
  text_layer_set_font(s_hr_label, s_font_label_14);
  text_layer_set_text(s_hr_label, "HR");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_label));

  // HR value
  s_hr_val = text_layer_create(GRect(48, 14, 36, 18));
  text_layer_set_background_color(s_hr_val, GColorClear);
  text_layer_set_text_color(s_hr_val, COL_FOOTER_FG);
  text_layer_set_font(s_hr_val, s_font_value_18);
  text_layer_set_text_alignment(s_hr_val, GTextAlignmentCenter);
  snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
  text_layer_set_text(s_hr_val, s_hr_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_val));

  // STEPS label
  s_steps_label = text_layer_create(GRect(90, 2, 50, 14));
  text_layer_set_background_color(s_steps_label, GColorClear);
  text_layer_set_text_color(s_steps_label, COL_FOOTER_FG);
  text_layer_set_font(s_steps_label, s_font_label_14);
  text_layer_set_text(s_steps_label, "STEPS");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_label));

  // STEPS value
  s_steps_val = text_layer_create(GRect(88, 14, 54, 18));
  text_layer_set_background_color(s_steps_val, GColorClear);
  text_layer_set_text_color(s_steps_val, COL_FOOTER_FG);
  text_layer_set_font(s_steps_val, s_font_value_18);
  text_layer_set_text_alignment(s_steps_val, GTextAlignmentCenter);
  snprintf(s_steps_buf, sizeof(s_steps_buf), "0");
  text_layer_set_text(s_steps_val, s_steps_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_val));

  // ── Initial values ────────────────────────────────────────────────────────
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

#if defined(PBL_HEALTH)
  // Seed health data immediately
  HealthValue steps = health_service_sum_today(HealthMetricStepCount);
  snprintf(s_steps_buf, sizeof(s_steps_buf), "%d", (int)steps);
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
  layer_destroy(s_footer_layer);
  text_layer_destroy(s_weather_label);
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
