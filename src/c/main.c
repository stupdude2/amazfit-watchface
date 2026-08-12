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
#define KEY_LEFT_SLOT    4
#define KEY_CENTER_SLOT  5
#define KEY_RIGHT_SLOT   6
#define KEY_FOOTER_MODE  7
#define KEY_STEPBAR_MODE 8

// ── Persistent settings ──────────────────────────────────────────────────────
#define SETTINGS_PERSIST_KEY 1
#define SETTINGS_VERSION     6

typedef enum {
  SLOT_WEATHER = 0,
  SLOT_STEPS = 1,
  SLOT_BATTERY = 2,
  SLOT_HEART_RATE = 3,
  SLOT_BLUETOOTH = 4
} SideSlotContent;

typedef enum {
  CENTER_HEART_RATE = 0,
  CENTER_BATTERY = 1,
  CENTER_BLUETOOTH = 2
} CenterSlotContent;

typedef enum {
  FOOTER_ALWAYS = 0,
  FOOTER_DOUBLE_TAP = 1,
  FOOTER_OFF = 2
} FooterMode;

typedef enum {
  STEPBAR_MIRRORED = 0,
  STEPBAR_LEFT_TO_RIGHT = 1,
  STEPBAR_HIDDEN = 2
} StepbarMode;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t stepbar_mode;
} WatchfaceSettings;

static WatchfaceSettings s_settings;

static void settings_set_defaults(void) {
  s_settings.version = SETTINGS_VERSION;
  s_settings.accent_color = GColorCobaltBlue;
  s_settings.left_slot = SLOT_WEATHER;
  s_settings.center_slot = CENTER_HEART_RATE;
  s_settings.right_slot = SLOT_STEPS;
  s_settings.footer_mode = FOOTER_ALWAYS;
  s_settings.stepbar_mode = STEPBAR_MIRRORED;
}

static bool settings_values_valid(const WatchfaceSettings *settings) {
  if (!settings) return false;
  return settings->version == SETTINGS_VERSION &&
         settings->left_slot <= SLOT_BLUETOOTH &&
         settings->center_slot <= CENTER_BLUETOOTH &&
         settings->right_slot <= SLOT_BLUETOOTH &&
         settings->footer_mode <= FOOTER_OFF &&
         settings->stepbar_mode <= STEPBAR_HIDDEN;
}

static void settings_load(void) {
  settings_set_defaults();
#if WATCHFACE_PRO
  if (persist_exists(SETTINGS_PERSIST_KEY) &&
      persist_get_size(SETTINGS_PERSIST_KEY) == (int)sizeof(s_settings)) {
    WatchfaceSettings stored;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &stored, sizeof(stored)) == (int)sizeof(stored) &&
        settings_values_valid(&stored)) {
      s_settings = stored;
    } else {
      // Discard stale/corrupt settings from earlier footer builds.
      persist_delete(SETTINGS_PERSIST_KEY);
    }
  }
#endif
}


static int32_t tuple_to_int32(const Tuple *tuple, int32_t fallback) {
  if (!tuple) return fallback;

  switch (tuple->type) {
    case TUPLE_INT:
      if (tuple->length == 1) return tuple->value->int8;
      if (tuple->length == 2) return tuple->value->int16;
      if (tuple->length == 4) return tuple->value->int32;
      break;
    case TUPLE_UINT:
      if (tuple->length == 1) return tuple->value->uint8;
      if (tuple->length == 2) return tuple->value->uint16;
      if (tuple->length == 4) return (int32_t)tuple->value->uint32;
      break;
    case TUPLE_CSTRING:
      // Clay select controls arrive as short decimal strings (for example "0" or "4").
      // Parse directly from the tuple's bounded payload instead of using strtol(),
      // which has proven unstable on this Pebble runtime.
      if (tuple->length > 1) {
        int32_t value = 0;
        bool saw_digit = false;
        bool negative = false;
        uint16_t i = 0;

        if (tuple->value->cstring[0] == '-') {
          negative = true;
          i = 1;
        }

        // length includes the terminating NUL; never read beyond it.
        for (; i + 1 < tuple->length; ++i) {
          char c = tuple->value->cstring[i];
          if (c < '0' || c > '9') break;
          value = value * 10 + (c - '0');
          saw_digit = true;
        }

        if (saw_digit) return negative ? -value : value;
      }
      break;
    default:
      break;
  }

  return fallback;
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
static TextLayer *s_left_label;
static TextLayer *s_left_val;
static TextLayer *s_center_label;
static TextLayer *s_center_val;
static TextLayer *s_right_label;
static TextLayer *s_right_val;
static BitmapLayer *s_weather_icon_left_layer;
static BitmapLayer *s_weather_icon_right_layer;
static GBitmap     *s_weather_icon_bitmap;

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
static char s_battery_buf[12];
static int  s_step_count  = 0;
static int  s_heart_rate  = 0;
static int  s_battery_percent = 0;
static bool s_bluetooth_connected = false;
static int  s_hour        = 0;
static int  s_minute      = 0;
static int  s_weather_icon = -1;
static bool s_footer_temporarily_visible = false;
static bool s_waiting_for_second_tap = false;
static AppTimer *s_tap_reset_timer = NULL;
static AppTimer *s_footer_hide_timer = NULL;

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
  if (s_weather_icon_left_layer) bitmap_layer_set_bitmap(s_weather_icon_left_layer, s_weather_icon_bitmap);
  if (s_weather_icon_right_layer) bitmap_layer_set_bitmap(s_weather_icon_right_layer, s_weather_icon_bitmap);
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

  if (s_settings.stepbar_mode == STEPBAR_HIDDEN) return;

  int steps = s_step_count < 0 ? 0 : s_step_count;
  int fill_w = (steps >= STEP_GOAL) ? bar_w : (steps * bar_w / STEP_GOAL);

  if (steps >= STEP_GOAL) {
    graphics_context_set_fill_color(ctx, COL_WHITE);
    graphics_fill_rect(ctx, GRect(bar_x, cy - 1, bar_w, 4), 0, GCornerNone);
    return;
  }

  graphics_context_set_stroke_color(ctx, s_settings.accent_color);
  graphics_context_set_stroke_width(ctx, 2);
  for (int x = bar_x; x < bar_x + bar_w; x += 4) {
    graphics_draw_pixel(ctx, GPoint(x, cy));
    if (x + 1 < bar_x + bar_w) graphics_draw_pixel(ctx, GPoint(x + 1, cy));
  }

  if (fill_w <= 0) return;
  graphics_context_set_fill_color(ctx, COL_WHITE);
  if (s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT) {
    graphics_fill_rect(ctx, GRect(bar_x, cy - 1, fill_w, 4), 0, GCornerNone);
  } else {
    int half_fill = fill_w / 2;
    int bar_cx = bar_x + bar_w / 2;
    graphics_fill_rect(ctx, GRect(bar_cx - half_fill, cy - 1, half_fill * 2, 4), 0, GCornerNone);
  }
}

// ── Footer icon helpers ──────────────────────────────────────────────────────
static void draw_battery_icon(GContext *ctx, GRect r, int percent, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 1);

  // Battery body and positive terminal.
  graphics_draw_rect(ctx, GRect(r.origin.x, r.origin.y, r.size.w - 3, r.size.h));
  graphics_fill_rect(ctx,
                     GRect(r.origin.x + r.size.w - 2,
                           r.origin.y + (r.size.h / 2) - 2,
                           2, 4),
                     0, GCornerNone);

  // Fill reflects the current charge level while leaving a 1 px inset.
  int inner_w = r.size.w - 5;
  int clamped = percent < 0 ? 0 : (percent > 100 ? 100 : percent);
  int fill_w = (inner_w * clamped) / 100;
  if (fill_w > 0) {
    graphics_fill_rect(ctx,
                       GRect(r.origin.x + 2, r.origin.y + 2,
                             fill_w, r.size.h - 4),
                       0, GCornerNone);
  }
}

static void draw_bluetooth_icon(GContext *ctx, GPoint c, int width, int height,
                                GColor color, bool connected) {
  (void)connected;
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);

  // Bluetooth rune based directly on the 24x24 SVG polyline:
  // 6.5,6.5 -> 17.5,17.5 -> 12,23 -> 12,1 ->
  // 17.5,6.5 -> 6.5,17.5
  //
  // Scale the SVG's 24x24 coordinate space into the requested square while
  // preserving the exact proportions of the supplied path.
  const int left = c.x - width / 2;
  const int top  = c.y - height / 2;

  #define BT_X2(x2) (left + (((x2) * width) + 24) / 48)
  #define BT_Y2(y2) (top  + (((y2) * height) + 24) / 48)

  GPoint points[] = {
    GPoint(BT_X2(13), BT_Y2(13)),  //  6.5,  6.5
    GPoint(BT_X2(35), BT_Y2(35)),  // 17.5, 17.5
    GPoint(BT_X2(24), BT_Y2(46)),  // 12.0, 23.0
    GPoint(BT_X2(24), BT_Y2(2)),   // 12.0,  1.0
    GPoint(BT_X2(35), BT_Y2(13)),  // 17.5,  6.5
    GPoint(BT_X2(13), BT_Y2(35))   //  6.5, 17.5
  };

  for (unsigned int i = 0; i < (sizeof(points) / sizeof(points[0])) - 1; ++i) {
    graphics_draw_line(ctx, points[i], points[i + 1]);
  }

  #undef BT_X2
  #undef BT_Y2
}

static void draw_slot_icon(GContext *ctx, uint8_t slot, GRect area,
                           GColor color, bool is_right_slot) {
  if (slot == SLOT_BATTERY) {
    // Keep the original battery shape, but make it wider and move it down.
    // Side batteries align with the same edge as their percentage value.
    const int icon_w = 36;
    const int icon_h = 9;
    const int icon_y = 7;
    int icon_x = is_right_slot
                   ? (area.origin.x + area.size.w - icon_w)
                   : area.origin.x;
    draw_battery_icon(ctx, GRect(icon_x, icon_y, icon_w, icon_h),
                      s_battery_percent, color);
  } else if (slot == SLOT_BLUETOOTH && s_bluetooth_connected) {
    // Connected Bluetooth is icon-only and fills most of the footer height.
    int cx = area.origin.x + area.size.w / 2;
    draw_bluetooth_icon(ctx, GPoint(cx, 22), 34, 30, color, true);
  }
}

static void draw_center_icon(GContext *ctx, uint8_t slot, GColor color) {
  int cx = HRBOX_X + BOX_W / 2;
  if (slot == CENTER_BATTERY) {
    // Center battery remains centered above the percentage value.
    draw_battery_icon(ctx, GRect(cx - 18, 7, 36, 9),
                      s_battery_percent, color);
  } else if (slot == CENTER_BLUETOOTH && s_bluetooth_connected) {
    // Connected Bluetooth is icon-only and spans the label/value area.
    draw_bluetooth_icon(ctx, GPoint(cx, 22), 34, 30, color, true);
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
  graphics_fill_rect(ctx, GRect(0, 0, HRBOX_X - BOX_GAP, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(HRBOX_X + BOX_W + BOX_GAP, 0, SCREEN_W - HRBOX_X - BOX_W - BOX_GAP, 2), 0, GCornerNone);

  GColor center_fg = gcolor_legible_over(s_settings.accent_color);
  GRect left_area = GRect(4, 0, HRBOX_X - BOX_GAP - 4, 14);
  int right_x = HRBOX_X + BOX_W + BOX_GAP;
  GRect right_area = GRect(right_x, 0, SCREEN_W - right_x - 4, 14);
  draw_slot_icon(ctx, s_settings.left_slot, left_area, COL_WHITE, false);
  draw_center_icon(ctx, s_settings.center_slot, center_fg);
  draw_slot_icon(ctx, s_settings.right_slot, right_area, COL_WHITE, true);
}

static void update_time(struct tm *tick_time);

static const char *side_slot_label(uint8_t slot) {
  switch (slot) {
    case SLOT_STEPS: return "STEPS";
    case SLOT_BATTERY: return "";
    case SLOT_HEART_RATE: return "HR";
    case SLOT_BLUETOOTH: return s_bluetooth_connected ? "" : "BT";
    case SLOT_WEATHER:
    default: return "WEATHER";
  }
}

static const char *side_slot_value(uint8_t slot) {
  switch (slot) {
    case SLOT_STEPS:
      snprintf(s_steps_buf, sizeof(s_steps_buf), "%d", s_step_count);
      return s_steps_buf;
    case SLOT_BATTERY:
      snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", s_battery_percent);
      return s_battery_buf;
    case SLOT_HEART_RATE:
      if (s_heart_rate > 0) snprintf(s_hr_buf, sizeof(s_hr_buf), "%d", s_heart_rate);
      else snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
      return s_hr_buf;
    case SLOT_BLUETOOTH:
      return "";
    case SLOT_WEATHER:
    default:
      return s_weather_buf;
  }
}

static const char *center_slot_label(void) {
  switch (s_settings.center_slot) {
    case CENTER_BATTERY: return "";
    case CENTER_BLUETOOTH: return s_bluetooth_connected ? "" : "BT";
    case CENTER_HEART_RATE:
    default: return "HR";
  }
}

static const char *center_slot_value(void) {
  switch (s_settings.center_slot) {
    case CENTER_BATTERY:
      // 100% is one character too wide for the center box; omit % only there.
      if (s_battery_percent == 100) {
        snprintf(s_battery_buf, sizeof(s_battery_buf), "100");
      } else {
        snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", s_battery_percent);
      }
      return s_battery_buf;
    case CENTER_BLUETOOTH:
      return "";
    case CENTER_HEART_RATE:
    default:
      if (s_heart_rate > 0) snprintf(s_hr_buf, sizeof(s_hr_buf), "%d", s_heart_rate);
      else snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
      return s_hr_buf;
  }
}

static void update_footer_content(void) {
  if (!s_left_label || !s_center_label || !s_right_label ||
      !s_weather_icon_left_layer || !s_weather_icon_right_layer) return;

  text_layer_set_text(s_left_label, side_slot_label(s_settings.left_slot));
  text_layer_set_text(s_left_val, side_slot_value(s_settings.left_slot));
  text_layer_set_text(s_center_label, center_slot_label());
  text_layer_set_text(s_center_val, center_slot_value());
  text_layer_set_text(s_right_label, side_slot_label(s_settings.right_slot));
  text_layer_set_text(s_right_val, side_slot_value(s_settings.right_slot));

  // Disconnected Bluetooth is a simple centered "BT" label with no value.
  // Restore the normal edge alignment automatically for every other option.
  text_layer_set_text_alignment(
      s_left_label,
      (s_settings.left_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentLeft);
  text_layer_set_text_alignment(s_left_val, GTextAlignmentLeft);
  text_layer_set_text_alignment(
      s_right_label,
      (s_settings.right_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentRight);
  text_layer_set_text_alignment(s_right_val, GTextAlignmentRight);

  bool weather_left = s_settings.left_slot == SLOT_WEATHER;
  bool weather_right = s_settings.right_slot == SLOT_WEATHER;
  layer_set_hidden(bitmap_layer_get_layer(s_weather_icon_left_layer), !weather_left);
  layer_set_hidden(bitmap_layer_get_layer(s_weather_icon_right_layer), !weather_right);
  if (s_footer_layer) layer_mark_dirty(s_footer_layer);
}

static void apply_footer_visibility(void) {
  if (!s_footer_layer) return;
  bool visible = s_settings.footer_mode == FOOTER_ALWAYS ||
                 (s_settings.footer_mode == FOOTER_DOUBLE_TAP && s_footer_temporarily_visible);
  layer_set_hidden(s_footer_layer, !visible);
}

static void footer_hide_callback(void *context) {
  s_footer_hide_timer = NULL;
  s_footer_temporarily_visible = false;
  apply_footer_visibility();
}

static void tap_reset_callback(void *context) {
  s_tap_reset_timer = NULL;
  s_waiting_for_second_tap = false;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  if (s_settings.footer_mode != FOOTER_DOUBLE_TAP) return;
  if (s_waiting_for_second_tap) {
    s_waiting_for_second_tap = false;
    if (s_tap_reset_timer) { app_timer_cancel(s_tap_reset_timer); s_tap_reset_timer = NULL; }
    s_footer_temporarily_visible = true;
    apply_footer_visibility();
    if (s_footer_hide_timer) app_timer_cancel(s_footer_hide_timer);
    s_footer_hide_timer = app_timer_register(5000, footer_hide_callback, NULL);
  } else {
    s_waiting_for_second_tap = true;
    if (s_tap_reset_timer) app_timer_cancel(s_tap_reset_timer);
    s_tap_reset_timer = app_timer_register(700, tap_reset_callback, NULL);
  }
}

static void battery_handler(BatteryChargeState charge) {
  s_battery_percent = charge.charge_percent;
  update_footer_content();
}

static void connection_handler(bool connected) {
  s_bluetooth_connected = connected;
  update_footer_content();
}

// ── Accent color helper ─────────────────────────────────────────────────────
static void update_accent_text_contrast(void) {
  GColor text_color = gcolor_legible_over(s_settings.accent_color);

  if (s_date_num_layer) {
    text_layer_set_text_color(s_date_num_layer, text_color);
  }
  if (s_center_label) text_layer_set_text_color(s_center_label, text_color);
  if (s_center_val) text_layer_set_text_color(s_center_val, text_color);
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
  bool weather_changed = false;
  bool accent_changed = false;
  bool layout_changed = false;
  uint32_t new_accent_hex = 0;

  // Parse the incoming dictionary exactly once. Do not redraw, persist, or
  // start any other AppMessage operation until parsing is complete.
  for (Tuple *t = dict_read_first(iter); t; t = dict_read_next(iter)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "RX key=%lu type=%d len=%u",
            (unsigned long)t->key, (int)t->type, (unsigned)t->length);

    switch (t->key) {
      case KEY_TEMPERATURE: {
        int32_t temp = tuple_to_int32(t, 0);
        snprintf(s_weather_buf, sizeof(s_weather_buf), "%ld\xC2\xB0", (long)temp);
        weather_changed = true;
        break;
      }

      case KEY_WEATHER_ICON: {
        int32_t icon = tuple_to_int32(t, s_weather_icon);
        if (icon >= 0 && icon <= 5) {
          s_weather_icon = (int)icon;
          weather_changed = true;
        }
        break;
      }

#if WATCHFACE_PRO
      case KEY_ACCENT_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          new_accent_hex = (uint32_t)tuple_to_int32(t, 0) & 0xFFFFFF;
          accent_changed = true;
        }
        break;

      case KEY_LEFT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.left_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_BLUETOOTH) {
          s_settings.left_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Left slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_CENTER_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.center_slot);
        if (value >= CENTER_HEART_RATE && value <= CENTER_BLUETOOTH) {
          s_settings.center_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Center slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_RIGHT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.right_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_BLUETOOTH) {
          s_settings.right_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Right slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_FOOTER_MODE: {
        int32_t value = tuple_to_int32(t, s_settings.footer_mode);
        if (value >= FOOTER_ALWAYS && value <= FOOTER_OFF) {
          s_settings.footer_mode = (uint8_t)value;
          s_footer_temporarily_visible = false;
          APP_LOG(APP_LOG_LEVEL_INFO, "Footer mode -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_STEPBAR_MODE: {
        int32_t value = tuple_to_int32(t, s_settings.stepbar_mode);
        if (value >= STEPBAR_MIRRORED && value <= STEPBAR_HIDDEN) {
          s_settings.stepbar_mode = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Stepbar mode -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }
#endif

      default:
        break;
    }
  }

  // Only touch persistent storage and UI after DictionaryIterator is finished.
#if WATCHFACE_PRO
  if (!settings_values_valid(&s_settings)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid settings after config; restoring footer defaults");
    GColor keep_accent = s_settings.accent_color;
    settings_set_defaults();
    s_settings.accent_color = keep_accent;
    layout_changed = true;
  }

  if (accent_changed) {
    s_settings.accent_color = GColorFromHEX(new_accent_hex);
  }

  if (accent_changed || layout_changed) {
    settings_save();
  }
#endif

  if (weather_changed) {
    update_weather_icon(s_weather_icon);
    update_footer_content();
  }

#if WATCHFACE_PRO
  if (accent_changed) {
    update_accent_text_contrast();
    if (s_header_layer) layer_mark_dirty(s_header_layer);
    if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
    if (s_footer_layer) layer_mark_dirty(s_footer_layer);

    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    if (current) update_time(current);

    APP_LOG(APP_LOG_LEVEL_INFO, "Accent color applied: 0x%06lX",
            (unsigned long)new_accent_hex);
  }

  if (layout_changed) {
    update_footer_content();
    apply_footer_visibility();
    if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
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
    if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
    update_footer_content();
  }
  if (event == HealthEventHeartRateUpdate || event == HealthEventSignificantUpdate) {
    HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
    s_heart_rate = hr > 0 ? (int)hr : 0;
    update_footer_content();
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

  int left_w = HRBOX_X - BOX_GAP - 4;
  int right_x = HRBOX_X + BOX_W + BOX_GAP;
  int right_w = SCREEN_W - right_x - 4;

  s_left_label = text_layer_create(GRect(4, 2, left_w, 14));
  text_layer_set_background_color(s_left_label, GColorClear);
  text_layer_set_text_color(s_left_label, COL_WHITE);
  text_layer_set_font(s_left_label, s_font_label);
  text_layer_set_text_alignment(s_left_label, GTextAlignmentLeft);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_left_label));

  s_left_val = text_layer_create(GRect(4, 14, left_w, 38));
  text_layer_set_background_color(s_left_val, GColorClear);
  text_layer_set_text_color(s_left_val, COL_WHITE);
  text_layer_set_font(s_left_val, s_font_value);
  text_layer_set_text_alignment(s_left_val, GTextAlignmentLeft);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_left_val));

  s_center_label = text_layer_create(GRect(HRBOX_X, 2, BOX_W, 14));
  text_layer_set_background_color(s_center_label, GColorClear);
  text_layer_set_text_color(s_center_label, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_center_label, s_font_label);
  text_layer_set_text_alignment(s_center_label, GTextAlignmentCenter);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_center_label));

  s_center_val = text_layer_create(GRect(HRBOX_X, 14, BOX_W, 38));
  text_layer_set_background_color(s_center_val, GColorClear);
  text_layer_set_text_color(s_center_val, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_center_val, s_font_value);
  text_layer_set_text_alignment(s_center_val, GTextAlignmentCenter);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_center_val));

  s_right_label = text_layer_create(GRect(right_x, 2, right_w, 14));
  text_layer_set_background_color(s_right_label, GColorClear);
  text_layer_set_text_color(s_right_label, COL_WHITE);
  text_layer_set_font(s_right_label, s_font_label);
  text_layer_set_text_alignment(s_right_label, GTextAlignmentRight);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_right_label));

  s_right_val = text_layer_create(GRect(right_x, 14, right_w, 38));
  text_layer_set_background_color(s_right_val, GColorClear);
  text_layer_set_text_color(s_right_val, COL_WHITE);
  text_layer_set_font(s_right_val, s_font_value);
  text_layer_set_text_alignment(s_right_val, GTextAlignmentRight);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_right_val));

  s_weather_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_NA);
  s_weather_icon_left_layer = bitmap_layer_create(GRect(40, 18, 25, 25));
  bitmap_layer_set_bitmap(s_weather_icon_left_layer, s_weather_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_weather_icon_left_layer, GCompOpSet);
  layer_add_child(s_footer_layer, bitmap_layer_get_layer(s_weather_icon_left_layer));

  s_weather_icon_right_layer = bitmap_layer_create(GRect(SCREEN_W - 68, 18, 25, 25));
  bitmap_layer_set_bitmap(s_weather_icon_right_layer, s_weather_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_weather_icon_right_layer, GCompOpSet);
  layer_add_child(s_footer_layer, bitmap_layer_get_layer(s_weather_icon_right_layer));

  snprintf(s_weather_buf, sizeof(s_weather_buf), "--");
  BatteryChargeState charge = battery_state_service_peek();
  s_battery_percent = charge.charge_percent;
  s_bluetooth_connected = connection_service_peek_pebble_app_connection();

#if defined(PBL_HEALTH)
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
  s_heart_rate = hr > 0 ? (int)hr : 0;
#endif

  update_footer_content();
  update_accent_text_contrast();
  apply_footer_visibility();

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  if (t) update_time(t);

}

// ── Window unload ─────────────────────────────────────────────────────────────
static void window_unload(Window *window) {
  // Destroy child layers before their parent layers. Destroying a parent first
  // and then destroying its former children can fault on Pebble.
  bitmap_layer_destroy(s_weather_icon_left_layer);
  bitmap_layer_destroy(s_weather_icon_right_layer);
  s_weather_icon_left_layer = NULL;
  s_weather_icon_right_layer = NULL;

  text_layer_destroy(s_left_label);
  text_layer_destroy(s_left_val);
  text_layer_destroy(s_center_label);
  text_layer_destroy(s_center_val);
  text_layer_destroy(s_right_label);
  text_layer_destroy(s_right_val);
  s_left_label = NULL;
  s_left_val = NULL;
  s_center_label = NULL;
  s_center_val = NULL;
  s_right_label = NULL;
  s_right_val = NULL;

  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_num_layer);
  text_layer_destroy(s_month_layer);
  s_day_layer = NULL;
  s_date_num_layer = NULL;
  s_month_layer = NULL;

  layer_destroy(s_header_layer);
  layer_destroy(s_clock_layer);
  layer_destroy(s_stepbar_layer);
  layer_destroy(s_footer_layer);
  s_header_layer = NULL;
  s_clock_layer = NULL;
  s_stepbar_layer = NULL;
  s_footer_layer = NULL;

  if (s_weather_icon_bitmap) {
    gbitmap_destroy(s_weather_icon_bitmap);
    s_weather_icon_bitmap = NULL;
  }
  fonts_unload_custom_font(s_font_header);
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
  battery_state_service_subscribe(battery_handler);
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = connection_handler
  });
  accel_tap_service_subscribe(accel_tap_handler);
#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  accel_tap_service_unsubscribe();
  if (s_tap_reset_timer) app_timer_cancel(s_tap_reset_timer);
  if (s_footer_hide_timer) app_timer_cancel(s_footer_hide_timer);
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) { init(); app_event_loop(); deinit(); }