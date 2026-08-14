#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <kiezelpay-core/kiezelpay.h>
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
// Step-bar layout helpers are declared before the draw proc because C99 does not
// allow implicit function declarations.
static bool stepbar_is_backlight_only(void);
static bool stepbar_is_above(void);
static bool stepbar_is_left_to_right(void);

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

#define CENTER12_VISIBLE_W (DIGIT_WIDTH + COLON_MARGIN + COLON_WIDTH + COLON_MARGIN + DIGIT_WIDTH + DIGIT_MARGIN + DIGIT_WIDTH)
#define CENTER12_H2_X      ((SCREEN_W - CENTER12_VISIBLE_W) / 2)
#define CENTER12_COL_X     (CENTER12_H2_X + DIGIT_WIDTH + COLON_MARGIN)
#define CENTER12_M1_X      (CENTER12_COL_X + COLON_WIDTH + COLON_MARGIN)
#define CENTER12_M2_X      (CENTER12_M1_X + DIGIT_WIDTH + DIGIT_MARGIN)

// ── 24-hour digit geometry ───────────────────────────────────────────────────
// Keep the same 110px height and the same 6px gaps used by the original clock.
// All four digit cells use identical geometry so HH:MM has consistent width.
// Thin strokes and compact cells keep the full clock within the 200px display.
#define H24_STK           14
#define H24_H1_WIDTH      39
#define H24_DIGIT_WIDTH   39
#define H24_TOTAL_W       (H24_H1_WIDTH + DIGIT_MARGIN + H24_DIGIT_WIDTH + COLON_MARGIN + COLON_WIDTH + COLON_MARGIN + H24_DIGIT_WIDTH + DIGIT_MARGIN + H24_DIGIT_WIDTH)
#define H24_ORIGIN_X      ((SCREEN_W - H24_TOTAL_W) / 2)
#define H24_H1_X          H24_ORIGIN_X
#define H24_H2_X          (H24_H1_X + H24_H1_WIDTH + DIGIT_MARGIN)
#define H24_COL_X         (H24_H2_X + H24_DIGIT_WIDTH + COLON_MARGIN)
#define H24_M1_X          (H24_COL_X + COLON_WIDTH + COLON_MARGIN)
#define H24_M2_X          (H24_M1_X + H24_DIGIT_WIDTH + DIGIT_MARGIN)

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
#define KEY_HEADER_MODE  9
#define KEY_STEP_GOAL    10
#define KEY_TEMP_UNIT    11
#define KEY_CLOCK_COLOR  12
#define KEY_BACKGROUND_COLOR 13
#define KEY_TOP_LEFT_SLOT 14
#define KEY_TOP_CENTER_SLOT 15
#define KEY_TOP_RIGHT_SLOT 16
#define KEY_TIME_FORMAT    17
#define KEY_CENTER_12H     18
#define KEY_SUNRISE        19
#define KEY_SUNSET         20
#define KEY_HIGH_TEMP      21
#define KEY_LOW_TEMP       22
#define KEY_RAISE_WAKE     23
#define KEY_PRO_LICENSE    24
#define KEY_LICENSE_CHECK  25
#define KEY_TRIAL_START    26
#define KEY_TRIAL_STATE    27
#define KEY_TRIAL_REMAINING 28
#define KEY_PURCHASE_PRO   29
#define KEY_TRIAL_USED_HINT 30

// ── Persistent settings ──────────────────────────────────────────────────────
#define SETTINGS_PERSIST_KEY      1
#define TRIAL_USED_PERSIST_KEY    2
#define TRIAL_START_PERSIST_KEY   3
#define PRO_SETTINGS_PERSIST_KEY  4
#define SETTINGS_VERSION          13
#define PRO_TRIAL_SECONDS          (48 * 60 * 60)

typedef enum {
  SLOT_WEATHER = 0,
  SLOT_STEPS = 1,
  SLOT_BATTERY = 2,
  SLOT_HEART_RATE = 3,
  SLOT_BLUETOOTH = 4,
  SLOT_DAY = 5,
  SLOT_DATE = 6,
  SLOT_MONTH = 7,
  SLOT_CALORIES = 8,
  SLOT_DISTANCE = 9,
  SLOT_SUNRISE = 10,
  SLOT_SUNSET = 11,
  SLOT_HIGH_LOW = 12
} SideSlotContent;

typedef enum {
  CENTER_HEART_RATE = 0,
  CENTER_BATTERY = 1,
  CENTER_BLUETOOTH = 2,
  CENTER_WEATHER = 3,
  CENTER_STEPS = 4,
  CENTER_DAY = 5,
  CENTER_DATE = 6,
  CENTER_MONTH = 7
} CenterSlotContent;

typedef enum {
  BAR_ALWAYS = 0,
  BAR_BACKLIGHT = 1,
  BAR_HIDDEN = 2
} BarVisibilityMode;

typedef enum {
  STEPBAR_MIRRORED = 0,
  STEPBAR_LEFT_TO_RIGHT = 1,
  STEPBAR_HIDDEN = 2,
  STEPBAR_MIRRORED_ABOVE = 3,
  STEPBAR_LEFT_TO_RIGHT_ABOVE = 4,
  STEPBAR_MIRRORED_BACKLIGHT = 5,
  STEPBAR_LEFT_TO_RIGHT_BACKLIGHT = 6,
  STEPBAR_MIRRORED_ABOVE_BACKLIGHT = 7,
  STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT = 8
} StepbarMode;

typedef enum {
  TEMP_FAHRENHEIT = 0,
  TEMP_CELSIUS = 1
} TemperatureUnit;

typedef enum {
  TIME_FORMAT_12H = 0,
  TIME_FORMAT_24H = 1
} ClockTimeFormat;

typedef enum {
  RAISE_WAKE_OFF = 0,
  RAISE_WAKE_NORMAL = 1,
  RAISE_WAKE_SENSITIVE = 2
} RaiseWakeMode;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
} WatchfaceSettingsV7;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
  uint16_t step_goal;
  uint8_t temp_unit;
  GColor clock_color;
} WatchfaceSettingsV8;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
  uint16_t step_goal;
  uint8_t temp_unit;
  GColor clock_color;
  GColor background_color;
} WatchfaceSettingsV9;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
  uint16_t step_goal;
  uint8_t temp_unit;
  GColor clock_color;
  GColor background_color;
  uint8_t top_left_slot;
  uint8_t top_center_slot;
  uint8_t top_right_slot;
} WatchfaceSettingsV10;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
  uint16_t step_goal;
  uint8_t temp_unit;
  GColor clock_color;
  GColor background_color;
  uint8_t top_left_slot;
  uint8_t top_center_slot;
  uint8_t top_right_slot;
  uint8_t time_format;
} WatchfaceSettingsV11;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
  uint16_t step_goal;
  uint8_t temp_unit;
  GColor clock_color;
  GColor background_color;
  uint8_t top_left_slot;
  uint8_t top_center_slot;
  uint8_t top_right_slot;
  uint8_t time_format;
  uint8_t center_12h;
} WatchfaceSettingsV12;

typedef struct {
  uint8_t version;
  GColor accent_color;
  uint8_t left_slot;
  uint8_t center_slot;
  uint8_t right_slot;
  uint8_t footer_mode;
  uint8_t header_mode;
  uint8_t stepbar_mode;
  uint16_t step_goal;
  uint8_t temp_unit;
  GColor clock_color;
  GColor background_color;
  uint8_t top_left_slot;
  uint8_t top_center_slot;
  uint8_t top_right_slot;
  uint8_t time_format;
  uint8_t center_12h;
  uint8_t raise_wake_mode;
} WatchfaceSettings;

static WatchfaceSettings s_settings;

// Runtime Pro entitlement. KiezelPay's product-specific Pebble library should
// call license_set_pro(true/false) from its license callback.
// Default is free/locked so a failed or unavailable license check never grants
// premium features accidentally.
static bool s_pro_unlocked = false;
static bool s_kiezelpay_licensed = false;
static AppTimer *s_kiezelpay_purchase_timer = NULL;
static bool s_trial_active = false;

static void license_send_status_to_phone(void);
static void license_recompute_effective(void);
static void trial_refresh_state(void);

static void settings_set_defaults(void) {
  s_settings.version = SETTINGS_VERSION;
  s_settings.accent_color = GColorCobaltBlue;
  s_settings.left_slot = SLOT_WEATHER;
  s_settings.center_slot = CENTER_HEART_RATE;
  s_settings.right_slot = SLOT_STEPS;
  s_settings.footer_mode = BAR_ALWAYS;
  s_settings.header_mode = BAR_ALWAYS;
  s_settings.stepbar_mode = STEPBAR_MIRRORED;
  s_settings.step_goal = 5000;
  s_settings.temp_unit = TEMP_FAHRENHEIT;
  s_settings.clock_color = GColorWhite;
  s_settings.background_color = GColorBlack;
  s_settings.top_left_slot = SLOT_DAY;
  s_settings.top_center_slot = SLOT_DATE;
  s_settings.top_right_slot = SLOT_MONTH;
  s_settings.time_format = TIME_FORMAT_12H;
  s_settings.center_12h = 1;
  s_settings.raise_wake_mode = RAISE_WAKE_OFF;
}


static void enforce_free_defaults(void) {
  // Preserve the two free customization choices.
  uint8_t keep_time_format = s_settings.time_format;
  uint8_t keep_center_12h = s_settings.center_12h;

  // Everything else returns to the standard watchface presentation.
  s_settings.accent_color = GColorCobaltBlue;
  s_settings.clock_color = GColorWhite;
  s_settings.background_color = GColorBlack;

  s_settings.left_slot = SLOT_WEATHER;
  s_settings.center_slot = CENTER_HEART_RATE;
  s_settings.right_slot = SLOT_STEPS;
  s_settings.footer_mode = BAR_ALWAYS;

  s_settings.top_left_slot = SLOT_DAY;
  s_settings.top_center_slot = SLOT_DATE;
  s_settings.top_right_slot = SLOT_MONTH;
  s_settings.header_mode = BAR_ALWAYS;

  s_settings.stepbar_mode = STEPBAR_MIRRORED;
  s_settings.step_goal = 5000;
  s_settings.temp_unit = TEMP_FAHRENHEIT;
  s_settings.raise_wake_mode = RAISE_WAKE_OFF;

  s_settings.time_format =
      keep_time_format <= TIME_FORMAT_24H ? keep_time_format : TIME_FORMAT_12H;
  s_settings.center_12h = keep_center_12h ? 1 : 0;
}

static bool key_is_free_customization(uint32_t key) {
  return key == KEY_TIME_FORMAT || key == KEY_CENTER_12H;
}

static bool key_is_pro_customization(uint32_t key) {
  switch (key) {
    case KEY_ACCENT_COLOR:
    case KEY_LEFT_SLOT:
    case KEY_CENTER_SLOT:
    case KEY_RIGHT_SLOT:
    case KEY_FOOTER_MODE:
    case KEY_STEPBAR_MODE:
    case KEY_HEADER_MODE:
    case KEY_STEP_GOAL:
    case KEY_TEMP_UNIT:
    case KEY_CLOCK_COLOR:
    case KEY_BACKGROUND_COLOR:
    case KEY_TOP_LEFT_SLOT:
    case KEY_TOP_CENTER_SLOT:
    case KEY_TOP_RIGHT_SLOT:
    case KEY_RAISE_WAKE:
      return true;
    default:
      return false;
  }
}

static bool settings_values_valid(const WatchfaceSettings *settings) {
  if (!settings) return false;
  return settings->version == SETTINGS_VERSION &&
         settings->left_slot <= SLOT_HIGH_LOW &&
         settings->center_slot <= CENTER_MONTH &&
         settings->right_slot <= SLOT_HIGH_LOW &&
         settings->top_left_slot <= SLOT_HIGH_LOW &&
         settings->top_center_slot <= SLOT_MONTH &&
         settings->top_right_slot <= SLOT_HIGH_LOW &&
         settings->footer_mode <= BAR_HIDDEN &&
         settings->header_mode <= BAR_HIDDEN &&
         settings->stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
         settings->step_goal >= 1000 && settings->step_goal <= 30000 &&
         settings->temp_unit <= TEMP_CELSIUS &&
         settings->time_format <= TIME_FORMAT_24H &&
         settings->center_12h <= 1 &&
         settings->raise_wake_mode <= RAISE_WAKE_SENSITIVE;
}

static void settings_load(void) {
  settings_set_defaults();
  if (!persist_exists(SETTINGS_PERSIST_KEY)) return;

  int stored_size = persist_get_size(SETTINGS_PERSIST_KEY);
  if (stored_size == (int)sizeof(s_settings)) {
    WatchfaceSettings stored;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &stored, sizeof(stored)) == (int)sizeof(stored) &&
        settings_values_valid(&stored)) {
      s_settings = stored;
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV12)) {
    WatchfaceSettingsV12 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 12 &&
        old.left_slot <= SLOT_HIGH_LOW &&
        old.center_slot <= CENTER_MONTH &&
        old.right_slot <= SLOT_HIGH_LOW &&
        old.top_left_slot <= SLOT_HIGH_LOW &&
        old.top_center_slot <= SLOT_MONTH &&
        old.top_right_slot <= SLOT_HIGH_LOW &&
        old.footer_mode <= BAR_HIDDEN &&
        old.header_mode <= BAR_HIDDEN &&
        old.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
        old.step_goal >= 1000 && old.step_goal <= 30000 &&
        old.temp_unit <= TEMP_CELSIUS &&
        old.time_format <= TIME_FORMAT_24H &&
        old.center_12h <= 1) {
      s_settings.accent_color = old.accent_color;
      s_settings.left_slot = old.left_slot;
      s_settings.center_slot = old.center_slot;
      s_settings.right_slot = old.right_slot;
      s_settings.footer_mode = old.footer_mode;
      s_settings.header_mode = old.header_mode;
      s_settings.stepbar_mode = old.stepbar_mode;
      s_settings.step_goal = old.step_goal;
      s_settings.temp_unit = old.temp_unit;
      s_settings.clock_color = old.clock_color;
      s_settings.background_color = old.background_color;
      s_settings.top_left_slot = old.top_left_slot;
      s_settings.top_center_slot = old.top_center_slot;
      s_settings.top_right_slot = old.top_right_slot;
      s_settings.time_format = old.time_format;
      s_settings.center_12h = old.center_12h;
      s_settings.raise_wake_mode = RAISE_WAKE_OFF;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV11)) {
    WatchfaceSettingsV11 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 11 &&
        old.left_slot <= SLOT_MONTH &&
        old.center_slot <= CENTER_MONTH &&
        old.right_slot <= SLOT_MONTH &&
        old.top_left_slot <= SLOT_MONTH &&
        old.top_center_slot <= SLOT_MONTH &&
        old.top_right_slot <= SLOT_MONTH &&
        old.footer_mode <= BAR_HIDDEN &&
        old.header_mode <= BAR_HIDDEN &&
        old.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
        old.step_goal >= 1000 && old.step_goal <= 30000 &&
        old.temp_unit <= TEMP_CELSIUS &&
        old.time_format <= TIME_FORMAT_24H) {
      s_settings.accent_color = old.accent_color;
      s_settings.left_slot = old.left_slot;
      s_settings.center_slot = old.center_slot;
      s_settings.right_slot = old.right_slot;
      s_settings.footer_mode = old.footer_mode;
      s_settings.header_mode = old.header_mode;
      s_settings.stepbar_mode = old.stepbar_mode;
      s_settings.step_goal = old.step_goal;
      s_settings.temp_unit = old.temp_unit;
      s_settings.clock_color = old.clock_color;
      s_settings.background_color = old.background_color;
      s_settings.top_left_slot = old.top_left_slot;
      s_settings.top_center_slot = old.top_center_slot;
      s_settings.top_right_slot = old.top_right_slot;
      s_settings.time_format = old.time_format;
      s_settings.center_12h = 0;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV10)) {
    // Migrate v1.4/v1.5 settings and default the new time format to 12-hour.
    WatchfaceSettingsV10 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 10 &&
        old.left_slot <= SLOT_MONTH &&
        old.center_slot <= CENTER_MONTH &&
        old.right_slot <= SLOT_MONTH &&
        old.top_left_slot <= SLOT_MONTH &&
        old.top_center_slot <= SLOT_MONTH &&
        old.top_right_slot <= SLOT_MONTH &&
        old.footer_mode <= BAR_HIDDEN &&
        old.header_mode <= BAR_HIDDEN &&
        old.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
        old.step_goal >= 1000 && old.step_goal <= 30000 &&
        old.temp_unit <= TEMP_CELSIUS) {
      s_settings.accent_color = old.accent_color;
      s_settings.left_slot = old.left_slot;
      s_settings.center_slot = old.center_slot;
      s_settings.right_slot = old.right_slot;
      s_settings.footer_mode = old.footer_mode;
      s_settings.header_mode = old.header_mode;
      s_settings.stepbar_mode = old.stepbar_mode;
      s_settings.step_goal = old.step_goal;
      s_settings.temp_unit = old.temp_unit;
      s_settings.clock_color = old.clock_color;
      s_settings.background_color = old.background_color;
      s_settings.top_left_slot = old.top_left_slot;
      s_settings.top_center_slot = old.top_center_slot;
      s_settings.top_right_slot = old.top_right_slot;
      s_settings.time_format = TIME_FORMAT_12H;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV9)) {
    // Migrate v1.3.x settings and preserve the classic DAY / DATE / MONTH header.
    WatchfaceSettingsV9 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 9 &&
        old.left_slot <= SLOT_BLUETOOTH &&
        old.center_slot <= CENTER_BLUETOOTH &&
        old.right_slot <= SLOT_BLUETOOTH &&
        old.footer_mode <= BAR_BACKLIGHT &&
        old.header_mode <= BAR_BACKLIGHT &&
        old.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
        old.step_goal >= 1000 && old.step_goal <= 30000 &&
        old.temp_unit <= TEMP_CELSIUS) {
      s_settings.accent_color = old.accent_color;
      s_settings.left_slot = old.left_slot;
      s_settings.center_slot = old.center_slot;
      s_settings.right_slot = old.right_slot;
      s_settings.footer_mode = old.footer_mode;
      s_settings.header_mode = old.header_mode;
      s_settings.stepbar_mode = old.stepbar_mode;
      s_settings.step_goal = old.step_goal;
      s_settings.temp_unit = old.temp_unit;
      s_settings.clock_color = old.clock_color;
      s_settings.background_color = old.background_color;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV8)) {
    // Migrate v1.3.0 settings and preserve all existing customization while
    // supplying the new background color default.
    WatchfaceSettingsV8 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 8 &&
        old.left_slot <= SLOT_BLUETOOTH &&
        old.center_slot <= CENTER_BLUETOOTH &&
        old.right_slot <= SLOT_BLUETOOTH &&
        old.footer_mode <= BAR_BACKLIGHT &&
        old.header_mode <= BAR_BACKLIGHT &&
        old.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
        old.step_goal >= 1000 && old.step_goal <= 30000 &&
        old.temp_unit <= TEMP_CELSIUS) {
      s_settings.accent_color = old.accent_color;
      s_settings.left_slot = old.left_slot;
      s_settings.center_slot = old.center_slot;
      s_settings.right_slot = old.right_slot;
      s_settings.footer_mode = old.footer_mode;
      s_settings.header_mode = old.header_mode;
      s_settings.stepbar_mode = old.stepbar_mode;
      s_settings.step_goal = old.step_goal;
      s_settings.temp_unit = old.temp_unit;
      s_settings.clock_color = old.clock_color;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV7)) {
    // Migrate the user's existing v1.2.x layout/accent settings and supply
    // defaults for the three new customization fields.
    WatchfaceSettingsV7 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 7 &&
        old.left_slot <= SLOT_BLUETOOTH &&
        old.center_slot <= CENTER_BLUETOOTH &&
        old.right_slot <= SLOT_BLUETOOTH &&
        old.footer_mode <= BAR_BACKLIGHT &&
        old.header_mode <= BAR_BACKLIGHT &&
        old.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT) {
      s_settings.accent_color = old.accent_color;
      s_settings.left_slot = old.left_slot;
      s_settings.center_slot = old.center_slot;
      s_settings.right_slot = old.right_slot;
      s_settings.footer_mode = old.footer_mode;
      s_settings.header_mode = old.header_mode;
      s_settings.stepbar_mode = old.stepbar_mode;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  }

  // Discard stale/corrupt settings that cannot be migrated safely.
  persist_delete(SETTINGS_PERSIST_KEY);
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
  persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
  // While Pro is active, continuously keep a second copy of the personalized
  // setup. This lets startup safely render Free defaults until KiezelPay has
  // revalidated a paid license, without destroying the user's Pro choices.
  if (s_pro_unlocked) {
    persist_write_data(PRO_SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
  }
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
#define COL_WHITE   GColorWhite

// ── Layers ────────────────────────────────────────────────────────────────────
static Window    *s_window;
static Layer     *s_header_layer;
static TextLayer *s_top_left_label;
static TextLayer *s_top_left_val;
static TextLayer *s_top_center_label;
static TextLayer *s_top_center_val;
static TextLayer *s_top_right_label;
static TextLayer *s_top_right_val;
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
static char s_calories_buf[12];
static char s_distance_buf[12];
static char s_sunrise_buf[12];
static char s_sunset_buf[12];
static char s_high_low_buf[16];
static int  s_step_count  = 0;
static int  s_active_kcal = 0;
static int  s_distance_m = 0;
static int  s_heart_rate  = 0;
static int  s_battery_percent = 0;
static bool s_bluetooth_connected = false;
static int  s_hour        = 0;
static int  s_minute      = 0;
static int  s_weather_icon = -1;
static int  s_temperature_c_x10 = 0;
static bool s_have_temperature = false;
static int  s_sunrise_minute = -1;
static int  s_sunset_minute = -1;
static int  s_high_c_x10 = 0;
static int  s_low_c_x10 = 0;
static bool s_have_high_low = false;
static bool s_backlight_on = false;
static bool s_backlight_subscribed = false;

// Conditional bars/step bar should follow "the user is interacting with the
// watch", not only whether the LED physically illuminated. In bright ambient
// light the OS may suppress the LED even though a raise/touch was recognized.
static bool s_interaction_active = false;
static AppTimer *s_interaction_timer = NULL;
static bool s_touch_subscribed = false;
#define INTERACTION_WINDOW_MS 4000

// ── Raise-to-wake gesture state ──────────────────────────────────────────────
// Pebble Z is perpendicular to the display. A normal glance holds the display
// much more vertically than "face up", so the read pose is detected primarily
// by a strong in-plane Y gravity component and a relatively small Z component.
//
// We do NOT require a specific starting pose. Instead we remember recent wrist
// motion, then trigger when that motion settles into the read pose.
static bool s_raise_accel_subscribed = false;
static uint64_t s_raise_last_wake_at = 0;
static uint64_t s_raise_motion_until = 0;
static uint8_t s_raise_face_samples = 0;
static bool s_raise_was_in_face_pose = false;
static bool s_raise_have_previous = false;
static int16_t s_raise_prev_x = 0;
static int16_t s_raise_prev_y = 0;
static int16_t s_raise_prev_z = 0;

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
  graphics_context_set_fill_color(ctx, s_settings.clock_color);
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
  graphics_context_set_fill_color(ctx, s_settings.clock_color);
  int ox    = H1_X + H1_ONE_X;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  draw_v(ctx, ox, oy,    mid_y - oy + STK);
  draw_v(ctx, ox, mid_y, bot_y - mid_y + STK);
}
static void draw_one(GContext *ctx, int cell_x, int oy) {
  graphics_context_set_fill_color(ctx, s_settings.clock_color);
  int ox    = cell_x + ONE_X_OFFSET;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  draw_v(ctx, ox, oy,    mid_y - oy + STK);
  draw_v(ctx, ox, mid_y, bot_y - mid_y + STK);
}
static void draw_colon(GContext *ctx, int ox, int oy) {
  graphics_context_set_fill_color(ctx, s_settings.clock_color);
  int cx      = ox + (COLON_WIDTH - COLON_DOT) / 2;
  int upper_y = oy + DIGIT_HEIGHT / 3 - COLON_DOT / 2;
  int lower_y = oy + (DIGIT_HEIGHT * 2) / 3 - COLON_DOT / 2;
  graphics_fill_rect(ctx, GRect(cx, upper_y, COLON_DOT, COLON_DOT), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(cx, lower_y, COLON_DOT, COLON_DOT), 0, GCornerNone);
}

static void draw_digit_24(GContext *ctx, int ox, int oy, int digit, int width) {
  if (digit < 0 || digit > 9) return;
  uint8_t s = DIGIT_SEGS[digit];
  graphics_context_set_fill_color(ctx, s_settings.clock_color);

  int top_y = oy;
  int mid_y = oy + HALF_V - H24_STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - H24_STK;
  int lx = ox;
  int rx = ox + width - H24_STK;

  if (s & SEG_TOP) graphics_fill_rect(ctx, GRect(ox, top_y, width, H24_STK), 0, GCornerNone);
  if (s & SEG_MID) graphics_fill_rect(ctx, GRect(ox, mid_y, width, H24_STK), 0, GCornerNone);
  if (s & SEG_BOT) graphics_fill_rect(ctx, GRect(ox, bot_y, width, H24_STK), 0, GCornerNone);
  if (s & SEG_TL) graphics_fill_rect(ctx, GRect(lx, top_y, H24_STK, mid_y - top_y + H24_STK), 0, GCornerNone);
  if (s & SEG_TR) graphics_fill_rect(ctx, GRect(rx, top_y, H24_STK, mid_y - top_y + H24_STK), 0, GCornerNone);
  if (s & SEG_BL) graphics_fill_rect(ctx, GRect(lx, mid_y, H24_STK, bot_y - mid_y + H24_STK), 0, GCornerNone);
  if (s & SEG_BR) graphics_fill_rect(ctx, GRect(rx, mid_y, H24_STK, bot_y - mid_y + H24_STK), 0, GCornerNone);
}

static void draw_one_24(GContext *ctx, int cell_x, int oy) {
  graphics_context_set_fill_color(ctx, s_settings.clock_color);

  // In 24-hour mode every "1" uses the same ONE_X_OFFSET (8px).
  // This keeps 01:11, 11:11, 21:11, etc. visually consistent.
  int ox = cell_x + ONE_X_OFFSET;
  int mid_y = oy + HALF_V - H24_STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - H24_STK;

  graphics_fill_rect(ctx, GRect(ox, oy, H24_STK, mid_y - oy + H24_STK), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(ox, mid_y, H24_STK, bot_y - mid_y + H24_STK), 0, GCornerNone);
}


// ── Weather icon ──────────────────────────────────────────────────────────────
static void tint_weather_bitmap(GBitmap *bitmap, GColor color) {
  if (!bitmap) return;

  const uint8_t rgb = color.argb & 0x3F;
  const GBitmapFormat format = gbitmap_get_format(bitmap);

  if (format == GBitmapFormat8Bit) {
    GRect bounds = gbitmap_get_bounds(bitmap);
    for (int y = bounds.origin.y; y < bounds.origin.y + bounds.size.h; ++y) {
      GBitmapDataRowInfo row = gbitmap_get_data_row_info(bitmap, y);
      for (int x = row.min_x; x <= row.max_x; ++x) {
        uint8_t *pixel = row.data + x;
        const uint8_t alpha = *pixel & 0xC0;
        if (alpha) *pixel = alpha | rgb;
      }
    }
    return;
  }

  // Also support palettized resources if CloudPebble chooses a compact format.
  GColor *palette = gbitmap_get_palette(bitmap);
  if (!palette) return;

  int palette_size = 0;
  if (format == GBitmapFormat1BitPalette) palette_size = 2;
  else if (format == GBitmapFormat2BitPalette) palette_size = 4;
  else if (format == GBitmapFormat4BitPalette) palette_size = 16;

  for (int i = 0; i < palette_size; ++i) {
    const uint8_t alpha = palette[i].argb & 0xC0;
    if (alpha) palette[i].argb = alpha | rgb;
  }
}

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
  tint_weather_bitmap(s_weather_icon_bitmap, gcolor_legible_over(s_settings.background_color));
  if (s_weather_icon_left_layer) bitmap_layer_set_bitmap(s_weather_icon_left_layer, s_weather_icon_bitmap);
  if (s_weather_icon_right_layer) bitmap_layer_set_bitmap(s_weather_icon_right_layer, s_weather_icon_bitmap);
}
static int round_tenths_to_int(int value_x10) {
  return value_x10 >= 0 ? (value_x10 + 5) / 10 : (value_x10 - 5) / 10;
}

static void update_temperature_text(void) {
  if (!s_have_temperature) {
    snprintf(s_weather_buf, sizeof(s_weather_buf), "--");
    return;
  }

  int display_x10 = s_temperature_c_x10;
  if (s_settings.temp_unit == TEMP_FAHRENHEIT) {
    display_x10 = (s_temperature_c_x10 * 9) / 5 + 320;
  }

  int display_temp = round_tenths_to_int(display_x10);
  snprintf(s_weather_buf, sizeof(s_weather_buf), "%d\xC2\xB0", display_temp);
}

static int display_temp_from_c_x10(int c_x10) {
  int value_x10 = c_x10;
  if (s_settings.temp_unit == TEMP_FAHRENHEIT) {
    value_x10 = (c_x10 * 9) / 5 + 320;
  }
  return round_tenths_to_int(value_x10);
}

static void format_solar_time(int minute_of_day, char *buffer, size_t buffer_size) {
  if (minute_of_day < 0) {
    snprintf(buffer, buffer_size, "--");
    return;
  }

  int hour = (minute_of_day / 60) % 24;
  int minute = minute_of_day % 60;
  if (s_settings.time_format == TIME_FORMAT_24H) {
    snprintf(buffer, buffer_size, "%02d:%02d", hour, minute);
  } else {
    int hour12 = hour % 12;
    if (hour12 == 0) hour12 = 12;
    snprintf(buffer, buffer_size, "%d:%02d", hour12, minute);
  }
}

static const char *distance_text(void) {
  if (s_settings.temp_unit == TEMP_CELSIUS) {
    int km_x10 = (s_distance_m + 50) / 100;
    snprintf(s_distance_buf, sizeof(s_distance_buf), "%d.%dKM",
             km_x10 / 10, km_x10 % 10);
  } else {
    int miles_x10 = (s_distance_m * 10 + 804) / 1609;
    snprintf(s_distance_buf, sizeof(s_distance_buf), "%d.%dMI",
             miles_x10 / 10, miles_x10 % 10);
  }
  return s_distance_buf;
}

static const char *high_low_text(void) {
  if (!s_have_high_low) {
    snprintf(s_high_low_buf, sizeof(s_high_low_buf), "--/--");
    return s_high_low_buf;
  }

  int high = display_temp_from_c_x10(s_high_c_x10);
  int low = display_temp_from_c_x10(s_low_c_x10);
  snprintf(s_high_low_buf, sizeof(s_high_low_buf), "%d/%d\xC2\xB0", high, low);
  return s_high_low_buf;
}

// ── Clock ─────────────────────────────────────────────────────────────────────
static void clock_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_settings.background_color);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  int h1 = s_hour / 10;
  int h2 = s_hour % 10;
  int m1 = s_minute / 10;
  int m2 = s_minute % 10;
  int sy = (b.size.h - DIGIT_HEIGHT) / 2;
  // Permanently hidden step bar uses the expanded clock area, biased 3 px upward.
  // Backlight-only modes reserve their step-bar space at all times so the clock
  // never jumps when the backlight turns the bar on or off.
  if (s_settings.stepbar_mode == STEPBAR_HIDDEN) sy -= 3;
  if (s_settings.time_format == TIME_FORMAT_24H) {
    // In 24-hour mode every numeral uses the same full-width seven-segment
    // geometry. In particular, "1" is no longer centered as a narrow special
    // case; its right-hand segments occupy the normal digit cell width.
    // Keep every numeral in the same allocated 24-hour cell width, but draw
    // "1" with the same intentional left-biased offsets as the 12-hour face.
    if (h1 == 1) draw_one_24(ctx, H24_H1_X, sy);
    else draw_digit_24(ctx, H24_H1_X, sy, h1, H24_DIGIT_WIDTH);

    if (h2 == 1) draw_one_24(ctx, H24_H2_X, sy);
    else draw_digit_24(ctx, H24_H2_X, sy, h2, H24_DIGIT_WIDTH);

    draw_colon(ctx, H24_COL_X, sy);

    if (m1 == 1) draw_one_24(ctx, H24_M1_X, sy);
    else draw_digit_24(ctx, H24_M1_X, sy, m1, H24_DIGIT_WIDTH);

    if (m2 == 1) draw_one_24(ctx, H24_M2_X, sy);
    else draw_digit_24(ctx, H24_M2_X, sy, m2, H24_DIGIT_WIDTH);
  } else {
    const bool center_three_digit = s_settings.center_12h && h1 == 0;
    const int h2_x = center_three_digit ? CENTER12_H2_X : H2_X;
    const int col_x = center_three_digit ? CENTER12_COL_X : COL_X;
    const int m1_x = center_three_digit ? CENTER12_M1_X : M1_X;
    const int m2_x = center_three_digit ? CENTER12_M2_X : M2_X;

    if (h1 == 1) draw_one_h1(ctx, sy);
    if (h2 == 1) draw_one(ctx, h2_x, sy); else draw_digit(ctx, h2_x, sy, h2);
    draw_colon(ctx, col_x, sy);
    if (m1 == 1) draw_one(ctx, m1_x, sy); else draw_digit(ctx, m1_x, sy, m1);
    if (m2 == 1) draw_one(ctx, m2_x, sy); else draw_digit(ctx, m2_x, sy, m2);
  }
}

static void draw_battery_icon(GContext *ctx, GRect r, int percent, GColor color);
static void draw_bluetooth_icon(GContext *ctx, GPoint c, int width, int height, GColor color, bool connected);

// ── Header ────────────────────────────────────────────────────────────────────
// Datebox starts at y=0 (top of screen) and extends to the underline row.
// The header layer itself starts at y=0 so DATEBOX_Y=0 reaches the screen top.
static void header_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_settings.background_color);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(DATEBOX_X, DATEBOX_Y, DATEBOX_W, DATEBOX_H), 0, GCornerNone);
  int line_y = DATEBOX_Y + DATEBOX_H - 2;
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(0, line_y, DATEBOX_X - UNDERLINE_GAP, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(DATEBOX_X + DATEBOX_W + UNDERLINE_GAP, line_y, SCREEN_W - DATEBOX_X - DATEBOX_W - UNDERLINE_GAP, 2), 0, GCornerNone);

  GColor side_fg = gcolor_legible_over(s_settings.background_color);
  GColor center_fg = gcolor_legible_over(s_settings.accent_color);
  GRect left_area = GRect(4, 0, DATEBOX_X - BOX_GAP - 4, HEADER_H);
  int right_x = DATEBOX_X + DATEBOX_W + BOX_GAP;
  GRect right_area = GRect(right_x, 0, SCREEN_W - right_x - 4, HEADER_H);
  if (s_settings.top_left_slot == SLOT_BATTERY)
    draw_battery_icon(ctx, GRect(left_area.origin.x, 20, 36, 9), s_battery_percent, side_fg);
  else if (s_settings.top_left_slot == SLOT_BLUETOOTH && s_bluetooth_connected)
    draw_bluetooth_icon(ctx, GPoint(left_area.origin.x + left_area.size.w/2, 27), 34, 30, side_fg, true);
  if (s_settings.top_center_slot == SLOT_BATTERY)
    draw_battery_icon(ctx, GRect(DATEBOX_X + 7, 7, 36, 9), s_battery_percent, center_fg);
  else if (s_settings.top_center_slot == SLOT_BLUETOOTH && s_bluetooth_connected)
    draw_bluetooth_icon(ctx, GPoint(DATEBOX_X + DATEBOX_W/2, 27), 34, 30, center_fg, true);
  if (s_settings.top_right_slot == SLOT_BATTERY)
    draw_battery_icon(ctx, GRect(right_area.origin.x + right_area.size.w - 36, 20, 36, 9), s_battery_percent, side_fg);
  else if (s_settings.top_right_slot == SLOT_BLUETOOTH && s_bluetooth_connected)
    draw_bluetooth_icon(ctx, GPoint(right_area.origin.x + right_area.size.w/2, 27), 34, 30, side_fg, true);
}

// ── Step bar ──────────────────────────────────────────────────────────────────
static void stepbar_update_proc(Layer *layer, GContext *ctx) {
  GRect b   = layer_get_bounds(layer);
  int bar_w = b.size.w - BAR_MARGIN * 2;
  int bar_x = BAR_MARGIN;
  int cy    = b.size.h / 2;
  graphics_context_set_fill_color(ctx, s_settings.background_color);
  graphics_fill_rect(ctx, b, 0, GCornerNone);

  if (s_settings.stepbar_mode == STEPBAR_HIDDEN) return;

  int steps = s_step_count < 0 ? 0 : s_step_count;
  int fill_w = (steps >= s_settings.step_goal) ? bar_w : (steps * bar_w / s_settings.step_goal);

  if (steps >= s_settings.step_goal) {
    // Goal reached: celebrate by switching the completed bar to the accent color.
    graphics_context_set_fill_color(ctx, s_settings.accent_color);
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
  graphics_context_set_fill_color(ctx, gcolor_legible_over(s_settings.background_color));
  if (stepbar_is_left_to_right()) {
    graphics_fill_rect(ctx, GRect(bar_x, cy - 1, fill_w, 4), 0, GCornerNone);
  } else {
    int half_fill = fill_w / 2;
    int bar_cx = bar_x + bar_w / 2;
    graphics_fill_rect(ctx, GRect(bar_cx - half_fill, cy - 1, half_fill * 2, 4), 0, GCornerNone);
  }
}

// Reflow the clock and step-progress zones when the bar moves or is hidden.
// With the step bar hidden, the clock gets the full space between header/footer,
// so its existing centering math automatically centers the digits vertically.
static bool stepbar_is_backlight_only(void) {
  return s_settings.stepbar_mode >= STEPBAR_MIRRORED_BACKLIGHT &&
         s_settings.stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT;
}

static bool stepbar_is_above(void) {
  return s_settings.stepbar_mode == STEPBAR_MIRRORED_ABOVE ||
         s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT_ABOVE ||
         s_settings.stepbar_mode == STEPBAR_MIRRORED_ABOVE_BACKLIGHT ||
         s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT;
}

static bool stepbar_is_left_to_right(void) {
  return s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT ||
         s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT_ABOVE ||
         s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT_BACKLIGHT ||
         s_settings.stepbar_mode == STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT;
}

// With the step bar hidden (either permanently or while the backlight is off),
// the clock gets the full space between header/footer and stays vertically centered.
static void update_stepbar_layout(void) {
  if (!s_clock_layer || !s_stepbar_layer) return;

  const int available_h = SCREEN_H - HEADER_H - FOOTER_H;
  const bool permanently_hidden = s_settings.stepbar_mode == STEPBAR_HIDDEN;
  const bool backlight_only = stepbar_is_backlight_only();
  const bool interaction_visible = s_backlight_on || s_interaction_active;
  const bool bar_visible = !permanently_hidden && (!backlight_only || interaction_visible);

  if (permanently_hidden) {
    // Only the true Hidden mode gives the clock the step-bar space.
    layer_set_hidden(s_stepbar_layer, true);
    layer_set_frame(s_clock_layer, GRect(0, HEADER_H, SCREEN_W, available_h));
  } else if (stepbar_is_above()) {
    // Raise the above-time bar 5 px. The clock begins 1 px below the bar,
    // moving the time 5 px upward versus v2.1.6. Backlight-only modes keep
    // these frames fixed so the time never jumps.
    const int16_t above_bar_y = HEADER_H - 5;
    const int16_t above_clock_y = HEADER_H + STEPBAR_H - 4;
    layer_set_hidden(s_stepbar_layer, !bar_visible);
    layer_set_frame(s_stepbar_layer, GRect(0, above_bar_y, SCREEN_W, STEPBAR_H));
    layer_set_frame(s_clock_layer,
                    GRect(0, above_clock_y, SCREEN_W, available_h - STEPBAR_H - 1));
  } else {
    // Likewise reserve the below-bar space for backlight-only modes.
    layer_set_hidden(s_stepbar_layer, !bar_visible);
    layer_set_frame(s_clock_layer, GRect(0, HEADER_H, SCREEN_W, available_h - STEPBAR_H));
    layer_set_frame(s_stepbar_layer,
                    GRect(0, HEADER_H + available_h - STEPBAR_H, SCREEN_W, STEPBAR_H));
  }

  layer_mark_dirty(s_clock_layer);
  if (!layer_get_hidden(s_stepbar_layer)) layer_mark_dirty(s_stepbar_layer);
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
  graphics_context_set_fill_color(ctx, s_settings.background_color);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_settings.accent_color);
  graphics_fill_rect(ctx, GRect(HRBOX_X, HRBOX_Y, BOX_W, b.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, 0, HRBOX_X - BOX_GAP, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(HRBOX_X + BOX_W + BOX_GAP, 0, SCREEN_W - HRBOX_X - BOX_W - BOX_GAP, 2), 0, GCornerNone);

  GColor center_fg = gcolor_legible_over(s_settings.accent_color);
  GRect left_area = GRect(4, 0, HRBOX_X - BOX_GAP - 4, 14);
  int right_x = HRBOX_X + BOX_W + BOX_GAP;
  GRect right_area = GRect(right_x, 0, SCREEN_W - right_x - 4, 14);
  draw_slot_icon(ctx, s_settings.left_slot, left_area, gcolor_legible_over(s_settings.background_color), false);
  draw_center_icon(ctx, s_settings.center_slot, center_fg);
  draw_slot_icon(ctx, s_settings.right_slot, right_area, gcolor_legible_over(s_settings.background_color), true);
}

static void update_time(struct tm *tick_time);

static const char *side_slot_label(uint8_t slot) {
  switch (slot) {
    case SLOT_STEPS: return "STEPS";
    case SLOT_BATTERY: return "";
    case SLOT_HEART_RATE: return "HR";
    case SLOT_BLUETOOTH: return s_bluetooth_connected ? "" : "BT";
    case SLOT_DAY: return "DAY";
    case SLOT_DATE: return "DATE";
    case SLOT_MONTH: return "MONTH";
    case SLOT_CALORIES: return "CAL";
    case SLOT_DISTANCE: return "DIST";
    case SLOT_SUNRISE: return "RISE";
    case SLOT_SUNSET: return "SET";
    case SLOT_HIGH_LOW: return "H/L";
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
    case SLOT_DAY:
      return s_day_buf;
    case SLOT_DATE: {
      char *d = s_date_buf; while (*d == ' ') d++;
      return d;
    }
    case SLOT_MONTH:
      return s_month_buf;
    case SLOT_CALORIES:
      snprintf(s_calories_buf, sizeof(s_calories_buf), "%d", s_active_kcal);
      return s_calories_buf;
    case SLOT_DISTANCE:
      return distance_text();
    case SLOT_SUNRISE:
      format_solar_time(s_sunrise_minute, s_sunrise_buf, sizeof(s_sunrise_buf));
      return s_sunrise_buf;
    case SLOT_SUNSET:
      format_solar_time(s_sunset_minute, s_sunset_buf, sizeof(s_sunset_buf));
      return s_sunset_buf;
    case SLOT_HIGH_LOW:
      return high_low_text();
    case SLOT_WEATHER:
    default:
      return s_weather_buf;
  }
}

static const char *center_slot_label(void) {
  switch (s_settings.center_slot) {
    case CENTER_BATTERY: return "";
    case CENTER_BLUETOOTH: return s_bluetooth_connected ? "" : "BT";
    case CENTER_WEATHER: return "TEMP";
    case CENTER_STEPS: return "STEPS";
    case CENTER_DATE: return "DATE";
    case CENTER_HEART_RATE:
    default: return "HR";
  }
}

static const char *center_slot_value(void) {
  switch (s_settings.center_slot) {
    case CENTER_BATTERY:
      if (s_battery_percent == 100) snprintf(s_battery_buf, sizeof(s_battery_buf), "100");
      else snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", s_battery_percent);
      return s_battery_buf;
    case CENTER_BLUETOOTH: return "";
    case CENTER_WEATHER: return s_weather_buf;
    case CENTER_STEPS:
      snprintf(s_steps_buf, sizeof(s_steps_buf), "%d", s_step_count); return s_steps_buf;
    case CENTER_DATE: { char *d = s_date_buf; while (*d == ' ') d++; return d; }
    case CENTER_HEART_RATE:
    default:
      if (s_heart_rate > 0) snprintf(s_hr_buf, sizeof(s_hr_buf), "%d", s_heart_rate);
      else snprintf(s_hr_buf, sizeof(s_hr_buf), "--");
      return s_hr_buf;
  }
}

static bool slot_is_calendar(uint8_t slot) {
  return slot == SLOT_DAY || slot == SLOT_DATE || slot == SLOT_MONTH;
}

static const char *top_slot_label(uint8_t slot) {
  // Preserve the original clean DAY / DATE / MONTH header look: calendar
  // items use the large value only, without a redundant label above them.
  return slot_is_calendar(slot) ? "" : side_slot_label(slot);
}

static const char *top_slot_value(uint8_t slot) {
  return side_slot_value(slot);
}

static void update_header_content(void) {
  if (!s_top_left_label || !s_top_center_label || !s_top_right_label) return;
  text_layer_set_text(s_top_left_label, top_slot_label(s_settings.top_left_slot));
  text_layer_set_text(s_top_left_val, top_slot_value(s_settings.top_left_slot));
  text_layer_set_text(
      s_top_center_label,
      s_settings.top_center_slot == SLOT_WEATHER ? "TEMP" : top_slot_label(s_settings.top_center_slot));
  text_layer_set_text(s_top_center_val, top_slot_value(s_settings.top_center_slot));
  text_layer_set_text(s_top_right_label, top_slot_label(s_settings.top_right_slot));
  text_layer_set_text(s_top_right_val, top_slot_value(s_settings.top_right_slot));

  // Calendar items retain the original large header typography. Other data
  // uses the footer value font so labels, icons, and values fit comfortably.
  text_layer_set_font(s_top_left_val, slot_is_calendar(s_settings.top_left_slot) ? s_font_header : s_font_value);
  text_layer_set_font(s_top_center_val, slot_is_calendar(s_settings.top_center_slot) ? s_font_header : s_font_value);
  text_layer_set_font(s_top_right_val, slot_is_calendar(s_settings.top_right_slot) ? s_font_header : s_font_value);

  // Calendar values have no label, so give them the full header height.
  // This restores the original vertical centering instead of leaving them in
  // the lower "value" half of the generic label/value layout.
  layer_set_frame(text_layer_get_layer(s_top_left_val),
      GRect(4, slot_is_calendar(s_settings.top_left_slot) ? 4 : 15, layer_get_bounds(s_header_layer).size.w > 0 ? DATEBOX_X - BOX_GAP - 4 : 0,
            slot_is_calendar(s_settings.top_left_slot) ? HEADER_H - 5 : 34));
  layer_set_frame(text_layer_get_layer(s_top_center_val),
      GRect(DATEBOX_X,
            slot_is_calendar(s_settings.top_center_slot) ? 4 :
              (s_settings.top_center_slot == SLOT_BATTERY ? 14 : 15),
            DATEBOX_W,
            slot_is_calendar(s_settings.top_center_slot) ? HEADER_H - 5 :
              (s_settings.top_center_slot == SLOT_BATTERY ? 38 : 34)));
  int top_right_x = DATEBOX_X + DATEBOX_W + BOX_GAP;
  layer_set_frame(text_layer_get_layer(s_top_right_val),
      GRect(top_right_x, slot_is_calendar(s_settings.top_right_slot) ? 4 : 15, SCREEN_W - top_right_x - 4,
            slot_is_calendar(s_settings.top_right_slot) ? HEADER_H - 5 : 34));

  text_layer_set_text_alignment(s_top_left_label,
      (s_settings.top_left_slot == SLOT_BLUETOOTH && !s_bluetooth_connected) ? GTextAlignmentCenter : GTextAlignmentLeft);
  text_layer_set_text_alignment(s_top_left_val,
      slot_is_calendar(s_settings.top_left_slot) ? GTextAlignmentCenter : GTextAlignmentLeft);
  text_layer_set_text_alignment(s_top_center_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_top_center_val, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_top_right_label,
      (s_settings.top_right_slot == SLOT_BLUETOOTH && !s_bluetooth_connected) ? GTextAlignmentCenter : GTextAlignmentRight);
  text_layer_set_text_alignment(s_top_right_val,
      slot_is_calendar(s_settings.top_right_slot) ? GTextAlignmentCenter : GTextAlignmentRight);
  if (s_header_layer) layer_mark_dirty(s_header_layer);
}

static void update_footer_content(void) {
  if (!s_left_label || !s_center_label || !s_right_label ||
      !s_weather_icon_left_layer || !s_weather_icon_right_layer) return;

  const bool left_calendar = slot_is_calendar(s_settings.left_slot);
  const bool center_calendar = slot_is_calendar(s_settings.center_slot);
  const bool right_calendar = slot_is_calendar(s_settings.right_slot);

  // Match the header treatment for calendar items: large value only, no
  // redundant DAY / DATE / MONTH label.
  text_layer_set_text(s_left_label, left_calendar ? "" : side_slot_label(s_settings.left_slot));
  text_layer_set_text(s_left_val, side_slot_value(s_settings.left_slot));
  text_layer_set_text(s_center_label, center_calendar ? "" : center_slot_label());
  text_layer_set_text(s_center_val, center_slot_value());
  text_layer_set_text(s_right_label, right_calendar ? "" : side_slot_label(s_settings.right_slot));
  text_layer_set_text(s_right_val, side_slot_value(s_settings.right_slot));

  // Calendar items use the same large custom font as their header versions.
  text_layer_set_font(s_left_val, left_calendar ? s_font_header : s_font_value);
  text_layer_set_font(s_center_val, center_calendar ? s_font_header : s_font_value);
  text_layer_set_font(s_right_val, right_calendar ? s_font_header : s_font_value);

  // Give calendar values the whole footer slot so they can be centered
  // vertically as well as horizontally. Non-calendar items retain the existing
  // label/value frames.
  int left_w = HRBOX_X - BOX_GAP - 4;
  int right_x = HRBOX_X + BOX_W + BOX_GAP;
  int right_w = SCREEN_W - right_x - 4;

  layer_set_frame(text_layer_get_layer(s_left_val),
      GRect(4, left_calendar ? 1 : 14, left_w, left_calendar ? FOOTER_H - 1 : 38));
  layer_set_frame(text_layer_get_layer(s_center_val),
      GRect(HRBOX_X, center_calendar ? 1 : 14, BOX_W, center_calendar ? FOOTER_H - 1 : 38));
  layer_set_frame(text_layer_get_layer(s_right_val),
      GRect(right_x, right_calendar ? 1 : 14, right_w, right_calendar ? FOOTER_H - 1 : 38));

  // Calendar values are centered in every slot. Other values keep their
  // established left/center/right alignment.
  text_layer_set_text_alignment(
      s_left_label,
      (s_settings.left_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentLeft);
  text_layer_set_text_alignment(
      s_left_val, left_calendar ? GTextAlignmentCenter : GTextAlignmentLeft);

  text_layer_set_text_alignment(s_center_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_center_val, GTextAlignmentCenter);

  text_layer_set_text_alignment(
      s_right_label,
      (s_settings.right_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentRight);
  text_layer_set_text_alignment(
      s_right_val, right_calendar ? GTextAlignmentCenter : GTextAlignmentRight);

  bool weather_left = s_settings.left_slot == SLOT_WEATHER;
  bool weather_right = s_settings.right_slot == SLOT_WEATHER;
  layer_set_hidden(bitmap_layer_get_layer(s_weather_icon_left_layer), !weather_left);
  layer_set_hidden(bitmap_layer_get_layer(s_weather_icon_right_layer), !weather_right);
  if (s_footer_layer) layer_mark_dirty(s_footer_layer);
}

static void apply_bar_visibility(void);

static bool interaction_should_be_visible(void) {
  return s_backlight_on || s_interaction_active;
}

static void interaction_timeout_handler(void *context) {
  s_interaction_timer = NULL;
  s_interaction_active = false;

  // Re-sample the actual LED state in case a physical backlight interval is
  // still active after our logical interaction timer expires.
  s_backlight_on = light_is_on();
  apply_bar_visibility();
  update_stepbar_layout();
}

static void begin_interaction_window(void) {
  s_interaction_active = true;

  if (s_interaction_timer) {
    if (!app_timer_reschedule(s_interaction_timer, INTERACTION_WINDOW_MS)) {
      s_interaction_timer = app_timer_register(
          INTERACTION_WINDOW_MS, interaction_timeout_handler, NULL);
    }
  } else {
    s_interaction_timer = app_timer_register(
        INTERACTION_WINDOW_MS, interaction_timeout_handler, NULL);
  }

  apply_bar_visibility();
  update_stepbar_layout();
}

static void touch_handler(const TouchEvent *event, void *context) {
  if (!event || event->type != TouchEvent_Touchdown) return;

  // Treat a screen touch exactly like an attempted backlight interaction.
  // This keeps conditional data visible even when ambient-light logic decides
  // the LED itself is unnecessary.
  begin_interaction_window();
  light_enable_interaction();
}

static void apply_bar_visibility(void) {
  if (s_header_layer) {
    const bool header_visible =
        (s_settings.header_mode == BAR_ALWAYS) ||
        (s_settings.header_mode == BAR_BACKLIGHT && interaction_should_be_visible());
    layer_set_hidden(s_header_layer, !header_visible);
  }
  if (s_footer_layer) {
    const bool footer_visible =
        (s_settings.footer_mode == BAR_ALWAYS) ||
        (s_settings.footer_mode == BAR_BACKLIGHT && interaction_should_be_visible());
    layer_set_hidden(s_footer_layer, !footer_visible);
  }
}

static void backlight_handler(bool on) {
  s_backlight_on = on;
  apply_bar_visibility();
  update_stepbar_layout();
}

static void focus_handler(bool in_focus) {
  if (!in_focus) return;

  // BacklightService is edge-triggered. If the user leaves the watchface while
  // the light is on and returns before it switches off, no new "on" edge occurs.
  // Query the current hardware state whenever the watchface regains focus so
  // backlight-controlled bars immediately match what the user can actually see.
  s_backlight_on = light_is_on();
  apply_bar_visibility();
  update_stepbar_layout();
}

static void update_bar_input_services(void) {
  const bool want_backlight =
      s_settings.footer_mode == BAR_BACKLIGHT ||
      s_settings.header_mode == BAR_BACKLIGHT ||
      stepbar_is_backlight_only();

  if (want_backlight && !s_backlight_subscribed) {
    backlight_service_subscribe(backlight_handler);
    s_backlight_subscribed = true;
  } else if (!want_backlight && s_backlight_subscribed) {
    backlight_service_unsubscribe();
    s_backlight_subscribed = false;
  }

  // On PT2, touchscreen intent is useful even when ambient light suppresses
  // the physical LED. Subscribe only when some UI actually depends on it.
  if (want_backlight && !s_touch_subscribed) {
    touch_service_subscribe(touch_handler, NULL);
    s_touch_subscribed = true;
  } else if (!want_backlight && s_touch_subscribed) {
    touch_service_unsubscribe();
    s_touch_subscribed = false;
  }

  // Synchronize immediately rather than waiting for the next on/off transition.
  s_backlight_on = light_is_on();
  apply_bar_visibility();
  update_stepbar_layout();
}

static int16_t abs_i16(int16_t v) {
  return v < 0 ? (int16_t)-v : v;
}

static bool accel_sample_is_stable_gravity(const AccelData *sample) {
  // Pebble accelerometer units are milli-G. Keep only samples reasonably close
  // to 1 g so a hard bump or arm swing doesn't masquerade as an orientation.
  int32_t x = sample->x;
  int32_t y = sample->y;
  int32_t z = sample->z;
  int32_t mag2 = x * x + y * y + z * z;
  return mag2 >= 650000 && mag2 <= 1400000;
}

static bool raise_read_pose(const AccelData *sample) {
  int16_t ax = abs_i16(sample->x);
  int16_t az = abs_i16(sample->z);

  // Real-world PT2 logs show the wearer-facing read pose has NEGATIVE Y,
  // while flipping the display away produces the mirror-image POSITIVE Y.
  // Preserve that sign instead of using abs(Y), so only the wearer-facing
  // orientation can satisfy the gesture.
  int16_t min_negative_y =
      (s_settings.raise_wake_mode == RAISE_WAKE_SENSITIVE) ? -560 : -680;
  int16_t max_z =
      (s_settings.raise_wake_mode == RAISE_WAKE_SENSITIVE) ? 700 : 560;
  int16_t max_x =
      (s_settings.raise_wake_mode == RAISE_WAKE_SENSITIVE) ? 780 : 680;

  return sample->y <= min_negative_y &&
         az <= max_z &&
         ax <= max_x &&
         accel_sample_is_stable_gravity(sample);
}

static void raise_wake_accel_handler(AccelData *data, uint32_t num_samples) {
  if (s_settings.raise_wake_mode == RAISE_WAKE_OFF || !data || num_samples == 0) return;

  const int32_t motion_threshold =
      (s_settings.raise_wake_mode == RAISE_WAKE_SENSITIVE) ? 120 : 210;
  const uint64_t motion_memory_ms =
      (s_settings.raise_wake_mode == RAISE_WAKE_SENSITIVE) ? 1400 : 950;
  const uint64_t cooldown_ms = 3500;

  for (uint32_t i = 0; i < num_samples; ++i) {
    AccelData *sample = &data[i];
    if (sample->did_vibrate) {
      s_raise_have_previous = false;
      continue;
    }

    uint64_t now_ms = sample->timestamp;

    // Detect meaningful wrist motion from sample-to-sample vector change.
    // This provides the "suddenly" part of raise-to-wake without assuming the
    // wrist was previously hanging at the user's side.
    if (s_raise_have_previous) {
      int32_t dx = (int32_t)sample->x - s_raise_prev_x;
      int32_t dy = (int32_t)sample->y - s_raise_prev_y;
      int32_t dz = (int32_t)sample->z - s_raise_prev_z;
      int32_t delta2 = dx * dx + dy * dy + dz * dz;
      if (delta2 >= motion_threshold * motion_threshold) {
        s_raise_motion_until = now_ms + motion_memory_ms;
      }
    }

    s_raise_prev_x = sample->x;
    s_raise_prev_y = sample->y;
    s_raise_prev_z = sample->z;
    s_raise_have_previous = true;

    bool face_pose = raise_read_pose(sample);

    if (!face_pose) {
      s_raise_face_samples = 0;
      s_raise_was_in_face_pose = false;
      continue;
    }

    // Already sitting in the read pose should not repeatedly retrigger.
    if (s_raise_was_in_face_pose) continue;

    if (s_raise_face_samples < 3) s_raise_face_samples++;

    // Two consecutive 10 Hz samples ~= 0.2 seconds of stable read pose.
    if (s_raise_face_samples >= 2) {
      bool recent_motion = now_ms <= s_raise_motion_until;
      bool cooldown_done =
          s_raise_last_wake_at == 0 || now_ms - s_raise_last_wake_at >= cooldown_ms;

      if (recent_motion && cooldown_done) {
        // Mark the UI as actively viewed even if ambient-light logic suppresses
        // the LED. This is what allows backlight-only bars to work outdoors.
        begin_interaction_window();

        if (!light_is_on()) {
          light_enable_interaction();
        }

        s_raise_last_wake_at = now_ms;
        APP_LOG(APP_LOG_LEVEL_INFO,
                "Raise wake: mode=%d x=%d y=%d z=%d logical=1 light=%d",
                (int)s_settings.raise_wake_mode,
                (int)sample->x, (int)sample->y, (int)sample->z,
                light_is_on() ? 1 : 0);
      }

      // Treat this as one entry into the pose whether it woke the light or not.
      // It must leave the read pose before another gesture can be recognized.
      s_raise_was_in_face_pose = true;
      s_raise_face_samples = 0;
    }
  }
}

static void reset_raise_wake_state(void) {
  s_raise_motion_until = 0;
  s_raise_face_samples = 0;
  s_raise_was_in_face_pose = false;
  s_raise_have_previous = false;
}

static void update_raise_wake_service(void) {
  bool want_accel = s_settings.raise_wake_mode != RAISE_WAKE_OFF;

  if (want_accel && !s_raise_accel_subscribed) {
    // Keep the sensor at 10 Hz for reliable orientation data, but batch ten
    // samples so the application wakes only once per second instead of twice.
    // This is our first battery-saving pass without lowering sensor fidelity.
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    accel_data_service_subscribe(10, raise_wake_accel_handler);
    s_raise_accel_subscribed = true;
    reset_raise_wake_state();
    APP_LOG(APP_LOG_LEVEL_INFO, "Raise wake accelerometer enabled");
  } else if (!want_accel && s_raise_accel_subscribed) {
    accel_data_service_unsubscribe();
    s_raise_accel_subscribed = false;
    reset_raise_wake_state();
    APP_LOG(APP_LOG_LEVEL_INFO, "Raise wake accelerometer disabled");
  } else if (want_accel) {
    // Mode changed while already subscribed (Normal <-> Sensitive).
    reset_raise_wake_state();
  }
}

static void battery_handler(BatteryChargeState charge) {
  s_battery_percent = charge.charge_percent;
  update_footer_content();
  update_header_content();
}

static void connection_handler(bool connected) {
  s_bluetooth_connected = connected;
  update_footer_content();
  update_header_content();
}

// ── Accent color helper ─────────────────────────────────────────────────────
static void update_accent_text_contrast(void) {
  GColor text_color = gcolor_legible_over(s_settings.accent_color);

  if (s_top_center_label) text_layer_set_text_color(s_top_center_label, text_color);
  if (s_top_center_val) text_layer_set_text_color(s_top_center_val, text_color);
  if (s_center_label) text_layer_set_text_color(s_center_label, text_color);
  if (s_center_val) text_layer_set_text_color(s_center_val, text_color);
}

static void update_background_contrast(void) {
  GColor fg = gcolor_legible_over(s_settings.background_color);

  if (s_top_left_label) text_layer_set_text_color(s_top_left_label, fg);
  if (s_top_left_val) text_layer_set_text_color(s_top_left_val, fg);
  if (s_top_right_label) text_layer_set_text_color(s_top_right_label, fg);
  if (s_top_right_val) text_layer_set_text_color(s_top_right_val, fg);
  if (s_left_label) text_layer_set_text_color(s_left_label, fg);
  if (s_left_val) text_layer_set_text_color(s_left_val, fg);
  if (s_right_label) text_layer_set_text_color(s_right_label, fg);
  if (s_right_val) text_layer_set_text_color(s_right_val, fg);

  if (s_window) window_set_background_color(s_window, s_settings.background_color);
  update_weather_icon(s_weather_icon);

  if (s_header_layer) layer_mark_dirty(s_header_layer);
  if (s_clock_layer) layer_mark_dirty(s_clock_layer);
  if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
  if (s_footer_layer) layer_mark_dirty(s_footer_layer);
}


static bool trial_has_been_used(void) {
  return persist_exists(TRIAL_USED_PERSIST_KEY) && persist_read_bool(TRIAL_USED_PERSIST_KEY);
}

static time_t trial_start_time(void) {
  if (!persist_exists(TRIAL_START_PERSIST_KEY)) return 0;
  return (time_t)persist_read_int(TRIAL_START_PERSIST_KEY);
}

static uint32_t trial_remaining_seconds(void) {
  if (!s_trial_active) return 0;
  time_t start = trial_start_time();
  if (start <= 0) return 0;
  time_t now = time(NULL);
  if (now <= start) return PRO_TRIAL_SECONDS;
  uint32_t elapsed = (uint32_t)(now - start);
  return elapsed >= PRO_TRIAL_SECONDS ? 0 : (PRO_TRIAL_SECONDS - elapsed);
}

static uint8_t trial_state_code(void) {
  // 0 = available, 1 = active, 2 = used/expired, 3 = permanently licensed
  if (s_kiezelpay_licensed) return 3;
  if (s_trial_active) return 1;
  return trial_has_been_used() ? 2 : 0;
}

static void pro_settings_save_current(void) {
  persist_write_data(PRO_SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
}

static bool pro_settings_restore_saved(void) {
  if (!persist_exists(PRO_SETTINGS_PERSIST_KEY) ||
      persist_get_size(PRO_SETTINGS_PERSIST_KEY) != (int)sizeof(s_settings)) {
    return false;
  }

  WatchfaceSettings saved;
  if (persist_read_data(PRO_SETTINGS_PERSIST_KEY, &saved, sizeof(saved)) != (int)sizeof(saved) ||
      !settings_values_valid(&saved)) {
    return false;
  }

  // Free clock choices remain the user's choices even while Pro is inactive.
  uint8_t keep_time_format = s_settings.time_format;
  uint8_t keep_center_12h = s_settings.center_12h;
  s_settings = saved;
  s_settings.time_format = keep_time_format;
  s_settings.center_12h = keep_center_12h;
  return true;
}

static void license_send_status_to_phone(void) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK || !iter) return;
  dict_write_uint8(iter, KEY_PRO_LICENSE, s_pro_unlocked ? 1 : 0);
  dict_write_uint8(iter, KEY_TRIAL_STATE, trial_state_code());
  dict_write_uint32(iter, KEY_TRIAL_REMAINING, trial_remaining_seconds());
  app_message_outbox_send();
}

static void license_refresh_ui(void) {
  if (!s_pro_unlocked) {
    enforce_free_defaults();
  }

  settings_save();
  update_footer_content();
  update_header_content();
  update_stepbar_layout();
  update_bar_input_services();
  update_raise_wake_service();
  apply_bar_visibility();
  update_accent_text_contrast();
  update_background_contrast();

  if (s_clock_layer) layer_mark_dirty(s_clock_layer);
  if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
  if (s_header_layer) layer_mark_dirty(s_header_layer);
  if (s_footer_layer) layer_mark_dirty(s_footer_layer);

  license_send_status_to_phone();
}

static void license_recompute_effective(void) {
  bool unlocked = s_kiezelpay_licensed || s_trial_active;
  if (s_pro_unlocked == unlocked) {
    license_send_status_to_phone();
    return;
  }

  if (s_pro_unlocked && !unlocked) {
    // Keep the personalized Pro setup so a later purchase restores it.
    pro_settings_save_current();
  }

  s_pro_unlocked = unlocked;
  if (unlocked) {
    pro_settings_restore_saved();
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Pro entitlement -> %s (paid=%d trial=%d)",
          unlocked ? "UNLOCKED" : "FREE",
          s_kiezelpay_licensed ? 1 : 0, s_trial_active ? 1 : 0);
  license_refresh_ui();
}

static bool trial_is_currently_active(void) {
  if (!trial_has_been_used()) return false;
  time_t start = trial_start_time();
  if (start <= 0) return false;
  time_t now = time(NULL);
  return (now <= start) || ((uint32_t)(now - start) < PRO_TRIAL_SECONDS);
}

static void trial_refresh_state(void) {
  bool should_be_active = trial_is_currently_active();

  if (s_trial_active != should_be_active) {
    s_trial_active = should_be_active;
    APP_LOG(APP_LOG_LEVEL_INFO, "48-hour Pro trial -> %s",
            s_trial_active ? "ACTIVE" : "INACTIVE");
    license_recompute_effective();
  }
}

static void trial_start_if_available(void) {
  if (s_kiezelpay_licensed || trial_has_been_used()) {
    license_send_status_to_phone();
    return;
  }

  time_t now = time(NULL);
  persist_write_bool(TRIAL_USED_PERSIST_KEY, true);
  persist_write_int(TRIAL_START_PERSIST_KEY, (int32_t)now);
  s_trial_active = true;
  APP_LOG(APP_LOG_LEVEL_INFO, "48-hour Pro trial started by user");
  license_recompute_effective();
}

// KiezelPay integration point. Paid licensing and the local opt-in trial are
// intentionally separate; either one unlocks the same Pro customization layer.
void watchface_kiezelpay_set_licensed(bool licensed) {
  s_kiezelpay_licensed = licensed;
  license_recompute_effective();
}

static bool kiezelpay_event_callback(kiezelpay_event event, void *extra_data) {
  switch (event) {
    case KIEZELPAY_LICENSED:
      APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: licensed");
      watchface_kiezelpay_set_licensed(true);
      break;
    case KIEZELPAY_CODE_AVAILABLE:
      // A purchase code means this installation is not currently licensed.
      APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: purchase code available");
      watchface_kiezelpay_set_licensed(false);
      break;
    case KIEZELPAY_PURCHASE_STARTED:
      APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: purchase started");
      break;
    case KIEZELPAY_ERROR:
      APP_LOG(APP_LOG_LEVEL_ERROR, "KiezelPay: error");
      break;
    default:
      break;
  }

  // Keep KiezelPay's built-in purchase/code/error messages.
  return false;
}

static void kiezelpay_purchase_retry_timer_cb(void *context) {
  s_kiezelpay_purchase_timer = NULL;

  if (s_kiezelpay_licensed) {
    APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: purchase retry skipped; already licensed");
    return;
  }

  // A previous test purchase/reinstall can leave KiezelPay with a stale
  // purchase session. Explicitly cancel that session before requesting a new
  // code/status. This call happens after the Settings AppMessage transaction
  // has completed so KiezelPay is free to use the shared AppMessage channel.
  APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: resetting purchase session and requesting fresh code/status");
  kiezelpay_cancel_purchase();
  kiezelpay_start_purchase();
}

static void kiezelpay_request_purchase_fresh(void) {
  if (s_kiezelpay_licensed) {
    license_send_status_to_phone();
    return;
  }

  if (s_kiezelpay_purchase_timer) {
    app_timer_cancel(s_kiezelpay_purchase_timer);
    s_kiezelpay_purchase_timer = NULL;
  }

  // Do not start another AppMessage-driven transaction from inside the inbox
  // callback that delivered the Clay settings. Defer very briefly instead.
  s_kiezelpay_purchase_timer = app_timer_register(250, kiezelpay_purchase_retry_timer_cb, NULL);
}

// ── AppMessage ────────────────────────────────────────────────────────────────
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool weather_changed = false;
  bool accent_changed = false;
  bool clock_color_changed = false;
  bool background_changed = false;
  bool layout_changed = false;
  bool temperature_setting_changed = false;
  uint32_t new_accent_hex = 0;
  uint32_t new_clock_hex = 0;
  uint32_t new_background_hex = 0;

  // Parse the incoming dictionary exactly once. Do not redraw, persist, or
  // start any other AppMessage operation until parsing is complete.
  for (Tuple *t = dict_read_first(iter); t; t = dict_read_next(iter)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "RX key=%lu type=%d len=%u",
            (unsigned long)t->key, (int)t->type, (unsigned)t->length);

    // Security boundary: Pro controls are enforced on-watch, not just hidden in
    // the settings page. A crafted AppMessage cannot unlock premium settings.
    if (!s_pro_unlocked && key_is_pro_customization(t->key)) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Ignoring locked Pro setting key=%lu",
              (unsigned long)t->key);
      continue;
    }

    switch (t->key) {
      case KEY_TRIAL_START:
        if (tuple_to_int32(t, 0) == 1) trial_start_if_available();
        break;

      case KEY_TRIAL_USED_HINT:
        // Phone-side storage can survive a watchface uninstall/reinstall. If it
        // remembers that this user already consumed the opt-in trial, re-seed
        // that fact on the watch so a reinstall does not offer another trial.
        if (tuple_to_int32(t, 0) == 1 && !trial_has_been_used()) {
          persist_write_bool(TRIAL_USED_PERSIST_KEY, true);
          persist_delete(TRIAL_START_PERSIST_KEY);
          s_trial_active = false;
          APP_LOG(APP_LOG_LEVEL_INFO, "Restored used-trial marker after reinstall");
          license_recompute_effective();
        }
        break;

      case KEY_PURCHASE_PRO:
        if (tuple_to_int32(t, 0) == 1 && !s_kiezelpay_licensed) {
          APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: fresh purchase requested from Settings");
          kiezelpay_request_purchase_fresh();
        }
        break;

      case KEY_TEMPERATURE: {
        // Phone sends Celsius in tenths of a degree so the watch can switch
        // units locally without another network request.
        s_temperature_c_x10 = (int)tuple_to_int32(t, 0);
        s_have_temperature = true;
        update_temperature_text();
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

      case KEY_SUNRISE:
        s_sunrise_minute = (int)tuple_to_int32(t, -1);
        weather_changed = true;
        break;

      case KEY_SUNSET:
        s_sunset_minute = (int)tuple_to_int32(t, -1);
        weather_changed = true;
        break;

      case KEY_HIGH_TEMP:
        s_high_c_x10 = (int)tuple_to_int32(t, 0);
        s_have_high_low = true;
        weather_changed = true;
        break;

      case KEY_LOW_TEMP:
        s_low_c_x10 = (int)tuple_to_int32(t, 0);
        s_have_high_low = true;
        weather_changed = true;
        break;

#if WATCHFACE_PRO
      case KEY_ACCENT_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          new_accent_hex = (uint32_t)tuple_to_int32(t, 0) & 0xFFFFFF;
          accent_changed = true;
        }
        break;

      case KEY_LEFT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.left_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_HIGH_LOW) {
          s_settings.left_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Left slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_CENTER_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.center_slot);
        if (value >= CENTER_HEART_RATE && value <= CENTER_MONTH) {
          s_settings.center_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Center slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_RIGHT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.right_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_HIGH_LOW) {
          s_settings.right_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Right slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_TOP_LEFT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.top_left_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_HIGH_LOW) { s_settings.top_left_slot = (uint8_t)value; layout_changed = true; }
        break;
      }
      case KEY_TOP_CENTER_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.top_center_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_MONTH) { s_settings.top_center_slot = (uint8_t)value; layout_changed = true; }
        break;
      }
      case KEY_TOP_RIGHT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.top_right_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_HIGH_LOW) { s_settings.top_right_slot = (uint8_t)value; layout_changed = true; }
        break;
      }

      case KEY_TIME_FORMAT: {
        int32_t value = tuple_to_int32(t, s_settings.time_format);
        if (value >= TIME_FORMAT_12H && value <= TIME_FORMAT_24H) {
          s_settings.time_format = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Time format -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_CENTER_12H: {
        int32_t value = tuple_to_int32(t, s_settings.center_12h);
        if (value == 0 || value == 1) {
          s_settings.center_12h = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Center 12h -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_RAISE_WAKE: {
        int32_t value = tuple_to_int32(t, s_settings.raise_wake_mode);
        if (value >= RAISE_WAKE_OFF && value <= RAISE_WAKE_SENSITIVE) {
          s_settings.raise_wake_mode = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Raise wake mode -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_FOOTER_MODE: {
        int32_t value = tuple_to_int32(t, s_settings.footer_mode);
        if (value >= BAR_ALWAYS && value <= BAR_HIDDEN) {
          s_settings.footer_mode = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Footer mode -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_HEADER_MODE: {
        int32_t value = tuple_to_int32(t, s_settings.header_mode);
        if (value >= BAR_ALWAYS && value <= BAR_HIDDEN) {
          s_settings.header_mode = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Header mode -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_STEPBAR_MODE: {
        int32_t value = tuple_to_int32(t, s_settings.stepbar_mode);
        if (value >= STEPBAR_MIRRORED && value <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT) {
          s_settings.stepbar_mode = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Stepbar mode -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_STEP_GOAL: {
        int32_t value = tuple_to_int32(t, s_settings.step_goal);
        if (value >= 1000 && value <= 30000) {
          s_settings.step_goal = (uint16_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Step goal -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_TEMP_UNIT: {
        int32_t value = tuple_to_int32(t, s_settings.temp_unit);
        if (value >= TEMP_FAHRENHEIT && value <= TEMP_CELSIUS) {
          s_settings.temp_unit = (uint8_t)value;
          update_temperature_text();
          APP_LOG(APP_LOG_LEVEL_INFO, "Temperature unit -> %ld", (long)value);
          temperature_setting_changed = true;
        }
        break;
      }

      case KEY_CLOCK_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          new_clock_hex = (uint32_t)tuple_to_int32(t, 0xFFFFFF) & 0xFFFFFF;
          clock_color_changed = true;
        }
        break;

      case KEY_BACKGROUND_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          new_background_hex = (uint32_t)tuple_to_int32(t, 0x000000) & 0xFFFFFF;
          background_changed = true;
        }
        break;
#endif

      case KEY_LICENSE_CHECK:
        trial_refresh_state();
        license_send_status_to_phone();
        break;

      // Phone cannot grant a license. KEY_PRO_LICENSE is watch -> phone only.
      case KEY_PRO_LICENSE:
        break;

      default:
        break;
    }
  }

  // Only touch persistent storage and UI after DictionaryIterator is finished.
#if WATCHFACE_PRO
  if (!settings_values_valid(&s_settings)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid settings after config; restoring footer defaults");
    GColor keep_accent = s_settings.accent_color;
    GColor keep_clock = s_settings.clock_color;
    GColor keep_background = s_settings.background_color;
    settings_set_defaults();
    s_settings.accent_color = keep_accent;
    s_settings.clock_color = keep_clock;
    s_settings.background_color = keep_background;
    layout_changed = true;
  }

  if (accent_changed) {
    s_settings.accent_color = GColorFromHEX(new_accent_hex);
  }
  if (clock_color_changed) {
    s_settings.clock_color = GColorFromHEX(new_clock_hex);
  }
  if (background_changed) {
    s_settings.background_color = GColorFromHEX(new_background_hex);

    // Treat pure black/white clock colors as automatic contrast choices.
    // Custom colors remain exactly as the user selected them.
    if (s_settings.clock_color.argb == GColorWhite.argb ||
        s_settings.clock_color.argb == GColorBlack.argb) {
      GColor auto_clock = gcolor_legible_over(s_settings.background_color);
      if (auto_clock.argb != s_settings.clock_color.argb) {
        s_settings.clock_color = auto_clock;
        clock_color_changed = true;
        new_clock_hex = (auto_clock.argb == GColorBlack.argb) ? 0x000000 : 0xFFFFFF;
      }
    }
  }

  if (accent_changed || clock_color_changed || background_changed || layout_changed || temperature_setting_changed) {
    settings_save();
  }
#endif

  if (weather_changed) {
    update_weather_icon(s_weather_icon);
    update_footer_content();
    update_header_content();
  }
  if (temperature_setting_changed) {
    update_footer_content();
    update_header_content();
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

  if (clock_color_changed && s_clock_layer) {
    layer_mark_dirty(s_clock_layer);
    APP_LOG(APP_LOG_LEVEL_INFO, "Clock color applied: 0x%06lX",
            (unsigned long)new_clock_hex);
  }

  if (background_changed) {
    update_background_contrast();
    update_footer_content();
    APP_LOG(APP_LOG_LEVEL_INFO, "Background color applied: 0x%06lX",
            (unsigned long)new_background_hex);
  }

  if (layout_changed) {
    if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
    update_footer_content();
    update_header_content();
    update_stepbar_layout();
    update_bar_input_services();
    update_raise_wake_service();
    apply_bar_visibility();

    // Time-format changes should be visible immediately instead of waiting for
    // the next minute tick.
    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    if (current) update_time(current);
  }
#endif
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage inbox dropped: %d", (int)reason);
}

// ── Time / date update ────────────────────────────────────────────────────────
static void update_time(struct tm *tick_time) {
  if (s_settings.time_format == TIME_FORMAT_24H) {
    s_hour = tick_time->tm_hour;
  } else {
    s_hour = tick_time->tm_hour % 12;
    if (s_hour == 0) s_hour = 12;
  }
  s_minute = tick_time->tm_min;
  layer_mark_dirty(s_clock_layer);

  strftime(s_day_buf,   sizeof(s_day_buf),   "%a", tick_time); to_upper(s_day_buf);
  strftime(s_date_buf,  sizeof(s_date_buf),  "%e", tick_time);
  strftime(s_month_buf, sizeof(s_month_buf), "%b", tick_time); to_upper(s_month_buf);

  update_header_content();
  update_footer_content();

  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  trial_refresh_state();
  update_time(tick_time);
}

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
    s_active_kcal = (int)health_service_sum_today(HealthMetricActiveKCalories);
    s_distance_m = (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
    if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
    update_footer_content();
    update_header_content();
  }
  if (event == HealthEventHeartRateUpdate || event == HealthEventSignificantUpdate) {
    HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
    s_heart_rate = hr > 0 ? (int)hr : 0;
    update_footer_content();
    update_header_content();
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
  int top_left_w = DATEBOX_X - BOX_GAP - 4;
  int top_right_x = DATEBOX_X + DATEBOX_W + BOX_GAP;
  int top_right_w = SCREEN_W - top_right_x - 4;

  s_top_left_label = text_layer_create(GRect(4, 3, top_left_w, 14));
  text_layer_set_background_color(s_top_left_label, GColorClear);
  text_layer_set_text_color(s_top_left_label, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_top_left_label, s_font_label);
  text_layer_set_text_alignment(s_top_left_label, GTextAlignmentLeft);
  layer_add_child(s_header_layer, text_layer_get_layer(s_top_left_label));
  s_top_left_val = text_layer_create(GRect(4, 15, top_left_w, 34));
  text_layer_set_background_color(s_top_left_val, GColorClear);
  text_layer_set_text_color(s_top_left_val, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_top_left_val, s_font_header);
  text_layer_set_text_alignment(s_top_left_val, GTextAlignmentLeft);
  layer_add_child(s_header_layer, text_layer_get_layer(s_top_left_val));

  s_top_center_label = text_layer_create(GRect(DATEBOX_X, 3, DATEBOX_W, 14));
  text_layer_set_background_color(s_top_center_label, GColorClear);
  text_layer_set_text_color(s_top_center_label, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_top_center_label, s_font_label);
  text_layer_set_text_alignment(s_top_center_label, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_top_center_label));
  s_top_center_val = text_layer_create(GRect(DATEBOX_X, 15, DATEBOX_W, 34));
  text_layer_set_background_color(s_top_center_val, GColorClear);
  text_layer_set_text_color(s_top_center_val, gcolor_legible_over(s_settings.accent_color));
  text_layer_set_font(s_top_center_val, s_font_header);
  text_layer_set_text_alignment(s_top_center_val, GTextAlignmentCenter);
  layer_add_child(s_header_layer, text_layer_get_layer(s_top_center_val));

  s_top_right_label = text_layer_create(GRect(top_right_x, 3, top_right_w, 14));
  text_layer_set_background_color(s_top_right_label, GColorClear);
  text_layer_set_text_color(s_top_right_label, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_top_right_label, s_font_label);
  text_layer_set_text_alignment(s_top_right_label, GTextAlignmentRight);
  layer_add_child(s_header_layer, text_layer_get_layer(s_top_right_label));
  s_top_right_val = text_layer_create(GRect(top_right_x, 15, top_right_w, 34));
  text_layer_set_background_color(s_top_right_val, GColorClear);
  text_layer_set_text_color(s_top_right_val, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_top_right_val, s_font_header);
  text_layer_set_text_alignment(s_top_right_val, GTextAlignmentRight);
  layer_add_child(s_header_layer, text_layer_get_layer(s_top_right_val));

  // Clock
  s_clock_layer = layer_create(GRect(0, CLOCK_Y, SCREEN_W, CLOCK_H));
  layer_set_update_proc(s_clock_layer, clock_update_proc);
  layer_add_child(root, s_clock_layer);

  // Step bar
  s_stepbar_layer = layer_create(GRect(0, STEPBAR_Y, SCREEN_W, STEPBAR_H));
  layer_set_update_proc(s_stepbar_layer, stepbar_update_proc);
  layer_add_child(root, s_stepbar_layer);
  update_stepbar_layout();

  // Footer — extends to bottom of screen
  s_footer_layer = layer_create(GRect(0, FOOTER_Y, SCREEN_W, SCREEN_H - FOOTER_Y));
  layer_set_update_proc(s_footer_layer, footer_update_proc);
  layer_add_child(root, s_footer_layer);

  int left_w = HRBOX_X - BOX_GAP - 4;
  int right_x = HRBOX_X + BOX_W + BOX_GAP;
  int right_w = SCREEN_W - right_x - 4;

  s_left_label = text_layer_create(GRect(4, 2, left_w, 14));
  text_layer_set_background_color(s_left_label, GColorClear);
  text_layer_set_text_color(s_left_label, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_left_label, s_font_label);
  text_layer_set_text_alignment(s_left_label, GTextAlignmentLeft);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_left_label));

  s_left_val = text_layer_create(GRect(4, 14, left_w, 38));
  text_layer_set_background_color(s_left_val, GColorClear);
  text_layer_set_text_color(s_left_val, gcolor_legible_over(s_settings.background_color));
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
  text_layer_set_text_color(s_right_label, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_right_label, s_font_label);
  text_layer_set_text_alignment(s_right_label, GTextAlignmentRight);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_right_label));

  s_right_val = text_layer_create(GRect(right_x, 14, right_w, 38));
  text_layer_set_background_color(s_right_val, GColorClear);
  text_layer_set_text_color(s_right_val, gcolor_legible_over(s_settings.background_color));
  text_layer_set_font(s_right_val, s_font_value);
  text_layer_set_text_alignment(s_right_val, GTextAlignmentRight);
  layer_add_child(s_footer_layer, text_layer_get_layer(s_right_val));

  s_weather_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_NA);
  tint_weather_bitmap(s_weather_icon_bitmap, gcolor_legible_over(s_settings.background_color));
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
  s_active_kcal = (int)health_service_sum_today(HealthMetricActiveKCalories);
  s_distance_m = (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
  HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
  s_heart_rate = hr > 0 ? (int)hr : 0;
#endif

  update_footer_content();
  update_header_content();
  update_accent_text_contrast();
  update_background_contrast();
  apply_bar_visibility();

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

  text_layer_destroy(s_top_left_label);
  text_layer_destroy(s_top_left_val);
  text_layer_destroy(s_top_center_label);
  text_layer_destroy(s_top_center_val);
  text_layer_destroy(s_top_right_label);
  text_layer_destroy(s_top_right_val);
  s_top_left_label = NULL; s_top_left_val = NULL;
  s_top_center_label = NULL; s_top_center_val = NULL;
  s_top_right_label = NULL; s_top_right_val = NULL;

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

  // Start from persisted trial state, then let KiezelPay independently validate
  // permanent entitlement. A trial never starts merely because the face runs.
  s_kiezelpay_licensed = false;
  s_trial_active = trial_is_currently_active();
  s_pro_unlocked = s_trial_active;
  if (!s_pro_unlocked) {
    enforce_free_defaults();
    settings_save();
  } else {
    // Keep the active trial's current settings backed up as Pro settings.
    settings_save();
  }

  s_window = window_create();
  window_set_background_color(s_window, s_settings.background_color);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load, .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // KiezelPay and the watchface both consume AppMessage, so use pebble-events
  // to multiplex callbacks and open the shared channel exactly once.
  events_app_message_register_inbox_received(inbox_received_handler, NULL);
  events_app_message_register_inbox_dropped(inbox_dropped_handler, NULL);
  kiezelpay_set_event_handler(kiezelpay_event_callback);
  kiezelpay_init();
  events_app_message_open();
  license_send_status_to_phone();
  battery_state_service_subscribe(battery_handler);
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = connection_handler
  });
  s_backlight_on = light_is_on();
  update_bar_input_services();
  update_raise_wake_service();
  app_focus_service_subscribe(focus_handler);
#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  if (s_kiezelpay_purchase_timer) {
    app_timer_cancel(s_kiezelpay_purchase_timer);
    s_kiezelpay_purchase_timer = NULL;
  }
  kiezelpay_deinit();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  if (s_backlight_subscribed) backlight_service_unsubscribe();
  if (s_touch_subscribed) touch_service_unsubscribe();
  if (s_interaction_timer) {
    app_timer_cancel(s_interaction_timer);
    s_interaction_timer = NULL;
  }
  if (s_raise_accel_subscribed) accel_data_service_unsubscribe();
  app_focus_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) { init(); app_event_loop(); deinit(); }