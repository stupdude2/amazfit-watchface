#include <pebble.h>
#include <stdlib.h>
#include "edition.h"

// ── Screen ────────────────────────────────────────────────────────────────────
#define SCREEN_W        200
#define SCREEN_H        228

// ── Zone heights ──────────────────────────────────────────────────────────────
#define HEADER_H         52
#define STEPBAR_H        12
#define FOOTER_H         46
#define CLOCK_Y          HEADER_H
#define CLOCK_H          (SCREEN_H - HEADER_H - STEPBAR_H - FOOTER_H)
#define STEPBAR_Y        (SCREEN_H - FOOTER_H - STEPBAR_H)
#define FOOTER_Y         (SCREEN_H - FOOTER_H)

// ── Footer thirds ─────────────────────────────────────────────────────────────
#define THIRD            (SCREEN_W / 3)

// ── Blue boxes ────────────────────────────────────────────────────────────────
#define BOX_W            50
#define BOX_GAP           3
#define DATEBOX_W        BOX_W
#define DATEBOX_Y         0    // extends to top of screen
#define DATEBOX_H        (HEADER_H - BOX_GAP - 2)
#define DATEBOX_X        ((SCREEN_W - DATEBOX_W) / 2)
#define HRBOX_X          (THIRD + (THIRD - BOX_W) / 2)
#define HRBOX_Y           0
#define UNDERLINE_GAP    BOX_GAP

// ── Step bar ──────────────────────────────────────────────────────────────────
#define STEP_GOAL        5000
#define BAR_MARGIN        8

// ── Digit drawing ─────────────────────────────────────────────────────────────
#define DIGIT_WIDTH      46
#define STK              15
#define DIGIT_MARGIN      6
#define COLON_WIDTH      9
#define COLON_MARGIN     DIGIT_MARGIN
#define ONE_X_OFFSET      8
#define COLON_DOT         9
#define DIGIT_HEIGHT    110
#define H1_ONE_X          1
#define H1_WIDTH         (H1_ONE_X + STK)
#define HALF_V           (DIGIT_HEIGHT / 2)

#define CLOCK_TOTAL_W    (H1_WIDTH + DIGIT_MARGIN + DIGIT_WIDTH + COLON_MARGIN + COLON_WIDTH + COLON_MARGIN + DIGIT_WIDTH + DIGIT_MARGIN + DIGIT_WIDTH)
#define CLOCK_ORIGIN_X   ((SCREEN_W - CLOCK_TOTAL_W) / 2)
#define H1_X             (CLOCK_ORIGIN_X)
#define H2_X             (H1_X + H1_WIDTH + DIGIT_MARGIN)
#define COL_X            (H2_X + DIGIT_WIDTH + COLON_MARGIN)
#define M1_X             (COL_X + COLON_WIDTH + COLON_MARGIN)
#define M2_X             (M1_X + DIGIT_WIDTH + DIGIT_MARGIN)

// ── AppMessage keys ───────────────────────────────────────────────────────────
#define KEY_TEMPERATURE  0
#define KEY_WEATHER_ICON 1
#define KEY_ACCENT_COLOR 2
#define KEY_CONFIG_ACK   3

// ── Persistent settings ──────────────────────────────────────────────────────
#define SETTINGS_PERSIST_KEY 1
#define SETTINGS_VERSION     3

typedef struct {
  uint8_t version;
  GColor accent_color;
} WatchfaceSettings;

static WatchfaceSettings s_settings;

static void settings_set_defaults(void) {
  s_settings.version = SETTINGS_VERSION;
  s_settings.accent_color = GColorCobaltBlue;
}

static void settings_load(void) {
  settings_set_defaults();
#if WATCHFACE_PRO
  if (persist_exists(SETTINGS_PERSIST_KEY) &&
      persist_get_size(SETTINGS_PERSIST_KEY) == (int)sizeof(s_settings)) {
    WatchfaceSettings stored;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &stored, sizeof(stored)) == (int)sizeof(stored) &&
        stored.version == SETTINGS_VERSION) {
      s_settings = stored;
    }
  }
#endif
}

static void settings_save(void) {
#if WATCHFACE_PRO
  persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
#endif
}

// ── Segment bitmasks ──────────────────────────────────────────────────────────
#define SEG_TOP    (1<<0)
#define SEG_TL     (1<<1)
#define SEG_TR     (1<<2)
#define SEG_MID    (1<<3)
#define SEG_BL     (1<<4)
#define SEG_BR     (1<<5)
#define SEG_BOT    (1<<6)

static const uint8_t DIGIT_SEGS[10] = {
  SEG_TOP|SEG_TL|SEG_TR|SEG_BL|SEG_BR|SEG_BOT,
  SEG_TR|SEG_BR,
  SEG_TOP|SEG_TR|SEG_MID|SEG_BL|SEG_BOT,
  SEG_TOP|SEG_TR|SEG_MID|SEG_BR|SEG_BOT,
  SEG_TL|SEG_TR|SEG_MID|SEG_BR,
  SEG_TOP|SEG_TL|SEG_MID|SEG_BR|SEG_BOT,
  SEG_TOP|SEG_TL|SEG_MID|SEG_BL|SEG_BR|SEG_BOT,
  SEG_TOP|SEG_TR|SEG_BR,
  SEG_TOP|SEG_TL|SEG_TR|SEG_MID|SEG_BL|SEG_BR|SEG_BOT,
  SEG_TOP|SEG_TL|SEG_TR|SEG_MID|SEG_BR|SEG_BOT,
};

// ── Colors ────────────────────────────────────────────────────────────────────
#define COL_BG      GColorBlack
#define COL_WHITE   GColorWhite
#define COL_WEEKDAY GColorWhite

// ── Layers ────────────────────────────────────────────────────────────────────
static Window    *s_window;
static Layer     *s_header_layer;
static TextLayer *s_day_layer;
static TextLayer *s_date_num_layer;
static TextLayer *s_month_layer;
static Layer     *s_clock_layer;
static Layer     *s_stepbar_layer;
static Layer     *s_footer_layer;
static TextLayer *s_weather_label;
static TextLayer *s_weather_val;
static BitmapLayer *s_weather_icon_layer;
static GBitmap     *s_weather_icon_bitmap;
static TextLayer *s_hr_label;
static TextLayer *s_hr_val;
static TextLayer *s_steps_label;
static TextLayer *s_steps_val;

// ── Fonts ─────────────────────────────────────────────────────────────────────
static GFont s_font_header;
static GFont s_font_label;
static GFont s_font_value;

// ── State ─────────────────────────────────────────────────────────────────────
static char s_day_buf[4];
static char s_date_buf[3];
static char s_month_buf[4];
static char s_hr_buf[12];
static char s_steps_buf[8];
static char s_weather_buf[12];
static int  s_step_count  = 0;
static int  s_hour        = 0;
static int  s_minute      = 0;
static int  s_weather_icon = -1;

static void to_upper(char *s) {
  for (; *s; s++) if (*s >= 'a' && *s <= 'z') *s -= 32;
}

// ── Segment drawing ───────────────────────────────────────────────────────────
static void draw_h(GContext *ctx, int ox, int oy) {
  graphics_fill_rect(ctx, GRect(ox, oy, DIGIT_WIDTH, STK), 0, GCornerNone);
}
static void draw_v(GContext *ctx, int ox, int oy, int len) {
  graphics_fill_rect(ctx, GRect(ox, oy, STK, len), 0, GCornerNone);
}
static void draw_digit(GContext *ctx, int ox, int oy, int digit) {
  if (digit < 0 || digit > 9) return;
  uint8_t s = DIGIT_SEGS[digit];
  graphics_context_set_fill_color(ctx, COL_WHITE);
  int top_y = oy;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  int lx    = ox;
  int rx    = ox + DIGIT_WIDTH - STK;
  if (s & SEG_TOP) draw_h(ctx, ox, top_y);
  if (s & SEG_MID) draw_h(ctx, ox, mid_y);
  if (s & SEG_BOT) draw_h(ctx, ox, bot_y);
  if (s & SEG_TL) draw_v(ctx, lx, top_y, mid_y - top_y + STK);
  if (s & SEG_TR) draw_v(ctx, rx, top_y, mid_y - top_y + STK);
  if (s & SEG_BL) draw_v(ctx, lx, mid_y, bot_y - mid_y + STK);
  if (s & SEG_BR) draw_v(ctx, rx, mid_y, bot_y - mid_y + STK);
}
static void draw_one_h1(GContext *ctx, int oy) {
  graphics_context_set_fill_color(ctx, COL_WHITE);
  int ox    = H1_X + H1_ONE_X;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  draw_v(ctx, ox, oy,    mid_y - oy + STK);
  draw_v(ctx, ox, mid_y, bot_y - mid_y + STK);
}
static void draw_one(GContext *ctx, int cell_x, int oy) {
  graphics_context_set_fill_color(ctx, COL_WHITE);
  int ox    = cell_x + ONE_X_OFFSET;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  draw_v(ctx, ox, oy,    mid_y - oy + STK);
  draw_v(ctx, ox, mid_y, bot_y - mid_y + STK);
}
static void draw_colon(GContext *ctx, int ox, int oy) {
  graphics_context_set_fill_color(ctx, COL_WHITE);
  int cx      = ox + (COLON_WIDTH - COLON_DOT) / 2;
  int upper_y = oy + DIGIT_HEIGHT / 3 - COLON_DOT / 2;
  int lower_y = oy + (DIGIT_HEIGHT * 2) / 3 - COLON_DOT / 2;
  graphics_fill_rect(ctx, GRect(cx, upper_y, COLON_DOT, COLON_DOT), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(cx, lower_y, COLON_DOT, COLON_DOT), 0, GCornerNone);
}

// ── Weather icon ──────────────────────────────────────────────────────────────
static void update_weather_icon(int icon_code) {
  if (s_weather_icon_bitmap) {
    gbitmap_destroy(s_weather_icon_bitmap);
    s_weather_icon_bitmap = NULL;
  }
  uint32_t resource_id;
  switch (icon_code) {
    case 0:  resource_id = RESOURCE_ID_IMAGE_ICON_CLEAR;   break;
    case 1:  resource_id = RESOURCE_ID_IMAGE_ICON_CLOUDY;  break;
    case 2:  resource_id = RESOURCE_ID_IMAGE_ICON_RAIN;    break;
    case 3:  resource_id = RESOURCE_ID_IMAGE_ICON_SNOW;    break;
    case 4:  resource_id = RESOURCE_ID_IMAGE_ICON_THUNDER; break;
    default: resource_id = RESOURCE_ID_IMAGE_ICON_NA;      break;
  }
  s_weather_icon_bitmap = gbitmap_create_with_resource(resource_id);
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
}

// ── Clock ─────────────────────────────────────────────────────────────────────
static void clock_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  int h1 = s_hour / 10;
  int h2 = s_hour % 10;
  int m1 = s_minute / 10;
  int m2 = s_minute % 10;
  int sy = (b.size.h - DIGIT_HEIGHT) / 2;
  if (h1 == 1) draw_one_h1(ctx, sy);
  if (h2 == 1) draw_one(ctx, H2_X, sy); else draw_digit(ctx, H2_X, sy, h2);
  draw_colon(ctx, COL_X, sy);
  if (m1 == 1) draw_one(ctx, M1_X, sy); else draw_digit(ctx, M1_X, sy, m1);
  if (m2 == 1) draw_one(ctx, M2_X, sy); else draw_digit(ctx, M2_X, sy, m2);
}

// ── Header ────────────────────────────────────────────────────────────────────
// Datebox starts at y=0 (top of screen) and extends to the underline row.
// The header layer itself starts at y=0 so DATEBOX_Y=0 reaches the screen top.
static void header_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(DATEBOX_X, DATEBOX_Y, DATEBOX_W, DATEBOX_H), 0, GCornerNone);
  int line_y = DATEBOX_Y + DATEBOX_H - 2;
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(0, line_y, DATEBOX_X - UNDERLINE_GAP, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(DATEBOX_X + DATEBOX_W + UNDERLINE_GAP, line_y, SCREEN_W - DATEBOX_X - DATEBOX_W - UNDERLINE_GAP, 2), 0, GCornerNone);
}

// ── Step bar ──────────────────────────────────────────────────────────────────
static void stepbar_update_proc(Layer *layer, GContext *ctx) {
  GRect b   = layer_get_bounds(layer);
  int bar_w = b.size.w - BAR_MARGIN * 2;
  int bar_x = BAR_MARGIN;
  int cy    = b.size.h / 2;
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  int steps     = s_step_count < 0 ? 0 : s_step_count;
  int fill_half = (steps >= STEP_GOAL) ? bar_w / 2 : (steps * (bar_w / 2) / STEP_GOAL);
  int bar_cx    = bar_x + bar_w / 2;
  if (steps >= STEP_GOAL) {
    graphics_context_set_fill_color(ctx, COL_WHITE);
    graphics_fill_rect(ctx, GRect(bar_x, cy - 1, bar_w, 4), 0, GCornerNone);
  } else {
    graphics_context_set_stroke_color(ctx, s_settings.accent_color);
    graphics_context_set_stroke_width(ctx, 2);
    for (int x = bar_x; x < bar_x + bar_w; x += 4) {
      graphics_draw_pixel(ctx, GPoint(x,     cy));
      graphics_draw_pixel(ctx, GPoint(x + 1, cy));
    }
    if (fill_half > 0) {
      graphics_context_set_fill_color(ctx, COL_WHITE);
      graphics_fill_rect(ctx, GRect(bar_cx - fill_half, cy - 1, fill_half * 2, 4), 0, GCornerNone);
    }
  }
}

// ── Footer ────────────────────────────────────────────────────────────────────
// HR box fills from y=0 to bottom of screen (footer layer extends to screen bottom).
static void footer_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(HRBOX_X, HRBOX_Y, BOX_W, b.size.h), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(0, 0, HRBOX_X - BOX_GAP, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(HRBOX_X + BOX_W + BOX_GAP, 0, SCREEN_W - HRBOX_X - BOX_W - BOX_GAP, 2), 0, GCornerNone);
}

static void update_time(struct tm *tick_time);

// ── Accent color helper ─────────────────────────────────────────────────────
static void update_accent_text_contrast(void) {
  GColor text_color = gcolor_legible_over(s_settings.accent_color);

  if (s_date_num_layer) {
    text_layer_set_text_color(s_date_num_layer, text_color);
  }
  if (s_hr_label) {
    text_layer_set_text_color(s_hr_label, text_color);
  }
  if (s_hr_val) {
    text_layer_set_text_color(s_hr_val, text_color);
  }
}

static void apply_accent_color(GColor color, bool persist_setting) {
  s_settings.accent_color = color;
  update_accent_text_contrast();
  if (persist_setting) settings_save();

  if (s_header_layer) layer_mark_dirty(s_header_layer);
  if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
  if (s_footer_layer) layer_mark_dirty(s_footer_layer);

  time_t now = time(NULL);
  struct tm *current = localtime(&now);
  if (current) update_time(current);
}

// ── AppMessage ────────────────────────────────────────────────────────────────
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *temperature_t = dict_find(iter, KEY_TEMPERATURE);
  if (temperature_t) {
    int temp = (int)temperature_t->value->int32;
    snprintf(s_weather_buf, sizeof(s_weather_buf), "%d\u00B0", temp);
    text_layer_set_text(s_weather_val, s_weather_buf);
  }

  Tuple *weather_icon_t = dict_find(iter, KEY_WEATHER_ICON);
  if (weather_icon_t) {
    s_weather_icon = (int)weather_icon_t->value->int32;
    update_weather_icon(s_weather_icon);
  }

#if WATCHFACE_PRO
  // Clay color controls arrive as a 32-bit RGB integer. This is the
  // documented Pebble/Clay path: tuple -> GColorFromHEX() -> persist/redraw.
  Tuple *accent_color_t = dict_find(iter, KEY_ACCENT_COLOR);
  if (accent_color_t) {
    uint32_t accent_hex = (uint32_t)accent_color_t->value->int32 & 0xFFFFFF;
    apply_accent_color(GColorFromHEX(accent_hex), true);
    APP_LOG(APP_LOG_LEVEL_INFO, "Accent color applied: 0x%06lX",
            (unsigned long)accent_hex);

    DictionaryIterator *out = NULL;
    if (app_message_outbox_begin(&out) == APP_MSG_OK && out) {
      dict_write_int32(out, KEY_CONFIG_ACK, (int32_t)accent_hex);
      app_message_outbox_send();
    }
  }
#endif
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage inbox dropped: %d", (int)reason);
}

// ── Time / date update ────────────────────────────────────────────────────────
static void update_time(struct tm *tick_time) {
  s_hour   = tick_time->tm_hour % 12;
  if (s_hour == 0) s_hour = 12;
  s_minute = tick_time->tm_min;
  layer_mark_dirty(s_clock_layer);

  strftime(s_day_buf,   sizeof(s_day_buf),   "%a", tick_time); to_upper(s_day_buf);
  strftime(s_date_buf,  sizeof(s_date_buf),  "%e", tick_time);
  strftime(s_month_buf, sizeof(s_month_buf), "%b", tick_time); to_upper(s_month_buf);

  text_layer_set_text_color(s_day_layer, tick_time->tm_wday == 0 ? s_settings.accent_color : COL_WEEKDAY);
  text_layer_set_text(s_day_layer, s_day_buf);
  char *d = s_date_buf; while (*d == ' ') d++;
  text_layer_set_text(s_date_num_layer, d);
  text_layer_set_text(s_month_layer, s_month_buf);

  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) { update_time(tick_time); }

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

  s_font_header = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HEADER_31));
  s_font_label  = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_font_value  = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // Header — full height, datebox starts at y=0
  s_header_layer = layer_create(GRect(0, 0, SCREEN_W, HEADER_H));
  layer_set_update_proc(s_header_layer, header_update_proc);
  layer_add_child(root, s_header_layer);

  // Text layers vertically centred in the datebox
  int text_y = DATEBOX_Y + (DATEBOX_H - 34) / 2;

  s_day_layer = text_layer_create(GRect(0, text_y, DATEBOX_X, 32));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, COL_WEEKDAY);
  text_layer_set_font(s_day_layer, s_font_header);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_day_layer));

  s_date_num_layer = text_layer_create(GRect(DATEBOX_X, text_y, DATEBOX_W, 32));
  text_layer_set_background_color(s_date_num_layer, GColorClear);
  text_layer_set_text_color(s_date_num_layer, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_date_num_layer, s_font_header);
  text_layer_set_text_alignment(s_date_num_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_date_num_layer));

  s_month_layer = text_layer_create(GRect(DATEBOX_X + DATEBOX_W, text_y, DATEBOX_X, 32));
  text_layer_set_background_color(s_month_layer, GColorClear);
  text_layer_set_text_color(s_month_layer, COL_WHITE);
  text_layer_set_font(s_month_layer, s_font_header);
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_month_layer));

  // Clock
  s_clock_layer = layer_create(GRect(0, CLOCK_Y, SCREEN_W, CLOCK_H));
  layer_set_update_proc(s_clock_layer, clock_update_proc);
  layer_add_child(root, s_clock_layer);

  // Step bar
  s_stepbar_layer = layer_create(GRect(0, STEPBAR_Y, SCREEN_W, STEPBAR_H));
  layer_set_update_proc(s_stepbar_layer, stepbar_update_proc);
  layer_add_child(root, s_stepbar_layer);

  // Footer — extends to bottom of screen
  s_footer_layer = layer_create(GRect(0, FOOTER_Y, SCREEN_W, SCREEN_H - FOOTER_Y));
  layer_set_update_proc(s_footer_layer, footer_update_proc);
  layer_add_child(root, s_footer_layer);

  s_weather_label = text_layer_create(GRect(4, 2, HRBOX_X - BOX_GAP - 4, 14));
  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text_color(s_weather_label, COL_WHITE);
  text_layer_set_font(s_weather_label, s_font_label);
  text_layer_set_text_alignment(s_weather_label, GTextAlignmentLeft);
  text_layer_set_text(s_weather_label, "WEATHER");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_label));

  s_weather_val = text_layer_create(GRect(4, 14, HRBOX_X - BOX_GAP - 4, 38));
  text_layer_set_background_color(s_weather_val, GColorClear);
  text_layer_set_text_color(s_weather_val, COL_WHITE);
  text_layer_set_font(s_weather_val, s_font_value);
  text_layer_set_text_alignment(s_weather_val, GTextAlignmentLeft);
  snprintf(s_weather_buf, sizeof(s_weather_buf), "--");
  text_layer_set_text(s_weather_val, s_weather_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_weather_val));

  s_weather_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_NA);
  s_weather_icon_layer  = bitmap_layer_create(GRect(40, 18, 25, 25));
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_weather_icon_layer, GCompOpSet);
  layer_add_child(s_footer_layer, bitmap_layer_get_layer(s_weather_icon_layer));

  s_hr_label = text_layer_create(GRect(HRBOX_X, 2, BOX_W, 14));
  text_layer_set_background_color(s_hr_label, GColorClear);
  text_layer_set_text_color(s_hr_label, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_hr_label, s_font_label);
  text_layer_set_text_alignment(s_hr_label, GTextAlignmentCenter);
  text_layer_set_text(s_hr_label, "HR");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_label));

  s_hr_val = text_layer_create(GRect(HRBOX_X, 14, BOX_W, 38));
  text_layer_set_background_color(s_hr_val, GColorClear);
  text_layer_set_text_color(s_hr_val, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_hr_val, s_font_value);
  text_layer_set_text_alignment(s_hr_val, GTextAlignmentCenter);
  snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
  text_layer_set_text(s_hr_val, s_hr_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_hr_val));

  int steps_x = HRBOX_X + BOX_W + BOX_GAP;
  int steps_w = SCREEN_W - steps_x - 4;

  s_steps_label = text_layer_create(GRect(steps_x, 2, steps_w, 14));
  text_layer_set_background_color(s_steps_label, GColorClear);
  text_layer_set_text_color(s_steps_label, COL_WHITE);
  text_layer_set_font(s_steps_label, s_font_label);
  text_layer_set_text_alignment(s_steps_label, GTextAlignmentRight);
  text_layer_set_text(s_steps_label, "STEPS");
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_label));

  s_steps_val = text_layer_create(GRect(steps_x, 14, steps_w, 38));
  text_layer_set_background_color(s_steps_val, GColorClear);
  text_layer_set_text_color(s_steps_val, COL_WHITE);
  text_layer_set_font(s_steps_val, s_font_value);
  text_layer_set_text_alignment(s_steps_val, GTextAlignmentRight);
  snprintf(s_steps_buf, sizeof(s_steps_buf), "0");
  text_layer_set_text(s_steps_val, s_steps_buf);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_steps_val));

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
  fonts_unload_custom_font(s_font_header);
  layer_destroy(s_header_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_num_layer);
  text_layer_destroy(s_month_layer);
  layer_destroy(s_clock_layer);
  layer_destroy(s_stepbar_layer);
  layer_destroy(s_footer_layer);
  text_layer_destroy(s_weather_label);
  text_layer_destroy(s_weather_val);
  bitmap_layer_destroy(s_weather_icon_layer);
  if (s_weather_icon_bitmap) gbitmap_destroy(s_weather_icon_bitmap);
  text_layer_destroy(s_hr_label);
  text_layer_destroy(s_hr_val);
  text_layer_destroy(s_steps_label);
  text_layer_destroy(s_steps_val);
}

static void init(void) {
  settings_load();

  s_window = window_create();
  window_set_background_color(s_window, COL_BG);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load, .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_open(256, 256);
#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) { init(); app_event_loop(); deinit(); }