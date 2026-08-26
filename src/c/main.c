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
static bool conditional_ui_is_visible(void);

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
#define KEY_TRY_PRO_FREE   26
#define KEY_UNLOCK_PRO     27
#define KEY_SHOW_LABELS     28  // legacy global label toggle
#define KEY_TOP_LEFT_HIDE_LABEL   29
#define KEY_TOP_CENTER_HIDE_LABEL 30
#define KEY_TOP_RIGHT_HIDE_LABEL  31
#define KEY_LEFT_HIDE_LABEL       32
#define KEY_CENTER_HIDE_LABEL     33
#define KEY_RIGHT_HIDE_LABEL      34
#define KEY_LANGUAGE              35
#define KEY_TOP_LEFT_TIME_ZONE    36
#define KEY_TOP_RIGHT_TIME_ZONE   37
#define KEY_LEFT_TIME_ZONE        38
#define KEY_RIGHT_TIME_ZONE       39
#define KEY_HOUR_COLOR            40
#define KEY_MINUTE_COLOR          41
#define KEY_SPLIT_CLOCK_COLORS    42
#define KEY_FLASH_COLON           43
#define KEY_ROUNDED_TIME          44

// Internal KiezelPay protocol value emitted by kiezelpay-core v2.2.4.
// KiezelPay's own handler is registered before Big Time's pebble-events
// handler, so by the time Big Time sees this tuple, kiezelpay-core has already
// processed the same dictionary.
#define KPAY_KEY_STATUS_RESULT      10009
#define KPAY_STATUS_LICENSED        2

// ── Persistent settings ──────────────────────────────────────────────────────
#define SETTINGS_PERSIST_KEY 1
#define SETTINGS_VERSION     16

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
  SLOT_HIGH_LOW = 12,
  SLOT_BATTERY_ICON = 13,
  SLOT_BATTERY_PERCENT = 14,
  SLOT_TIME_ZONE = 15,
  SLOT_SECONDS = 16
} SideSlotContent;

typedef enum {
  CENTER_HEART_RATE = 0,
  CENTER_BATTERY = 1,
  CENTER_BLUETOOTH = 2,
  CENTER_WEATHER = 3,
  CENTER_STEPS = 4,
  CENTER_DAY = 5,
  CENTER_DATE = 6,
  CENTER_MONTH = 7,
  CENTER_BATTERY_ICON = 8,
  CENTER_BATTERY_PERCENT = 9,
  CENTER_SECONDS = 10
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

/*
 * ADDING A WATCHFACE LANGUAGE — REQUIRED CHECKLIST
 *
 * Language support spans both watch C code and PebbleKit JS. When adding a
 * language, update EVERY item below. Append new IDs; never renumber existing
 * languages because the numeric value is persisted on users' watches/phones.
 *
 *   1. WatchLanguage enum below — append the new LANG_* ID.
 *   2. TRANSLATIONS[] — add labels, weekdays, and months in the same order.
 *   3. settings_values_valid() — upper bound must allow the newest language.
 *   4. enforce_free_defaults() — keep_language upper bound must allow it.
 *   5. inbox_received_handler(), KEY_LANGUAGE — accepted upper bound must allow it.
 *   6. src/pkjs/config.js LANGUAGE selector — add the visible dropdown option.
 *   7. src/pkjs/index.js SUPPORTED_LANGUAGES — add the same ID/label once;
 *      this registry drives runtime Clay options and JS language validation.
 *
 * Before release, verify: select newest language -> Save -> face translates ->
 * reopen Settings -> newest language is still selected.
 */
typedef enum {
  LANG_ENGLISH = 0,
  LANG_SWEDISH = 1,
  LANG_SPANISH = 2,
  LANG_FRENCH = 3,
  LANG_GERMAN = 4,
  LANG_PORTUGUESE = 5,
  LANG_CATALAN = 6
} WatchLanguage;

typedef struct {
  const char *label;
  int16_t utc_offset_minutes;
  bool use_local;
} TimeZonePreset;

static const TimeZonePreset TIME_ZONE_PRESETS[] = {
  { "LOCAL",       0, true  },
  { "UTC-12:00", -720, false },
  { "UTC-11:00", -660, false },
  { "UTC-10:00", -600, false },
  { "UTC-09:00", -540, false },
  { "UTC-08:00", -480, false },
  { "UTC-07:00", -420, false },
  { "UTC-06:00", -360, false },
  { "UTC-05:00", -300, false },
  { "UTC-04:00", -240, false },
  { "UTC-03:00", -180, false },
  { "UTC-02:00", -120, false },
  { "UTC-01:00",  -60, false },
  { "UTC",           0, false },
  { "UTC+01:00",    60, false },
  { "UTC+02:00",   120, false },
  { "UTC+03:00",   180, false },
  { "UTC+03:30",   210, false },
  { "UTC+04:00",   240, false },
  { "UTC+04:30",   270, false },
  { "UTC+05:00",   300, false },
  { "UTC+05:30",   330, false },
  { "UTC+05:45",   345, false },
  { "UTC+06:00",   360, false },
  { "UTC+06:30",   390, false },
  { "UTC+07:00",   420, false },
  { "UTC+08:00",   480, false },
  { "UTC+08:45",   525, false },
  { "UTC+09:00",   540, false },
  { "UTC+09:30",   570, false },
  { "UTC+10:00",   600, false },
  { "UTC+10:30",   630, false },
  { "UTC+11:00",   660, false },
  { "UTC+12:00",   720, false },
  { "UTC+12:45",   765, false },
  { "UTC+13:00",   780, false },
  { "UTC+14:00",   840, false }
};

#define TIME_ZONE_PRESET_COUNT ((uint8_t)ARRAY_LENGTH(TIME_ZONE_PRESETS))


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
} WatchfaceSettingsV13;

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
  uint8_t show_labels;
} WatchfaceSettingsV14;

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
  uint8_t top_left_hide_label;
  uint8_t top_center_hide_label;
  uint8_t top_right_hide_label;
  uint8_t left_hide_label;
  uint8_t center_hide_label;
  uint8_t right_hide_label;
} WatchfaceSettingsV15;

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
  uint8_t top_left_hide_label;
  uint8_t top_center_hide_label;
  uint8_t top_right_hide_label;
  uint8_t left_hide_label;
  uint8_t center_hide_label;
  uint8_t right_hide_label;
  uint8_t language;
} WatchfaceSettings;

static WatchfaceSettings s_settings;
static WatchfaceSettings s_saved_settings;
static bool s_saved_settings_valid = false;

// Runtime Pro entitlement. KiezelPay's product-specific Pebble library should
// call license_set_pro(true/false) from its license callback.
// Default is free/locked so a failed or unavailable license check never grants
// premium features accidentally.
static bool s_pro_unlocked = false;

// Big Time's trial is deliberately user-started rather than KiezelPay's
// automatic timed trial. This preserves a permanently usable Free edition.
#define PRO_TRIAL_PERSIST_KEY  1002
#define PURCHASED_PRO_PERSIST_KEY 1003
#define WEATHER_CACHE_PERSIST_KEY 1004
#define TZ_TOP_LEFT_PERSIST_KEY   1101
#define TZ_TOP_RIGHT_PERSIST_KEY  1102
#define TZ_LEFT_PERSIST_KEY       1103
#define TZ_RIGHT_PERSIST_KEY      1104
#define HOUR_COLOR_PERSIST_KEY    1105
#define MINUTE_COLOR_PERSIST_KEY  1106
#define SPLIT_COLOR_PERSIST_KEY   1107
#define FLASH_COLON_PERSIST_KEY   1108
#define ROUNDED_TIME_PERSIST_KEY  1109
#define PRO_TRIAL_SECONDS      (48 * 60 * 60)
static bool s_trial_active = false;
static bool s_kiezelpay_licensed = false;
static bool s_kiezelpay_status_known = false;
static bool s_license_query_waiting = false;
static AppTimer *s_license_query_wait_timer = NULL;
static uint8_t s_license_query_wait_count = 0;

static EventHandle s_appmsg_received_handle;
static EventHandle s_appmsg_dropped_handle;
static bool s_appmsg_handlers_registered = false;
static AppTimer *s_license_status_retry_timer = NULL;
static uint8_t s_license_status_retry_count = 0;

static void license_send_status_to_phone(void);
static void license_set_pro(bool unlocked);
static void pro_trial_mark_expired(void);
static void schedule_license_status_retry(void);
static void send_license_status_when_ready(void);
static void purchased_pro_set_persisted(bool purchased);
static bool purchased_pro_is_persisted(void);

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
  s_settings.top_left_hide_label = 0;
  s_settings.top_center_hide_label = 0;
  s_settings.top_right_hide_label = 0;
  s_settings.left_hide_label = 0;
  s_settings.center_hide_label = 0;
  s_settings.right_hide_label = 0;
  s_settings.language = 0;
}


static void enforce_free_defaults(void) {
  // Preserve all Free customization choices while resetting Pro-only
  // presentation settings.
  uint8_t keep_time_format = s_settings.time_format;
  uint8_t keep_center_12h = s_settings.center_12h;
  uint8_t keep_temp_unit = s_settings.temp_unit;
  uint8_t keep_language = s_settings.language;

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
  s_settings.temp_unit =
      keep_temp_unit <= TEMP_CELSIUS ? keep_temp_unit : TEMP_FAHRENHEIT;
  s_settings.raise_wake_mode = RAISE_WAKE_OFF;
  s_settings.top_left_hide_label = 0;
  s_settings.top_center_hide_label = 0;
  s_settings.top_right_hide_label = 0;
  s_settings.left_hide_label = 0;
  s_settings.center_hide_label = 0;
  s_settings.right_hide_label = 0;

  s_settings.language =
      keep_language <= LANG_CATALAN ? keep_language : LANG_ENGLISH;
  s_settings.time_format =
      keep_time_format <= TIME_FORMAT_24H ? keep_time_format : TIME_FORMAT_12H;
  s_settings.center_12h = keep_center_12h ? 1 : 0;
}

static bool key_is_free_customization(uint32_t key) {
  return key == KEY_TIME_FORMAT ||
         key == KEY_CENTER_12H ||
         key == KEY_TEMP_UNIT ||
         key == KEY_LANGUAGE;
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
    case KEY_CLOCK_COLOR:
    case KEY_BACKGROUND_COLOR:
    case KEY_TOP_LEFT_SLOT:
    case KEY_TOP_CENTER_SLOT:
    case KEY_TOP_RIGHT_SLOT:
    case KEY_RAISE_WAKE:
    case KEY_TOP_LEFT_HIDE_LABEL:
    case KEY_TOP_CENTER_HIDE_LABEL:
    case KEY_TOP_RIGHT_HIDE_LABEL:
    case KEY_LEFT_HIDE_LABEL:
    case KEY_CENTER_HIDE_LABEL:
    case KEY_RIGHT_HIDE_LABEL:
    case KEY_TOP_LEFT_TIME_ZONE:
    case KEY_TOP_RIGHT_TIME_ZONE:
    case KEY_LEFT_TIME_ZONE:
    case KEY_RIGHT_TIME_ZONE:
    case KEY_HOUR_COLOR:
    case KEY_MINUTE_COLOR:
    case KEY_SPLIT_CLOCK_COLORS:
    case KEY_FLASH_COLON:
    case KEY_ROUNDED_TIME:
      return true;
    default:
      return false;
  }
}

static bool settings_values_valid(const WatchfaceSettings *settings) {
  if (!settings) return false;
  return settings->version == SETTINGS_VERSION &&
         settings->left_slot <= SLOT_SECONDS &&
         ((settings->center_slot <= CENTER_MONTH &&
           settings->center_slot != CENTER_STEPS) ||
          settings->center_slot == CENTER_BATTERY_ICON ||
          settings->center_slot == CENTER_BATTERY_PERCENT ||
          settings->center_slot == CENTER_SECONDS) &&
         settings->right_slot <= SLOT_SECONDS &&
         settings->top_left_slot <= SLOT_SECONDS &&
         ((settings->top_center_slot <= SLOT_MONTH &&
           settings->top_center_slot != SLOT_STEPS) ||
          settings->top_center_slot == SLOT_BATTERY_ICON ||
          settings->top_center_slot == SLOT_BATTERY_PERCENT ||
          settings->top_center_slot == SLOT_SECONDS) &&
         settings->top_right_slot <= SLOT_SECONDS &&
         settings->footer_mode <= BAR_HIDDEN &&
         settings->header_mode <= BAR_HIDDEN &&
         settings->stepbar_mode <= STEPBAR_LEFT_TO_RIGHT_ABOVE_BACKLIGHT &&
         settings->step_goal >= 1000 && settings->step_goal <= 30000 &&
         settings->temp_unit <= TEMP_CELSIUS &&
         settings->time_format <= TIME_FORMAT_24H &&
         settings->center_12h <= 1 &&
         settings->raise_wake_mode <= RAISE_WAKE_SENSITIVE &&
         settings->top_left_hide_label <= 1 &&
         settings->top_center_hide_label <= 1 &&
         settings->top_right_hide_label <= 1 &&
         settings->left_hide_label <= 1 &&
         settings->center_hide_label <= 1 &&
         settings->right_hide_label <= 1 &&
         settings->language <= LANG_CATALAN;
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
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV15)) {
    WatchfaceSettingsV15 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 15) {
      memcpy(&s_settings, &old, sizeof(old));
      s_settings.version = SETTINGS_VERSION;
      s_settings.language = 0;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV14)) {
    WatchfaceSettingsV14 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 14) {
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
      s_settings.raise_wake_mode = old.raise_wake_mode;

      // Preserve the old global label choice across all six slots.
      uint8_t hide = old.show_labels ? 0 : 1;
      s_settings.top_left_hide_label = hide;
      s_settings.top_center_hide_label = hide;
      s_settings.top_right_hide_label = hide;
      s_settings.left_hide_label = hide;
      s_settings.center_hide_label = hide;
      s_settings.right_hide_label = hide;

      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
      return;
    }
  } else if (stored_size == (int)sizeof(WatchfaceSettingsV13)) {
    WatchfaceSettingsV13 old;
    if (persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old)) == (int)sizeof(old) &&
        old.version == 13 &&
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
        old.center_12h <= 1 &&
        old.raise_wake_mode <= RAISE_WAKE_SENSITIVE) {
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
      s_settings.raise_wake_mode = old.raise_wake_mode;
      s_settings.top_left_hide_label = 0;
      s_settings.top_center_hide_label = 0;
      s_settings.top_right_hide_label = 0;
      s_settings.left_hide_label = 0;
      s_settings.center_hide_label = 0;
      s_settings.right_hide_label = 0;
      persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
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
  if (s_pro_unlocked) {
    // While Pro/trial is active, the effective settings are the user's real
    // preferences. Keep the preserved copy synchronized.
    s_saved_settings = s_settings;
    s_saved_settings_valid = true;
    persist_write_data(SETTINGS_PERSIST_KEY, &s_saved_settings, sizeof(s_saved_settings));
    return;
  }

  // While Free, s_settings contains the enforced Free presentation. Never save
  // those defaults over the user's premium preferences. Only the two controls
  // available in Free are allowed to update the preserved preference record.
  if (!s_saved_settings_valid) {
    s_saved_settings = s_settings;
    s_saved_settings_valid = true;
  }

  s_saved_settings.time_format = s_settings.time_format;
  s_saved_settings.center_12h = s_settings.center_12h;
  s_saved_settings.temp_unit = s_settings.temp_unit;
  s_saved_settings.language = s_settings.language;
  persist_write_data(SETTINGS_PERSIST_KEY, &s_saved_settings, sizeof(s_saved_settings));
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
static GFont s_font_medium;
static GFont s_font_label;
static GFont s_font_value;

// ── State ─────────────────────────────────────────────────────────────────────
static char s_day_buf[8];
static char s_date_buf[3];
static char s_month_buf[8];
static char s_hr_buf[12];
static char s_steps_buf[8];
static char s_weather_buf[12];
static char s_battery_buf[12];
static char s_seconds_buf[3];
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
static GColor s_hour_color;
static GColor s_minute_color;
static bool s_split_clock_colors = false;
static bool s_flash_colon = false;
typedef enum {
  TIME_STYLE_SQUARE = 0,
  TIME_STYLE_ROUNDED = 1,
  TIME_STYLE_SOFT_SQUARE = 2
} TimeStyle;
static TimeStyle s_time_style = TIME_STYLE_SQUARE;
static bool s_second_tick_mode = false;
// Drawing helpers use this transient color so their geometry remains unchanged.
static GColor s_clock_draw_color;
static uint8_t s_top_left_time_zone = 0;
static uint8_t s_top_right_time_zone = 0;
static uint8_t s_left_time_zone = 0;
static uint8_t s_right_time_zone = 0;
static char s_tz_top_left_buf[12];
static char s_tz_top_right_buf[12];
static char s_tz_left_buf[12];
static char s_tz_right_buf[12];
static void load_split_clock_colors(void) {
  // Backward compatible default: existing Clock Color controls everything.
  s_hour_color = s_settings.clock_color;
  s_minute_color = s_settings.clock_color;
  s_split_clock_colors = false;

  if (persist_exists(HOUR_COLOR_PERSIST_KEY)) {
    uint32_t hex = (uint32_t)persist_read_int(HOUR_COLOR_PERSIST_KEY) & 0xFFFFFF;
    s_hour_color = GColorFromHEX(hex);
  }
  if (persist_exists(MINUTE_COLOR_PERSIST_KEY)) {
    uint32_t hex = (uint32_t)persist_read_int(MINUTE_COLOR_PERSIST_KEY) & 0xFFFFFF;
    s_minute_color = GColorFromHEX(hex);
  }
  if (persist_exists(SPLIT_COLOR_PERSIST_KEY)) {
    s_split_clock_colors = persist_read_int(SPLIT_COLOR_PERSIST_KEY) != 0;
  }
  s_flash_colon =
      persist_exists(FLASH_COLON_PERSIST_KEY) &&
      persist_read_int(FLASH_COLON_PERSIST_KEY) != 0;
  if (persist_exists(ROUNDED_TIME_PERSIST_KEY)) {
    int saved_style = persist_read_int(ROUNDED_TIME_PERSIST_KEY);
    // v3.2.2 stored this key as a bool, so 0/1 remain fully compatible.
    if (saved_style >= TIME_STYLE_SQUARE &&
        saved_style <= TIME_STYLE_SOFT_SQUARE) {
      s_time_style = (TimeStyle)saved_style;
    } else {
      s_time_style = TIME_STYLE_SQUARE;
    }
  } else {
    s_time_style = TIME_STYLE_SQUARE;
  }
}

static uint8_t load_time_zone_preset(uint32_t persist_key) {
  if (!persist_exists(persist_key)) return 0;

  int value = persist_read_int(persist_key);
  if (value < 0 || value >= TIME_ZONE_PRESET_COUNT) return 0;
  return (uint8_t)value;
}

static void load_time_zone_presets(void) {
  s_top_left_time_zone = load_time_zone_preset(TZ_TOP_LEFT_PERSIST_KEY);
  s_top_right_time_zone = load_time_zone_preset(TZ_TOP_RIGHT_PERSIST_KEY);
  s_left_time_zone = load_time_zone_preset(TZ_LEFT_PERSIST_KEY);
  s_right_time_zone = load_time_zone_preset(TZ_RIGHT_PERSIST_KEY);
}

static bool s_bluetooth_connected = false;
static int  s_hour        = 0;
static int  s_minute      = 0;
static int  s_second      = 0;
static int  s_weather_icon = -1;
static int  s_temperature_c_x10 = 0;
static bool s_have_temperature = false;
static int  s_sunrise_minute = -1;
static int  s_sunset_minute = -1;
static int  s_high_c_x10 = 0;
static int  s_low_c_x10 = 0;
static bool s_have_high_low = false;

typedef struct {
  int32_t temperature_c_x10;
  int32_t weather_icon;
  int32_t sunrise_minute;
  int32_t sunset_minute;
  int32_t high_c_x10;
  int32_t low_c_x10;
  uint8_t have_temperature;
  uint8_t have_high_low;
} WeatherCache;
static bool s_backlight_subscribed = false;

// Conditional bars/step bar normally follow Pebble's actual system backlight.
// If an explicit interaction occurs while the OS keeps the LED off (for example
// in bright sunlight), a single five-second fallback reveals the hidden UI.
static bool s_touch_subscribed = false;
static bool s_sunlight_fallback_active = false;
static AppTimer *s_sunlight_fallback_timer = NULL;
#define SUNLIGHT_FALLBACK_MS 5000

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
static int clock_segment_radius(int thickness) {
  switch (s_time_style) {
    case TIME_STYLE_ROUNDED:
      // Capsule-like ends, matching the rounded style introduced in v3.2.2.
      return thickness / 2;
    case TIME_STYLE_SOFT_SQUARE:
      // Preserve the squared seven-segment character while just softening
      // each corner. A small fixed radius keeps the effect subtle.
      return 2;
    case TIME_STYLE_SQUARE:
    default:
      return 0;
  }
}

static GCornerMask clock_segment_corners(void) {
  return s_time_style == TIME_STYLE_SQUARE ? GCornerNone : GCornersAll;
}

static void draw_h(GContext *ctx, int ox, int oy) {
  graphics_fill_rect(ctx, GRect(ox, oy, DIGIT_WIDTH, STK),
                     clock_segment_radius(STK), clock_segment_corners());
}
static void draw_v(GContext *ctx, int ox, int oy, int len) {
  graphics_fill_rect(ctx, GRect(ox, oy, STK, len),
                     clock_segment_radius(STK), clock_segment_corners());
}
static void draw_digit(GContext *ctx, int ox, int oy, int digit) {
  if (digit < 0 || digit > 9) return;
  uint8_t s = DIGIT_SEGS[digit];
  graphics_context_set_fill_color(ctx, s_clock_draw_color);
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
  graphics_context_set_fill_color(ctx, s_clock_draw_color);
  int ox    = H1_X + H1_ONE_X;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  draw_v(ctx, ox, oy,    mid_y - oy + STK);
  draw_v(ctx, ox, mid_y, bot_y - mid_y + STK);
}
static void draw_one(GContext *ctx, int cell_x, int oy) {
  graphics_context_set_fill_color(ctx, s_clock_draw_color);
  int ox    = cell_x + ONE_X_OFFSET;
  int mid_y = oy + HALF_V - STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - STK;
  draw_v(ctx, ox, oy,    mid_y - oy + STK);
  draw_v(ctx, ox, mid_y, bot_y - mid_y + STK);
}
static void draw_colon(GContext *ctx, int ox, int oy) {
  graphics_context_set_fill_color(ctx, s_clock_draw_color);
  int cx      = ox + (COLON_WIDTH - COLON_DOT) / 2;
  int upper_y = oy + DIGIT_HEIGHT / 3 - COLON_DOT / 2;
  int lower_y = oy + (DIGIT_HEIGHT * 2) / 3 - COLON_DOT / 2;
  int radius = 0;
  if (s_time_style == TIME_STYLE_ROUNDED) {
    radius = COLON_DOT / 2;
  } else if (s_time_style == TIME_STYLE_SOFT_SQUARE) {
    radius = 2;
  }
  GCornerMask corners =
      s_time_style == TIME_STYLE_SQUARE ? GCornerNone : GCornersAll;
  graphics_fill_rect(ctx, GRect(cx, upper_y, COLON_DOT, COLON_DOT),
                     radius, corners);
  graphics_fill_rect(ctx, GRect(cx, lower_y, COLON_DOT, COLON_DOT),
                     radius, corners);
}

static void draw_digit_24(GContext *ctx, int ox, int oy, int digit, int width) {
  if (digit < 0 || digit > 9) return;
  uint8_t s = DIGIT_SEGS[digit];
  graphics_context_set_fill_color(ctx, s_clock_draw_color);

  int top_y = oy;
  int mid_y = oy + HALF_V - H24_STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - H24_STK;
  int lx = ox;
  int rx = ox + width - H24_STK;

  if (s & SEG_TOP) graphics_fill_rect(ctx, GRect(ox, top_y, width, H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  if (s & SEG_MID) graphics_fill_rect(ctx, GRect(ox, mid_y, width, H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  if (s & SEG_BOT) graphics_fill_rect(ctx, GRect(ox, bot_y, width, H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  if (s & SEG_TL) graphics_fill_rect(ctx, GRect(lx, top_y, H24_STK, mid_y - top_y + H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  if (s & SEG_TR) graphics_fill_rect(ctx, GRect(rx, top_y, H24_STK, mid_y - top_y + H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  if (s & SEG_BL) graphics_fill_rect(ctx, GRect(lx, mid_y, H24_STK, bot_y - mid_y + H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  if (s & SEG_BR) graphics_fill_rect(ctx, GRect(rx, mid_y, H24_STK, bot_y - mid_y + H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
}

static void draw_one_24(GContext *ctx, int cell_x, int oy) {
  graphics_context_set_fill_color(ctx, s_clock_draw_color);

  // In 24-hour mode every "1" uses the same ONE_X_OFFSET (8px).
  // This keeps 01:11, 11:11, 21:11, etc. visually consistent.
  int ox = cell_x + ONE_X_OFFSET;
  int mid_y = oy + HALF_V - H24_STK / 2;
  int bot_y = oy + DIGIT_HEIGHT - H24_STK;

  graphics_fill_rect(ctx, GRect(ox, oy, H24_STK, mid_y - oy + H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
  graphics_fill_rect(ctx, GRect(ox, mid_y, H24_STK, bot_y - mid_y + H24_STK), clock_segment_radius(H24_STK), clock_segment_corners());
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

static void weather_cache_save(void) {
  WeatherCache cache = {
    .temperature_c_x10 = s_temperature_c_x10,
    .weather_icon = s_weather_icon,
    .sunrise_minute = s_sunrise_minute,
    .sunset_minute = s_sunset_minute,
    .high_c_x10 = s_high_c_x10,
    .low_c_x10 = s_low_c_x10,
    .have_temperature = s_have_temperature ? 1 : 0,
    .have_high_low = s_have_high_low ? 1 : 0,
  };
  persist_write_data(WEATHER_CACHE_PERSIST_KEY, &cache, sizeof(cache));
}

static void weather_cache_load(void) {
  if (!persist_exists(WEATHER_CACHE_PERSIST_KEY)) return;
  if (persist_get_size(WEATHER_CACHE_PERSIST_KEY) != (int)sizeof(WeatherCache)) return;

  WeatherCache cache;
  if (persist_read_data(WEATHER_CACHE_PERSIST_KEY, &cache, sizeof(cache)) !=
      (int)sizeof(cache)) {
    return;
  }

  s_temperature_c_x10 = cache.temperature_c_x10;
  s_weather_icon = cache.weather_icon;
  s_sunrise_minute = cache.sunrise_minute;
  s_sunset_minute = cache.sunset_minute;
  s_high_c_x10 = cache.high_c_x10;
  s_low_c_x10 = cache.low_c_x10;
  s_have_temperature = cache.have_temperature != 0;
  s_have_high_low = cache.have_high_low != 0;

  APP_LOG(APP_LOG_LEVEL_INFO,
          "Restored cached weather: temp=%ld icon=%ld",
          (long)s_temperature_c_x10,
          (long)s_weather_icon);
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
    const char *ampm = hour < 12 ? "AM" : "PM";
    snprintf(buffer, buffer_size, "%d:%02d %s", hour12, minute, ampm);
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
  GColor hour_color =
      (s_pro_unlocked && s_split_clock_colors)
        ? s_hour_color : s_settings.clock_color;
  GColor minute_color =
      (s_pro_unlocked && s_split_clock_colors)
        ? s_minute_color : s_settings.clock_color;

  if (s_settings.time_format == TIME_FORMAT_24H) {
    s_clock_draw_color = hour_color;
    // In 24-hour mode every numeral uses the same full-width seven-segment
    // geometry. In particular, "1" is no longer centered as a narrow special
    // case; its right-hand segments occupy the normal digit cell width.
    // Keep every numeral in the same allocated 24-hour cell width, but draw
    // "1" with the same intentional left-biased offsets as the 12-hour face.
    if (h1 == 1) draw_one_24(ctx, H24_H1_X, sy);
    else draw_digit_24(ctx, H24_H1_X, sy, h1, H24_DIGIT_WIDTH);

    if (h2 == 1) draw_one_24(ctx, H24_H2_X, sy);
    else draw_digit_24(ctx, H24_H2_X, sy, h2, H24_DIGIT_WIDTH);

    s_clock_draw_color = s_settings.clock_color;
    if (!s_flash_colon || (s_second % 2) == 0) {
      draw_colon(ctx, H24_COL_X, sy);
    }

    s_clock_draw_color = minute_color;
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

    s_clock_draw_color = hour_color;
    if (h1 == 1) draw_one_h1(ctx, sy);
    if (h2 == 1) draw_one(ctx, h2_x, sy); else draw_digit(ctx, h2_x, sy, h2);

    s_clock_draw_color = s_settings.clock_color;
    if (!s_flash_colon || (s_second % 2) == 0) {
      draw_colon(ctx, col_x, sy);
    }

    s_clock_draw_color = minute_color;
    if (m1 == 1) draw_one(ctx, m1_x, sy); else draw_digit(ctx, m1_x, sy, m1);
    if (m2 == 1) draw_one(ctx, m2_x, sy); else draw_digit(ctx, m2_x, sy, m2);
  }
}

static void draw_battery_icon(GContext *ctx, GRect r, int percent, GColor color);
static void draw_battery_icon_fat(GContext *ctx, GRect r, int percent, GColor color);
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
    draw_battery_icon(ctx, GRect(left_area.origin.x, 7, 36, 9), s_battery_percent, side_fg);
  else if (s_settings.top_left_slot == SLOT_BATTERY_ICON)
    draw_battery_icon_fat(ctx,
        GRect(left_area.origin.x + (left_area.size.w - 42) / 2, 15, 42, 16),
        s_battery_percent, side_fg);
  else if (s_settings.top_left_slot == SLOT_BLUETOOTH && s_bluetooth_connected)
    draw_bluetooth_icon(ctx, GPoint(left_area.origin.x + left_area.size.w/2, 24), 34, 30, side_fg, true);
  if (s_settings.top_center_slot == SLOT_BATTERY)
    draw_battery_icon(ctx, GRect(DATEBOX_X + 7, 7, 36, 9), s_battery_percent, center_fg);
  else if (s_settings.top_center_slot == SLOT_BATTERY_ICON)
    draw_battery_icon_fat(ctx,
        GRect(DATEBOX_X + (DATEBOX_W - 36) / 2, 15, 36, 16),
        s_battery_percent, center_fg);
  else if (s_settings.top_center_slot == SLOT_BLUETOOTH && s_bluetooth_connected)
    draw_bluetooth_icon(ctx, GPoint(DATEBOX_X + DATEBOX_W/2, 27), 34, 30, center_fg, true);
  if (s_settings.top_right_slot == SLOT_BATTERY)
    draw_battery_icon(ctx, GRect(right_area.origin.x + right_area.size.w - 36, 7, 36, 9), s_battery_percent, side_fg);
  else if (s_settings.top_right_slot == SLOT_BATTERY_ICON)
    draw_battery_icon_fat(ctx,
        GRect(right_area.origin.x + (right_area.size.w - 42) / 2, 15, 42, 16),
        s_battery_percent, side_fg);
  else if (s_settings.top_right_slot == SLOT_BLUETOOTH && s_bluetooth_connected)
    draw_bluetooth_icon(ctx, GPoint(right_area.origin.x + right_area.size.w/2, 24), 34, 30, side_fg, true);
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
  const bool interaction_visible = conditional_ui_is_visible();
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

static void draw_battery_icon_fat(GContext *ctx, GRect r, int percent, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);

  GRect body = GRect(r.origin.x, r.origin.y, r.size.w - 4, r.size.h);
  graphics_draw_rect(ctx, body);
  graphics_draw_rect(ctx,
                     GRect(body.origin.x + 1, body.origin.y + 1,
                           body.size.w - 2, body.size.h - 2));

  graphics_fill_rect(ctx,
                     GRect(r.origin.x + r.size.w - 3,
                           r.origin.y + (r.size.h / 2) - 3,
                           3, 6),
                     0, GCornerNone);

  int clamped = percent < 0 ? 0 : (percent > 100 ? 100 : percent);
  int inner_w = r.size.w - 11;
  int fill_w = (inner_w * clamped) / 100;
  if (fill_w > 0) {
    graphics_fill_rect(ctx,
                       GRect(r.origin.x + 4, r.origin.y + 4,
                             fill_w, r.size.h - 8),
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
    const int icon_w = 36;
    const int icon_h = 9;
    const int icon_y = 7;
    int icon_x = is_right_slot
                   ? (area.origin.x + area.size.w - icon_w)
                   : area.origin.x;
    draw_battery_icon(ctx, GRect(icon_x, icon_y, icon_w, icon_h),
                      s_battery_percent, color);
  } else if (slot == SLOT_BATTERY_ICON) {
    const int icon_w = 42;
    const int icon_h = 16;
    int icon_x = area.origin.x + (area.size.w - icon_w) / 2;
    draw_battery_icon_fat(ctx, GRect(icon_x, 15, icon_w, icon_h),
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
    draw_battery_icon(ctx, GRect(cx - 18, 7, 36, 9),
                      s_battery_percent, color);
  } else if (slot == CENTER_BATTERY_ICON) {
    draw_battery_icon_fat(ctx, GRect(cx - 18, 15, 36, 16),
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
static void tick_handler(struct tm *tick_time, TimeUnits changed);
static void update_tick_service(void);

typedef enum {
  TXT_WEATHER,
  TXT_STEPS,
  TXT_HR,
  TXT_BT,
  TXT_DAY,
  TXT_DATE,
  TXT_MONTH,
  TXT_CAL,
  TXT_DIST,
  TXT_RISE,
  TXT_SET,
  TXT_HIGH_LOW,
  TXT_TEMP
} WatchTextId;

typedef struct {
  const char *weather;
  const char *steps;
  const char *hr;
  const char *bt;
  const char *day;
  const char *date;
  const char *month;
  const char *cal;
  const char *dist;
  const char *rise;
  const char *set;
  const char *high_low;
  const char *temp;
  const char *days[7];
  const char *months[12];
} WatchTranslation;

static const WatchTranslation TRANSLATIONS[] = {
  {
    "WEATHER", "STEPS", "HR", "BT", "DAY", "DATE", "MONTH",
    "CAL", "DIST", "RISE", "SET", "H/L", "TEMP",
    { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" },
    { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
      "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" }
  },
  {
    "VÄDER", "STEG", "PULS", "BT", "DAG", "DATUM", "MÅNAD",
    "KCAL", "DIST", "UPP", "NED", "H/L", "TEMP",
    { "SÖN", "MÅN", "TIS", "ONS", "TOR", "FRE", "LÖR" },
    { "JAN", "FEB", "MAR", "APR", "MAJ", "JUN",
      "JUL", "AUG", "SEP", "OKT", "NOV", "DEC" }
  },
  {
    "CLIMA", "PASOS", "PULSO", "BT", "DÍA", "FECHA", "MES",
    "KCAL", "DIST", "SALE", "PONE", "M/M", "TEMP",
    { "DOM", "LUN", "MAR", "MIÉ", "JUE", "VIE", "SÁB" },
    { "ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
      "JUL", "AGO", "SEP", "OCT", "NOV", "DIC" }
  },
  {
    "MÉTÉO", "PAS", "POULS", "BT", "JOUR", "DATE", "MOIS",
    "KCAL", "DIST", "LEVE", "COUC", "H/B", "TEMP",
    { "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM" },
    { "JAN", "FÉV", "MAR", "AVR", "MAI", "JUN",
      "JUL", "AOÛ", "SEP", "OCT", "NOV", "DÉC" }
  },
  {
    "WETTER", "SCHR", "PULS", "BT", "TAG", "DATUM", "MONAT",
    "KCAL", "DIST", "AUFG", "UNTR", "H/T", "TEMP",
    { "SO", "MO", "DI", "MI", "DO", "FR", "SA" },
    { "JAN", "FEB", "MÄR", "APR", "MAI", "JUN",
      "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ" }
  },
  {
    "TEMPO", "PASSOS", "PULSO", "BT", "DIA", "DATA", "MÊS",
    "KCAL", "DIST", "NASCE", "PÕE", "M/M", "TEMP",
    { "DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SÁB" },
    { "JAN", "FEV", "MAR", "ABR", "MAI", "JUN",
      "JUL", "AGO", "SET", "OUT", "NOV", "DEZ" }
  },
  {
    "TEMPS", "PASSOS", "POLS", "BT", "DIA", "DATA", "MES",
    "KCAL", "DIST", "SURT", "POSTA", "M/M", "TEMP",
    { "DG", "DL", "DT", "DC", "DJ", "DV", "DS" },
    { "GEN", "FEB", "MAR", "ABR", "MAI", "JUN",
      "JUL", "AGO", "SET", "OCT", "NOV", "DES" }
  }
};

static const WatchTranslation *watch_translation(void) {
  uint8_t language = s_settings.language;
  if (language >= ARRAY_LENGTH(TRANSLATIONS)) language = LANG_ENGLISH;
  return &TRANSLATIONS[language];
}

static const char *watch_text(WatchTextId id) {
  const WatchTranslation *t = watch_translation();
  switch (id) {
    case TXT_WEATHER: return t->weather;
    case TXT_STEPS: return t->steps;
    case TXT_HR: return t->hr;
    case TXT_BT: return t->bt;
    case TXT_DAY: return t->day;
    case TXT_DATE: return t->date;
    case TXT_MONTH: return t->month;
    case TXT_CAL: return t->cal;
    case TXT_DIST: return t->dist;
    case TXT_RISE: return t->rise;
    case TXT_SET: return t->set;
    case TXT_HIGH_LOW: return t->high_low;
    case TXT_TEMP: return t->temp;
    default: return "";
  }
}

static void format_time_zone_value(uint8_t preset_index,
                                   char *buffer,
                                   size_t buffer_size) {
  if (preset_index >= TIME_ZONE_PRESET_COUNT) preset_index = 0;

  time_t now = time(NULL);
  struct tm *time_ptr = NULL;

  if (TIME_ZONE_PRESETS[preset_index].use_local) {
    time_ptr = localtime(&now);
  } else {
    time_t shifted =
        now + ((time_t)TIME_ZONE_PRESETS[preset_index].utc_offset_minutes * 60);
    time_ptr = gmtime(&shifted);
  }

  if (!time_ptr) {
    snprintf(buffer, buffer_size, "--");
    return;
  }

  if (s_settings.time_format == TIME_FORMAT_24H) {
    snprintf(buffer, buffer_size, "%02d:%02d",
             time_ptr->tm_hour, time_ptr->tm_min);
  } else {
    int hour12 = time_ptr->tm_hour % 12;
    if (hour12 == 0) hour12 = 12;
    snprintf(buffer, buffer_size, "%d:%02d",
             hour12, time_ptr->tm_min);
  }
}

static const char *side_slot_label(uint8_t slot) {
  switch (slot) {
    case SLOT_STEPS: return watch_text(TXT_STEPS);
    case SLOT_BATTERY: return "";
    case SLOT_HEART_RATE: return watch_text(TXT_HR);
    case SLOT_BLUETOOTH: return s_bluetooth_connected ? "" : watch_text(TXT_BT);
    case SLOT_DAY: return watch_text(TXT_DAY);
    case SLOT_DATE: return watch_text(TXT_DATE);
    case SLOT_MONTH: return watch_text(TXT_MONTH);
    case SLOT_CALORIES: return watch_text(TXT_CAL);
    case SLOT_DISTANCE: return watch_text(TXT_DIST);
    case SLOT_SUNRISE: return watch_text(TXT_RISE);
    case SLOT_SUNSET: return watch_text(TXT_SET);
    case SLOT_HIGH_LOW: return watch_text(TXT_HIGH_LOW);
    case SLOT_TIME_ZONE: return "TZ";
    case SLOT_SECONDS: return "";
    case SLOT_WEATHER:
    default: return watch_text(TXT_WEATHER);
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
    case SLOT_TIME_ZONE:
      return "";
    case SLOT_SECONDS:
      snprintf(s_seconds_buf, sizeof(s_seconds_buf), "%02d", s_second);
      return s_seconds_buf;
    case SLOT_BATTERY_ICON:
      return "";
    case SLOT_BATTERY_PERCENT:
      snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", s_battery_percent);
      return s_battery_buf;
    case SLOT_WEATHER:
    default:
      return s_weather_buf;
  }
}

static const char *top_slot_label(uint8_t slot);

static const char *time_zone_label(uint8_t preset_index) {
  if (preset_index >= TIME_ZONE_PRESET_COUNT) preset_index = 0;
  return TIME_ZONE_PRESETS[preset_index].label;
}

static const char *top_side_slot_label(uint8_t slot, bool left_side) {
  if (slot != SLOT_TIME_ZONE) return top_slot_label(slot);
  return time_zone_label(
      left_side ? s_top_left_time_zone : s_top_right_time_zone);
}

static const char *bottom_side_slot_label(uint8_t slot, bool left_side) {
  if (slot != SLOT_TIME_ZONE) return side_slot_label(slot);
  return time_zone_label(
      left_side ? s_left_time_zone : s_right_time_zone);
}

static const char *top_side_slot_value(uint8_t slot, bool left_side) {
  if (slot != SLOT_TIME_ZONE) return side_slot_value(slot);

  if (left_side) {
    format_time_zone_value(s_top_left_time_zone,
                           s_tz_top_left_buf, sizeof(s_tz_top_left_buf));
    return s_tz_top_left_buf;
  }

  format_time_zone_value(s_top_right_time_zone,
                         s_tz_top_right_buf, sizeof(s_tz_top_right_buf));
  return s_tz_top_right_buf;
}

static const char *bottom_side_slot_value(uint8_t slot, bool left_side) {
  if (slot != SLOT_TIME_ZONE) return side_slot_value(slot);

  if (left_side) {
    format_time_zone_value(s_left_time_zone,
                           s_tz_left_buf, sizeof(s_tz_left_buf));
    return s_tz_left_buf;
  }

  format_time_zone_value(s_right_time_zone,
                         s_tz_right_buf, sizeof(s_tz_right_buf));
  return s_tz_right_buf;
}

static const char *center_slot_label(void) {
  switch (s_settings.center_slot) {
    case CENTER_BATTERY: return "";
    case CENTER_BATTERY_ICON: return "";
    case CENTER_BATTERY_PERCENT: return "";
    case CENTER_SECONDS: return "";
    case CENTER_BLUETOOTH: return s_bluetooth_connected ? "" : watch_text(TXT_BT);
    case CENTER_WEATHER: return watch_text(TXT_TEMP);
    case CENTER_STEPS: return watch_text(TXT_STEPS);
    case CENTER_DATE: return watch_text(TXT_DATE);
    case CENTER_HEART_RATE:
    default: return watch_text(TXT_HR);
  }
}

static const char *center_slot_value(void) {
  switch (s_settings.center_slot) {
    case CENTER_BATTERY:
      if (s_battery_percent == 100) snprintf(s_battery_buf, sizeof(s_battery_buf), "100");
      else snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", s_battery_percent);
      return s_battery_buf;
    case CENTER_BATTERY_ICON:
      return "";
    case CENTER_BATTERY_PERCENT:
      snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", s_battery_percent);
      return s_battery_buf;
    case CENTER_SECONDS:
      snprintf(s_seconds_buf, sizeof(s_seconds_buf), "%02d", s_second);
      return s_seconds_buf;
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

static bool side_slot_has_optional_label(uint8_t slot) {
  return slot == SLOT_WEATHER ||
         slot == SLOT_STEPS ||
         slot == SLOT_HEART_RATE ||
         slot == SLOT_CALORIES ||
         slot == SLOT_DISTANCE ||
         slot == SLOT_SUNRISE ||
         slot == SLOT_SUNSET ||
         slot == SLOT_HIGH_LOW ||
         slot == SLOT_TIME_ZONE;
}

static bool center_slot_has_optional_label(uint8_t slot) {
  return slot == CENTER_HEART_RATE ||
         slot == CENTER_WEATHER ||
         slot == CENTER_STEPS;
}

static bool weather_value_needs_smaller_font(void) {
  // s_weather_buf includes the degree symbol. Three-digit temperatures (100°+)
  // and similarly long negative temperatures are safer in the medium font
  // than the large calendar font.
  if (!s_have_temperature) return false;

  int display_x10 = s_temperature_c_x10;
  if (s_settings.temp_unit == TEMP_FAHRENHEIT) {
    display_x10 = (s_temperature_c_x10 * 9) / 5 + 320;
  }

  int display_temp =
      display_x10 >= 0 ? (display_x10 + 5) / 10 : (display_x10 - 5) / 10;

  return display_temp >= 100 || display_temp <= -10;
}

static bool side_slot_needs_medium_hidden_font(uint8_t slot) {
  // Distance is intentionally medium at all times because its unit makes the
  // rendered value wider than the other single-value metrics.
  if (slot == SLOT_DISTANCE) return true;
  if (slot == SLOT_SUNRISE || slot == SLOT_SUNSET) return true;
  if (slot == SLOT_HIGH_LOW) return true;
  if (slot == SLOT_TIME_ZONE) return true;
  if (slot == SLOT_STEPS && s_step_count >= 10000) return true;
  if (slot == SLOT_WEATHER && weather_value_needs_smaller_font()) return true;
  return false;
}

static bool center_slot_needs_medium_hidden_font(uint8_t slot) {
  if (slot == CENTER_STEPS && s_step_count >= 10000) return true;
  if (slot == CENTER_WEATHER && weather_value_needs_smaller_font()) return true;
  return false;
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

  const bool left_calendar = slot_is_calendar(s_settings.top_left_slot);
  const bool center_calendar = slot_is_calendar(s_settings.top_center_slot);
  const bool right_calendar = slot_is_calendar(s_settings.top_right_slot);

  const bool left_label_hidden =
      s_settings.top_left_hide_label &&
      side_slot_has_optional_label(s_settings.top_left_slot);
  const bool center_label_hidden =
      s_settings.top_center_hide_label &&
      side_slot_has_optional_label(s_settings.top_center_slot);
  const bool right_label_hidden =
      s_settings.top_right_hide_label &&
      side_slot_has_optional_label(s_settings.top_right_slot);

  const bool left_large = left_calendar || left_label_hidden ||
      s_settings.top_left_slot == SLOT_BATTERY_PERCENT ||
      s_settings.top_left_slot == SLOT_SECONDS;
  const bool center_large = center_calendar || center_label_hidden ||
      s_settings.top_center_slot == SLOT_BATTERY_PERCENT ||
      s_settings.top_center_slot == SLOT_SECONDS;
  const bool right_large = right_calendar || right_label_hidden ||
      s_settings.top_right_slot == SLOT_BATTERY_PERCENT ||
      s_settings.top_right_slot == SLOT_SECONDS;

  text_layer_set_text(
      s_top_left_label,
      (left_label_hidden ||
       s_settings.top_left_slot == SLOT_BATTERY_ICON ||
       s_settings.top_left_slot == SLOT_BATTERY_PERCENT ||
       s_settings.top_left_slot == SLOT_SECONDS)
          ? "" : top_side_slot_label(s_settings.top_left_slot, true));
  text_layer_set_text(
      s_top_left_val,
      top_side_slot_value(s_settings.top_left_slot, true));

  const char *center_label =
      s_settings.top_center_slot == SLOT_WEATHER
          ? "TEMP"
          : top_slot_label(s_settings.top_center_slot);

  text_layer_set_text(
      s_top_center_label,
      (center_label_hidden ||
       s_settings.top_center_slot == SLOT_BATTERY_ICON ||
       s_settings.top_center_slot == SLOT_BATTERY_PERCENT ||
       s_settings.top_center_slot == SLOT_SECONDS)
          ? "" : center_label);
  text_layer_set_text(
      s_top_center_val,
      top_slot_value(s_settings.top_center_slot));

  text_layer_set_text(
      s_top_right_label,
      (right_label_hidden ||
       s_settings.top_right_slot == SLOT_BATTERY_ICON ||
       s_settings.top_right_slot == SLOT_BATTERY_PERCENT ||
       s_settings.top_right_slot == SLOT_SECONDS)
          ? "" : top_side_slot_label(s_settings.top_right_slot, false));
  text_layer_set_text(
      s_top_right_val,
      top_side_slot_value(s_settings.top_right_slot, false));

  // Hidden-label data normally uses the same large font as DAY / DATE / MONTH.
  // Long values use a 28px medium font before Pebble can ellipsize.
  const bool left_medium_for_fit =
      left_label_hidden &&
      side_slot_needs_medium_hidden_font(s_settings.top_left_slot);
  const bool center_medium_for_fit =
      (center_label_hidden &&
       side_slot_needs_medium_hidden_font(s_settings.top_center_slot)) ||
      s_settings.top_center_slot == SLOT_BATTERY_PERCENT;
  const bool right_medium_for_fit =
      right_label_hidden &&
      side_slot_needs_medium_hidden_font(s_settings.top_right_slot);

  text_layer_set_font(
      s_top_left_val,
      left_medium_for_fit ? s_font_medium :
        (left_large ? s_font_header : s_font_value));
  text_layer_set_font(
      s_top_center_val,
      center_medium_for_fit ? s_font_medium :
        (center_large ? s_font_header : s_font_value));
  text_layer_set_font(
      s_top_right_val,
      right_medium_for_fit ? s_font_medium :
        (right_large ? s_font_header : s_font_value));

  int left_w = DATEBOX_X - BOX_GAP - 4;
  int top_right_x = DATEBOX_X + DATEBOX_W + BOX_GAP;
  int top_right_w = SCREEN_W - top_right_x - 4;

  const bool left_full_value =
      left_label_hidden ||
      s_settings.top_left_slot == SLOT_BATTERY_PERCENT ||
      s_settings.top_left_slot == SLOT_SECONDS;
  const bool center_full_value =
      center_label_hidden ||
      s_settings.top_center_slot == SLOT_BATTERY_PERCENT ||
      s_settings.top_center_slot == SLOT_SECONDS;
  const bool right_full_value =
      right_label_hidden ||
      s_settings.top_right_slot == SLOT_BATTERY_PERCENT ||
      s_settings.top_right_slot == SLOT_SECONDS;

  layer_set_frame(
      text_layer_get_layer(s_top_left_val),
      GRect(4,
            left_calendar ? 4 :
              (left_full_value ? 4 : 15),
            left_w,
            left_calendar ? HEADER_H - 5 :
              (left_full_value ? HEADER_H - 5 : 34)));

  const int top_center_value_x =
      DATEBOX_X + ((s_settings.top_center_slot == SLOT_WEATHER) ? 2 : 0);
  const int top_center_value_w =
      DATEBOX_W - ((s_settings.top_center_slot == SLOT_WEATHER) ? 2 : 0);

  layer_set_frame(
      text_layer_get_layer(s_top_center_val),
      GRect(top_center_value_x,
            center_calendar ? 4 :
              (center_full_value ? 4 :
                (s_settings.top_center_slot == SLOT_BATTERY ? 14 : 15)),
            top_center_value_w,
            center_calendar ? HEADER_H - 5 :
              (center_full_value ? HEADER_H - 5 :
                (s_settings.top_center_slot == SLOT_BATTERY ? 38 : 34))));

  layer_set_frame(
      text_layer_get_layer(s_top_right_val),
      GRect(top_right_x,
            right_calendar ? 4 :
              (right_full_value ? 4 : 15),
            top_right_w,
            right_calendar ? HEADER_H - 5 :
              (right_full_value ? HEADER_H - 5 : 34)));

  text_layer_set_text_alignment(
      s_top_left_label,
      (s_settings.top_left_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentLeft);

  text_layer_set_text_alignment(
      s_top_left_val,
      left_calendar ? GTextAlignmentCenter : GTextAlignmentLeft);

  text_layer_set_text_alignment(s_top_center_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_top_center_val, GTextAlignmentCenter);

  text_layer_set_text_alignment(
      s_top_right_label,
      (s_settings.top_right_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentRight);

  text_layer_set_text_alignment(
      s_top_right_val,
      right_calendar ? GTextAlignmentCenter : GTextAlignmentRight);

  if (s_header_layer) layer_mark_dirty(s_header_layer);
}

static void update_footer_content(void) {
  if (!s_left_label || !s_center_label || !s_right_label ||
      !s_weather_icon_left_layer || !s_weather_icon_right_layer) return;

  const bool left_calendar = slot_is_calendar(s_settings.left_slot);
  const bool center_calendar = slot_is_calendar(s_settings.center_slot);
  const bool right_calendar = slot_is_calendar(s_settings.right_slot);

  const bool left_label_hidden =
      s_settings.left_hide_label &&
      side_slot_has_optional_label(s_settings.left_slot);
  const bool center_label_hidden =
      s_settings.center_hide_label &&
      center_slot_has_optional_label(s_settings.center_slot);
  const bool right_label_hidden =
      s_settings.right_hide_label &&
      side_slot_has_optional_label(s_settings.right_slot);

  const bool left_large = left_calendar || left_label_hidden ||
      s_settings.left_slot == SLOT_BATTERY_PERCENT ||
      s_settings.left_slot == SLOT_SECONDS;
  const bool center_large = center_calendar || center_label_hidden ||
      s_settings.center_slot == CENTER_BATTERY_PERCENT ||
      s_settings.center_slot == CENTER_SECONDS;
  const bool right_large = right_calendar || right_label_hidden ||
      s_settings.right_slot == SLOT_BATTERY_PERCENT ||
      s_settings.right_slot == SLOT_SECONDS;

  text_layer_set_text(
      s_left_label,
      (left_calendar || left_label_hidden ||
       s_settings.left_slot == SLOT_BATTERY_ICON ||
       s_settings.left_slot == SLOT_BATTERY_PERCENT ||
       s_settings.left_slot == SLOT_SECONDS)
          ? "" : bottom_side_slot_label(s_settings.left_slot, true));
  text_layer_set_text(s_left_val, bottom_side_slot_value(s_settings.left_slot, true));

  text_layer_set_text(
      s_center_label,
      (center_calendar || center_label_hidden ||
       s_settings.center_slot == CENTER_BATTERY_ICON ||
       s_settings.center_slot == CENTER_BATTERY_PERCENT ||
       s_settings.center_slot == CENTER_SECONDS)
          ? "" : center_slot_label());
  text_layer_set_text(s_center_val, center_slot_value());

  text_layer_set_text(
      s_right_label,
      (right_calendar || right_label_hidden ||
       s_settings.right_slot == SLOT_BATTERY_ICON ||
       s_settings.right_slot == SLOT_BATTERY_PERCENT ||
       s_settings.right_slot == SLOT_SECONDS)
          ? "" : bottom_side_slot_label(s_settings.right_slot, false));
  text_layer_set_text(s_right_val, bottom_side_slot_value(s_settings.right_slot, false));

  const bool left_medium_for_fit =
      left_label_hidden &&
      side_slot_needs_medium_hidden_font(s_settings.left_slot);
  const bool center_medium_for_fit =
      (center_label_hidden &&
       center_slot_needs_medium_hidden_font(s_settings.center_slot)) ||
      s_settings.center_slot == CENTER_BATTERY_PERCENT;
  const bool right_medium_for_fit =
      right_label_hidden &&
      side_slot_needs_medium_hidden_font(s_settings.right_slot);

  text_layer_set_font(
      s_left_val,
      left_medium_for_fit ? s_font_medium :
        (left_large ? s_font_header : s_font_value));
  text_layer_set_font(
      s_center_val,
      center_medium_for_fit ? s_font_medium :
        (center_large ? s_font_header : s_font_value));
  text_layer_set_font(
      s_right_val,
      right_medium_for_fit ? s_font_medium :
        (right_large ? s_font_header : s_font_value));

  int left_w = HRBOX_X - BOX_GAP - 4;
  int right_x = HRBOX_X + BOX_W + BOX_GAP;
  int right_w = SCREEN_W - right_x - 4;

  const bool left_full_value =
      left_label_hidden ||
      s_settings.left_slot == SLOT_BATTERY_PERCENT ||
      s_settings.left_slot == SLOT_SECONDS;
  const bool center_full_value =
      center_label_hidden ||
      s_settings.center_slot == CENTER_BATTERY_PERCENT ||
      s_settings.center_slot == CENTER_SECONDS;
  const bool right_full_value =
      right_label_hidden ||
      s_settings.right_slot == SLOT_BATTERY_PERCENT ||
      s_settings.right_slot == SLOT_SECONDS;

  layer_set_frame(
      text_layer_get_layer(s_left_val),
      GRect(4,
            left_calendar ? 1 :
              (left_full_value ? 4 : 14),
            left_w,
            left_calendar ? FOOTER_H - 1 :
              (left_full_value ? FOOTER_H - 4 : 38)));

  const int center_value_x =
      HRBOX_X + ((s_settings.center_slot == CENTER_WEATHER) ? 2 : 0);
  const int center_value_w =
      BOX_W - ((s_settings.center_slot == CENTER_WEATHER) ? 2 : 0);

  layer_set_frame(
      text_layer_get_layer(s_center_val),
      GRect(center_value_x,
            center_calendar ? 1 :
              (center_full_value ? 4 : 14),
            center_value_w,
            center_calendar ? FOOTER_H - 1 :
              (center_full_value ? FOOTER_H - 4 : 38)));

  layer_set_frame(
      text_layer_get_layer(s_right_val),
      GRect(right_x,
            right_calendar ? 1 :
              (right_full_value ? 4 : 14),
            right_w,
            right_calendar ? FOOTER_H - 1 :
              (right_full_value ? FOOTER_H - 4 : 38)));

  // Weather icon uses the outer/right edge of its side slot. When data labels
  // are hidden, center the 25px icon against the full-height enlarged value.
  const int weather_icon_size = 25;
  const bool any_side_weather_label_hidden =
      (s_settings.left_slot == SLOT_WEATHER && left_label_hidden) ||
      (s_settings.right_slot == SLOT_WEATHER && right_label_hidden);
  const int weather_icon_y =
      any_side_weather_label_hidden ? ((FOOTER_H - weather_icon_size) / 2) : 18;

  const int left_weather_icon_x =
      4 + left_w - weather_icon_size;
  const int right_weather_icon_x =
      right_x;

  layer_set_frame(
      bitmap_layer_get_layer(s_weather_icon_left_layer),
      GRect(left_weather_icon_x, weather_icon_y,
            weather_icon_size, weather_icon_size));
  layer_set_frame(
      bitmap_layer_get_layer(s_weather_icon_right_layer),
      GRect(right_weather_icon_x, weather_icon_y,
            weather_icon_size, weather_icon_size));

  // Temperature keeps the full side-slot text width. The right-aligned
  // weather icon may overlap the far edge slightly; this is preferable to
  // shrinking the TextLayer enough that Pebble replaces the degree symbol
  // with an ellipsis.
  text_layer_set_text_alignment(
      s_left_label,
      (s_settings.left_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentLeft);
  text_layer_set_text_alignment(
      s_left_val,
      left_calendar ? GTextAlignmentCenter : GTextAlignmentLeft);

  text_layer_set_text_alignment(s_center_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_center_val, GTextAlignmentCenter);

  text_layer_set_text_alignment(
      s_right_label,
      (s_settings.right_slot == SLOT_BLUETOOTH && !s_bluetooth_connected)
          ? GTextAlignmentCenter : GTextAlignmentRight);
  text_layer_set_text_alignment(
      s_right_val,
      right_calendar ? GTextAlignmentCenter : GTextAlignmentRight);

  // Existing weather icon behavior remains unchanged.
  layer_set_hidden(
      bitmap_layer_get_layer(s_weather_icon_left_layer),
      s_settings.left_slot != SLOT_WEATHER);
  layer_set_hidden(
      bitmap_layer_get_layer(s_weather_icon_right_layer),
      s_settings.right_slot != SLOT_WEATHER);

  if (s_footer_layer) layer_mark_dirty(s_footer_layer);
}


static void apply_bar_visibility(void);

static bool conditional_ui_is_visible(void) {
  // Never cache physical backlight state. The only synthetic state is the
  // bounded sunlight fallback.
  return light_is_on() || s_sunlight_fallback_active;
}

static void apply_bar_visibility(void) {
  const bool backlight_visible = conditional_ui_is_visible();

  if (s_header_layer) {
    const bool header_visible =
        (s_settings.header_mode == BAR_ALWAYS) ||
        (s_settings.header_mode == BAR_BACKLIGHT && backlight_visible);
    layer_set_hidden(s_header_layer, !header_visible);
  }

  if (s_footer_layer) {
    const bool footer_visible =
        (s_settings.footer_mode == BAR_ALWAYS) ||
        (s_settings.footer_mode == BAR_BACKLIGHT && backlight_visible);
    layer_set_hidden(s_footer_layer, !footer_visible);
  }
}

static void sunlight_fallback_timeout(void *context) {
  s_sunlight_fallback_timer = NULL;
  s_sunlight_fallback_active = false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sunlight fallback expired");
  apply_bar_visibility();
  update_stepbar_layout();
}

static void cancel_sunlight_fallback(void) {
  if (s_sunlight_fallback_timer) {
    app_timer_cancel(s_sunlight_fallback_timer);
    s_sunlight_fallback_timer = NULL;
  }
  if (s_license_status_retry_timer) {
    app_timer_cancel(s_license_status_retry_timer);
    s_license_status_retry_timer = NULL;
  }
  if (s_license_query_wait_timer) {
    app_timer_cancel(s_license_query_wait_timer);
    s_license_query_wait_timer = NULL;
  }
  s_sunlight_fallback_active = false;
}

static void start_sunlight_fallback(void) {
  if (s_sunlight_fallback_timer) {
    app_timer_cancel(s_sunlight_fallback_timer);
    s_sunlight_fallback_timer = NULL;
  }

  s_sunlight_fallback_active = true;
  s_sunlight_fallback_timer =
      app_timer_register(SUNLIGHT_FALLBACK_MS, sunlight_fallback_timeout, NULL);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sunlight fallback started");
  apply_bar_visibility();
  update_stepbar_layout();
}

static void request_light_with_fallback(void) {
  light_enable_interaction();

  // If the LED comes on asynchronously, BacklightService ON cancels this
  // immediately. If bright ambient light suppresses it, fallback stays active.
  if (!light_is_on()) {
    start_sunlight_fallback();
  } else {
    cancel_sunlight_fallback();
    apply_bar_visibility();
    update_stepbar_layout();
  }
}

static void touch_handler(const TouchEvent *event, void *context) {
  if (!event || event->type != TouchEvent_Touchdown) return;
  request_light_with_fallback();
}

static void backlight_handler(bool on) {
  // The real backlight supersedes the sunlight fallback.
  if (on) {
    cancel_sunlight_fallback();
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG,
          "Backlight event: %s fallback=%d",
          on ? "ON" : "OFF",
          s_sunlight_fallback_active ? 1 : 0);

  apply_bar_visibility();
  update_stepbar_layout();
}

static void focus_handler(bool in_focus) {
  if (!in_focus) return;

  APP_LOG(APP_LOG_LEVEL_DEBUG,
          "Focus returned: light=%d",
          light_is_on() ? 1 : 0);

  if (light_is_on()) {
    cancel_sunlight_fallback();
    apply_bar_visibility();
    update_stepbar_layout();
  } else {
    request_light_with_fallback();
  }
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

  // Synchronize from the live system backlight state.
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
        // Request Pebble's normal backlight interaction. Conditional UI follows
        // the actual system backlight state and has no independent timeout.
        request_light_with_fallback();

        s_raise_last_wake_at = now_ms;
        APP_LOG(APP_LOG_LEVEL_INFO,
                "Raise wake: mode=%d x=%d y=%d z=%d light=%d",
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


static bool purchased_pro_is_persisted(void) {
  return persist_exists(PURCHASED_PRO_PERSIST_KEY) &&
         persist_read_bool(PURCHASED_PRO_PERSIST_KEY);
}

static void purchased_pro_set_persisted(bool purchased) {
  if (purchased) {
    persist_write_bool(PURCHASED_PRO_PERSIST_KEY, true);
    APP_LOG(APP_LOG_LEVEL_INFO, "Purchased Pro marker persisted");
  } else {
    if (persist_exists(PURCHASED_PRO_PERSIST_KEY)) {
      persist_delete(PURCHASED_PRO_PERSIST_KEY);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Purchased Pro marker cleared");
  }
}

static bool pro_trial_is_active(void) {
  if (!persist_exists(PRO_TRIAL_PERSIST_KEY)) {
    s_trial_active = false;
    return false;
  }

  time_t expires_at = (time_t)persist_read_int(PRO_TRIAL_PERSIST_KEY);
  time_t now = time(NULL);

  if (expires_at > now) {
    s_trial_active = true;
    return true;
  }

  // Preserve a permanent used-trial marker. Deleting this value would make
  // pro_trial_start() believe the user had never taken a trial and allow a
  // second 48-hour period.
  pro_trial_mark_expired();
  return false;
}

static bool pro_trial_has_been_used(void) {
  // A negative persisted value marks an already-used/expired trial.
  return persist_exists(PRO_TRIAL_PERSIST_KEY) &&
         persist_read_int(PRO_TRIAL_PERSIST_KEY) < 0;
}

static void pro_trial_mark_expired(void) {
  persist_write_int(PRO_TRIAL_PERSIST_KEY, -1);
  s_trial_active = false;
}

static void pro_trial_start(void) {
  if (pro_trial_is_active()) {
    license_set_pro(true);
    return;
  }

  if (pro_trial_has_been_used()) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Pro trial already used");
    return;
  }

  time_t expires_at = time(NULL) + PRO_TRIAL_SECONDS;
  persist_write_int(PRO_TRIAL_PERSIST_KEY, (int32_t)expires_at);
  s_trial_active = true;
  APP_LOG(APP_LOG_LEVEL_INFO, "Big Time Pro trial started; expires=%ld",
          (long)expires_at);
  license_set_pro(true);
}

static void pro_trial_refresh(void) {
  if (!persist_exists(PRO_TRIAL_PERSIST_KEY)) return;

  int32_t stored = persist_read_int(PRO_TRIAL_PERSIST_KEY);
  if (stored < 0) {
    s_trial_active = false;
    return;
  }

  if ((time_t)stored > time(NULL)) {
    s_trial_active = true;
    license_set_pro(true);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Big Time Pro trial expired");
    pro_trial_mark_expired();

    // If the user has not purchased Pro, transition out of trial immediately.
    // license_set_pro(false) captures the complete trial configuration into
    // s_saved_settings BEFORE Free defaults are applied, so a later purchase
    // restores exactly what the user configured during the trial.
    if (!s_kiezelpay_licensed && !purchased_pro_is_persisted()) {
      license_set_pro(false);
    }
  }
}

static bool kiezelpay_event_callback(kiezelpay_event e, void *extra_data) {
  switch (e) {
    case KIEZELPAY_LICENSED:
      APP_LOG(APP_LOG_LEVEL_INFO,
              "KiezelPay: LICENSED -> Purchased Pro");
      s_kiezelpay_status_known = true;
      s_kiezelpay_licensed = true;
      purchased_pro_set_persisted(true);
      license_set_pro(true);
      schedule_license_status_retry();
      break;

    case KIEZELPAY_CODE_AVAILABLE:
      APP_LOG(APP_LOG_LEVEL_INFO,
              "KiezelPay: purchase code available -> explicitly unlicensed");
      s_kiezelpay_status_known = true;
      s_kiezelpay_licensed = false;
      purchased_pro_set_persisted(false);
      if (!s_trial_active) {
        license_set_pro(false);
      }
      break;

    case KIEZELPAY_PURCHASE_STARTED:
      APP_LOG(APP_LOG_LEVEL_INFO, "KiezelPay: purchase started");
      break;

    case KIEZELPAY_ERROR:
      APP_LOG(APP_LOG_LEVEL_WARNING, "KiezelPay: error");
      // Do not revoke an already-known entitlement for a transient error.
      break;

    case KIEZELPAY_BLUETOOTH_UNAVAILABLE:
      APP_LOG(APP_LOG_LEVEL_WARNING, "KiezelPay: Bluetooth unavailable");
      break;

    case KIEZELPAY_INTERNET_UNAVAILABLE:
      APP_LOG(APP_LOG_LEVEL_WARNING, "KiezelPay: internet unavailable");
      break;

    default:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "KiezelPay: event=%d", (int)e);
      break;
  }

  // Let KiezelPay show its standard trial/purchase/license messages.
  return false;
}

static void license_query_wait_handler(void *context) {
  s_license_query_wait_timer = NULL;

  if (!s_license_query_waiting) return;

  if (s_kiezelpay_status_known || s_kiezelpay_licensed ||
      s_trial_active || s_license_query_wait_count >= 12) {
    s_license_query_waiting = false;
    s_license_query_wait_count = 0;
    license_send_status_to_phone();
    return;
  }

  s_license_query_wait_count++;
  s_license_query_wait_timer =
      app_timer_register(250, license_query_wait_handler, NULL);
}

static void send_license_status_when_ready(void) {
  // Purchased-license status may arrive asynchronously from KiezelPay's server.
  // Do not immediately tell Clay "Free" while that check is still unresolved.
  if (s_kiezelpay_status_known || s_kiezelpay_licensed || s_trial_active) {
    s_license_query_waiting = false;
    s_license_query_wait_count = 0;
    license_send_status_to_phone();
    return;
  }

  if (s_license_query_wait_timer) {
    app_timer_cancel(s_license_query_wait_timer);
    s_license_query_wait_timer = NULL;
  }

  s_license_query_waiting = true;
  s_license_query_wait_count = 0;
  s_license_query_wait_timer =
      app_timer_register(250, license_query_wait_handler, NULL);

  APP_LOG(APP_LOG_LEVEL_DEBUG,
          "Waiting for KiezelPay status before answering license check");
}

static void license_status_retry_handler(void *context) {
  s_license_status_retry_timer = NULL;
  license_send_status_to_phone();
}

static void schedule_license_status_retry(void) {
  if (s_license_status_retry_count >= 6) {
    APP_LOG(APP_LOG_LEVEL_WARNING,
            "License status sync gave up after %d retries",
            (int)s_license_status_retry_count);
    s_license_status_retry_count = 0;
    return;
  }

  s_license_status_retry_count++;

  if (s_license_status_retry_timer) {
    app_timer_cancel(s_license_status_retry_timer);
    s_license_status_retry_timer = NULL;
  }

  s_license_status_retry_timer =
      app_timer_register(250, license_status_retry_handler, NULL);
}

static void license_send_status_to_phone(void) {
  DictionaryIterator *iter = NULL;
  AppMessageResult begin_result = app_message_outbox_begin(&iter);

  if (begin_result != APP_MSG_OK || !iter) {
    APP_LOG(APP_LOG_LEVEL_DEBUG,
            "License status outbox busy/result=%d; retrying",
            (int)begin_result);
    schedule_license_status_retry();
    return;
  }

  // Reuse the existing PRO_LICENSE key as an entitlement state:
  //   0 = Free
  //   1 = Free Trial
  //   2 = Purchased / restored Pro
  uint8_t entitlement = 0;
  if (s_kiezelpay_licensed || purchased_pro_is_persisted()) {
    entitlement = 2;
  } else if (s_trial_active && s_pro_unlocked) {
    entitlement = 1;
  }

  dict_write_uint8(iter, KEY_PRO_LICENSE, entitlement);
  dict_write_uint8(iter, KEY_LANGUAGE, s_settings.language);

  int32_t trial_remaining = 0;
  if (entitlement == 1 && persist_exists(PRO_TRIAL_PERSIST_KEY)) {
    int32_t expires_at = persist_read_int(PRO_TRIAL_PERSIST_KEY);
    int32_t now = (int32_t)time(NULL);
    if (expires_at > now) {
      trial_remaining = expires_at - now;
    }
  } else if (entitlement == 0 && pro_trial_has_been_used()) {
    trial_remaining = -1;
  }
  dict_write_int32(iter, KEY_LICENSE_CHECK, trial_remaining);

  AppMessageResult send_result = app_message_outbox_send();
  if (send_result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_DEBUG,
            "License status send result=%d; retrying",
            (int)send_result);
    schedule_license_status_retry();
    return;
  }

  APP_LOG(APP_LOG_LEVEL_INFO,
          "License status sent: entitlement=%d trial_remaining=%ld",
          (int)entitlement,
          (long)trial_remaining);

  s_license_status_retry_count = 0;
}

static void license_refresh_ui(void) {
  if (!s_pro_unlocked) {
    enforce_free_defaults();
  }

  // Entitlement changes may change the effective presentation, but persistence
  // must retain the user's complete preference record.
  settings_save();

  APP_LOG(APP_LOG_LEVEL_INFO,
          "Effective UI: pro=%d header=%d footer=%d steps=%d raise=%d",
          s_pro_unlocked ? 1 : 0,
          (int)s_settings.header_mode,
          (int)s_settings.footer_mode,
          (int)s_settings.stepbar_mode,
          (int)s_settings.raise_wake_mode);

  update_footer_content();
  update_header_content();
  update_stepbar_layout();
  update_bar_input_services();
  update_raise_wake_service();
  update_tick_service();
  apply_bar_visibility();
  update_accent_text_contrast();
  update_background_contrast();

  if (s_clock_layer) layer_mark_dirty(s_clock_layer);
  if (s_stepbar_layer) layer_mark_dirty(s_stepbar_layer);
  if (s_header_layer) layer_mark_dirty(s_header_layer);
  if (s_footer_layer) layer_mark_dirty(s_footer_layer);

  license_send_status_to_phone();
}

static void license_set_pro(bool unlocked) {
  if (s_pro_unlocked == unlocked) {
    license_send_status_to_phone();
    return;
  }

  if (!unlocked && s_pro_unlocked) {
    // Capture the complete Pro/trial configuration BEFORE switching to the
    // enforced Free presentation.
    s_saved_settings = s_settings;
    s_saved_settings_valid = true;
  }

  s_pro_unlocked = unlocked;

  if (unlocked && s_saved_settings_valid) {
    // Restore the user's actual preferences after trial/license validation.
    // This is the critical part that prevents a watchface process restart from
    // turning BAR_BACKLIGHT into BAR_ALWAYS and Raise to Wake into Off.
    s_settings = s_saved_settings;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Pro license -> %s", unlocked ? "UNLOCKED" : "FREE");
  license_refresh_ui();
}

// KiezelPay integration point:
// Call this from the product-specific KiezelPay license callback.
// Only a verified KiezelPay license should call this true. The optional Pro trial is managed separately by Big Time.
void watchface_kiezelpay_set_licensed(bool licensed) {
  license_set_pro(licensed);
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

    // KiezelPay's pebble-events handler is registered before this handler and
    // processes/validates the same incoming status dictionary first. Mirror its
    // resolved status into Big Time's runtime entitlement so the settings UI
    // cannot disagree with KiezelPay's own license state.
    if (t->key == KPAY_KEY_STATUS_RESULT) {
      int32_t kpay_status = tuple_to_int32(t, -1);

      APP_LOG(APP_LOG_LEVEL_INFO,
              "KiezelPay status result observed: %ld",
              (long)kpay_status);

      s_kiezelpay_status_known = true;

      if (kpay_status == KPAY_STATUS_LICENSED) {
        s_kiezelpay_licensed = true;
        purchased_pro_set_persisted(true);
        license_set_pro(true);
      } else if (kpay_status == 0) {
        // Only an explicit validated "unlicensed" result may revoke a
        // previously purchased entitlement. Unknown/transient statuses never
        // demote a paying customer.
        APP_LOG(APP_LOG_LEVEL_INFO,
                "KiezelPay explicitly reports unlicensed");
        s_kiezelpay_licensed = false;
        purchased_pro_set_persisted(false);
        if (!s_trial_active) {
          license_set_pro(false);
        }
      } else {
        APP_LOG(APP_LOG_LEVEL_DEBUG,
                "KiezelPay non-final status=%ld; preserving purchase marker",
                (long)kpay_status);
      }

      if (s_license_query_wait_timer) {
        app_timer_cancel(s_license_query_wait_timer);
        s_license_query_wait_timer = NULL;
      }
      s_license_query_waiting = false;
      s_license_query_wait_count = 0;

      // Send the newly resolved entitlement to the phone/Clay immediately.
      schedule_license_status_retry();
      continue;
    }

    // Security boundary: Pro controls are enforced on-watch, not just hidden in
    // the settings page. A crafted AppMessage cannot unlock premium settings.
    if (!s_pro_unlocked && key_is_pro_customization(t->key)) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Ignoring locked Pro setting key=%lu",
              (unsigned long)t->key);
      continue;
    }

    switch (t->key) {
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
        if (value >= SLOT_WEATHER && value <= SLOT_SECONDS) {
          s_settings.left_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Left slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_CENTER_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.center_slot);
        if ((value >= CENTER_HEART_RATE && value <= CENTER_MONTH &&
             value != CENTER_STEPS) ||
            value == CENTER_BATTERY_ICON ||
            value == CENTER_BATTERY_PERCENT ||
            value == CENTER_SECONDS) {
          s_settings.center_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Center slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_RIGHT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.right_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_SECONDS) {
          s_settings.right_slot = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO, "Right slot -> %ld", (long)value);
          layout_changed = true;
        }
        break;
      }

      case KEY_TOP_LEFT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.top_left_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_SECONDS) { s_settings.top_left_slot = (uint8_t)value; layout_changed = true; }
        break;
      }
      case KEY_TOP_CENTER_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.top_center_slot);
        if ((value >= SLOT_WEATHER && value <= SLOT_MONTH &&
             value != SLOT_STEPS) ||
            value == SLOT_BATTERY_ICON ||
            value == SLOT_BATTERY_PERCENT ||
            value == SLOT_SECONDS) {
          s_settings.top_center_slot = (uint8_t)value;
          layout_changed = true;
        }
        break;
      }
      case KEY_TOP_RIGHT_SLOT: {
        int32_t value = tuple_to_int32(t, s_settings.top_right_slot);
        if (value >= SLOT_WEATHER && value <= SLOT_SECONDS) { s_settings.top_right_slot = (uint8_t)value; layout_changed = true; }
        break;
      }

      case KEY_TOP_LEFT_TIME_ZONE: {
        int32_t value = tuple_to_int32(t, s_top_left_time_zone);
        if (value >= 0 && value < TIME_ZONE_PRESET_COUNT) {
          s_top_left_time_zone = (uint8_t)value;
          persist_write_int(TZ_TOP_LEFT_PERSIST_KEY, value);
          layout_changed = true;
        }
        break;
      }

      case KEY_TOP_RIGHT_TIME_ZONE: {
        int32_t value = tuple_to_int32(t, s_top_right_time_zone);
        if (value >= 0 && value < TIME_ZONE_PRESET_COUNT) {
          s_top_right_time_zone = (uint8_t)value;
          persist_write_int(TZ_TOP_RIGHT_PERSIST_KEY, value);
          layout_changed = true;
        }
        break;
      }

      case KEY_LEFT_TIME_ZONE: {
        int32_t value = tuple_to_int32(t, s_left_time_zone);
        if (value >= 0 && value < TIME_ZONE_PRESET_COUNT) {
          s_left_time_zone = (uint8_t)value;
          persist_write_int(TZ_LEFT_PERSIST_KEY, value);
          layout_changed = true;
        }
        break;
      }

      case KEY_RIGHT_TIME_ZONE: {
        int32_t value = tuple_to_int32(t, s_right_time_zone);
        if (value >= 0 && value < TIME_ZONE_PRESET_COUNT) {
          s_right_time_zone = (uint8_t)value;
          persist_write_int(TZ_RIGHT_PERSIST_KEY, value);
          layout_changed = true;
        }
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

      case KEY_LANGUAGE: {
        int32_t value = tuple_to_int32(t, s_settings.language);
        if (value >= LANG_ENGLISH && value <= LANG_CATALAN) {
          s_settings.language = (uint8_t)value;
          APP_LOG(APP_LOG_LEVEL_INFO,
                  "Watchface language -> %ld", (long)value);
          layout_changed = true;
          time_t now = time(NULL);
          struct tm *now_tm = localtime(&now);
          if (now_tm) update_time(now_tm);
        }
        break;
      }

      case KEY_TOP_LEFT_HIDE_LABEL:
        s_settings.top_left_hide_label =
            tuple_to_int32(t, s_settings.top_left_hide_label) ? 1 : 0;
        layout_changed = true;
        break;

      case KEY_TOP_CENTER_HIDE_LABEL:
        s_settings.top_center_hide_label =
            tuple_to_int32(t, s_settings.top_center_hide_label) ? 1 : 0;
        layout_changed = true;
        break;

      case KEY_TOP_RIGHT_HIDE_LABEL:
        s_settings.top_right_hide_label =
            tuple_to_int32(t, s_settings.top_right_hide_label) ? 1 : 0;
        layout_changed = true;
        break;

      case KEY_LEFT_HIDE_LABEL:
        s_settings.left_hide_label =
            tuple_to_int32(t, s_settings.left_hide_label) ? 1 : 0;
        layout_changed = true;
        break;

      case KEY_CENTER_HIDE_LABEL:
        s_settings.center_hide_label =
            tuple_to_int32(t, s_settings.center_hide_label) ? 1 : 0;
        layout_changed = true;
        break;

      case KEY_RIGHT_HIDE_LABEL:
        s_settings.right_hide_label =
            tuple_to_int32(t, s_settings.right_hide_label) ? 1 : 0;
        layout_changed = true;
        break;

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

      case KEY_HOUR_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          uint32_t value =
              (uint32_t)tuple_to_int32(t, 0xFFFFFF) & 0xFFFFFF;
          s_hour_color = GColorFromHEX(value);
          persist_write_int(HOUR_COLOR_PERSIST_KEY, (int32_t)value);
          if (s_clock_layer) layer_mark_dirty(s_clock_layer);
          APP_LOG(APP_LOG_LEVEL_INFO,
                  "Hour color -> 0x%06lX", (unsigned long)value);
        }
        break;

      case KEY_MINUTE_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          uint32_t value =
              (uint32_t)tuple_to_int32(t, 0xFFFFFF) & 0xFFFFFF;
          s_minute_color = GColorFromHEX(value);
          persist_write_int(MINUTE_COLOR_PERSIST_KEY, (int32_t)value);
          if (s_clock_layer) layer_mark_dirty(s_clock_layer);
          APP_LOG(APP_LOG_LEVEL_INFO,
                  "Minute color -> 0x%06lX", (unsigned long)value);
        }
        break;

      case KEY_SPLIT_CLOCK_COLORS: {
        bool enabled = tuple_to_int32(t, s_split_clock_colors ? 1 : 0) != 0;
        s_split_clock_colors = enabled;
        persist_write_int(SPLIT_COLOR_PERSIST_KEY, enabled ? 1 : 0);
        if (s_clock_layer) layer_mark_dirty(s_clock_layer);
        APP_LOG(APP_LOG_LEVEL_INFO,
                "Separate hour/minute colors -> %d", enabled ? 1 : 0);
        break;
      }

      case KEY_FLASH_COLON: {
        bool enabled = tuple_to_int32(t, s_flash_colon ? 1 : 0) != 0;
        s_flash_colon = enabled;
        persist_write_int(FLASH_COLON_PERSIST_KEY, enabled ? 1 : 0);
        update_tick_service();
        if (s_clock_layer) layer_mark_dirty(s_clock_layer);
        APP_LOG(APP_LOG_LEVEL_INFO, "Flashing colon -> %d", enabled ? 1 : 0);
        break;
      }

      case KEY_ROUNDED_TIME: {
        int value = tuple_to_int32(t, (int)s_time_style);
        if (value < TIME_STYLE_SQUARE || value > TIME_STYLE_SOFT_SQUARE) {
          value = TIME_STYLE_SQUARE;
        }
        s_time_style = (TimeStyle)value;
        persist_write_int(ROUNDED_TIME_PERSIST_KEY, value);
        if (s_clock_layer) layer_mark_dirty(s_clock_layer);
        APP_LOG(APP_LOG_LEVEL_INFO, "Time style -> %d", value);
        break;
      }

      case KEY_BACKGROUND_COLOR:
        if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
          new_background_hex = (uint32_t)tuple_to_int32(t, 0x000000) & 0xFFFFFF;
          background_changed = true;
        }
        break;
#endif

      case KEY_TRY_PRO_FREE: {
        int32_t trial_value = tuple_to_int32(t, 0);
        if (!s_kiezelpay_licensed) {
          if (trial_value < 0) {
            // Phone-side localStorage confirms this installation history has
            // already consumed its one-time trial (important after reinstall).
            pro_trial_mark_expired();
            license_set_pro(false);
            APP_LOG(APP_LOG_LEVEL_INFO, "Phone confirmed Pro trial already used");
          } else if (trial_value > 1) {
            // Restore only the ORIGINAL absolute expiry timestamp.
            time_t now = time(NULL);
            if ((time_t)trial_value > now && !pro_trial_has_been_used()) {
              persist_write_int(PRO_TRIAL_PERSIST_KEY, trial_value);
              s_trial_active = true;
              APP_LOG(APP_LOG_LEVEL_INFO,
                      "Restored Pro trial expiry from phone: %ld",
                      (long)trial_value);
              license_set_pro(true);
            } else {
              pro_trial_mark_expired();
              license_set_pro(false);
              APP_LOG(APP_LOG_LEVEL_INFO,
                      "Rejected used/expired Pro trial restore: %ld",
                      (long)trial_value);
            }
          } else if (trial_value == 1) {
            pro_trial_start();
          }
        }
        break;
      }

      case KEY_UNLOCK_PRO:
        if (tuple_to_int32(t, 0) != 0 &&
            !s_kiezelpay_licensed &&
            !purchased_pro_is_persisted()) {
          APP_LOG(APP_LOG_LEVEL_INFO,
                  "Starting KiezelPay purchase on user request");
          kiezelpay_start_purchase();
        } else if (tuple_to_int32(t, 0) != 0 &&
                   purchased_pro_is_persisted()) {
          APP_LOG(APP_LOG_LEVEL_INFO,
                  "Purchase/restore requested but Purchased Pro is already persisted");
          s_kiezelpay_licensed = true;
          license_set_pro(true);
        }
        break;

      case KEY_LICENSE_CHECK:
        pro_trial_refresh();
        send_license_status_when_ready();
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
  }

  if (accent_changed || clock_color_changed || background_changed || layout_changed || temperature_setting_changed) {
    settings_save();
  }
#endif

  if (weather_changed) {
    weather_cache_save();
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
    update_tick_service();
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
static bool seconds_display_is_used(void) {
  return s_settings.top_left_slot == SLOT_SECONDS ||
         s_settings.top_center_slot == SLOT_SECONDS ||
         s_settings.top_right_slot == SLOT_SECONDS ||
         s_settings.left_slot == SLOT_SECONDS ||
         s_settings.center_slot == CENTER_SECONDS ||
         s_settings.right_slot == SLOT_SECONDS;
}

static bool second_ticks_are_needed(void) {
  return (s_pro_unlocked && s_flash_colon) || seconds_display_is_used();
}

static void update_tick_service(void) {
  bool need_seconds = second_ticks_are_needed();
  if (need_seconds == s_second_tick_mode) return;
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(
      need_seconds ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
  s_second_tick_mode = need_seconds;
  APP_LOG(APP_LOG_LEVEL_INFO, "Tick cadence -> %s",
          need_seconds ? "SECOND" : "MINUTE");
}

static void update_time(struct tm *tick_time) {
  if (s_settings.time_format == TIME_FORMAT_24H) {
    s_hour = tick_time->tm_hour;
  } else {
    s_hour = tick_time->tm_hour % 12;
    if (s_hour == 0) s_hour = 12;
  }
  s_minute = tick_time->tm_min;
  s_second = tick_time->tm_sec;
  layer_mark_dirty(s_clock_layer);

  const WatchTranslation *translation = watch_translation();
  snprintf(s_day_buf, sizeof(s_day_buf), "%s", translation->days[tick_time->tm_wday]);
  strftime(s_date_buf, sizeof(s_date_buf), "%e", tick_time);
  snprintf(s_month_buf, sizeof(s_month_buf), "%s", translation->months[tick_time->tm_mon]);

  update_header_content();
  update_footer_content();

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
static void window_appear(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG,
          "Window appear: light=%d",
          light_is_on() ? 1 : 0);

  if (light_is_on()) {
    cancel_sunlight_fallback();
    apply_bar_visibility();
    update_stepbar_layout();
  } else {
    request_light_with_fallback();
  }
}

static void window_disappear(Window *window) {
  // No physical backlight state is cached. An active sunlight fallback remains
  // bounded and can expire while this window is off-screen.
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);

  s_font_header = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HEADER_31));
  s_font_medium = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
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

  // Do not reset s_weather_buf here. init() has already restored and
  // formatted the persisted weather cache before the window is created.
  // Resetting it to "--" caused cached temperature to disappear until the
  // phone re-sent its cached payload.
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
  load_split_clock_colors();
  load_time_zone_presets();
  weather_cache_load();

  // weather_cache_load() restores the raw Celsius value and availability flag.
  // Rebuild the display string immediately so cached temperature and cached
  // weather icon appear atomically when the watchface becomes visible.
  update_temperature_text();

  // Steps is no longer supported in either center position.
  if (s_settings.top_center_slot == SLOT_STEPS) {
    s_settings.top_center_slot = SLOT_WEATHER;
  }
  if (s_settings.center_slot == CENTER_STEPS) {
    s_settings.center_slot = CENTER_HEART_RATE;
  }

  // Preserve the persisted user preference record before entitlement is known.
  // The effective watchface may temporarily render Free defaults while
  // KiezelPay/trial state is restored, but those defaults must NEVER overwrite
  // the user's saved Pro configuration.
  s_saved_settings = s_settings;
  s_saved_settings_valid = true;

  // A validated KiezelPay purchase is sticky across watchface process restarts.
  // Restore it immediately so paying users never flash/revert to Free merely
  // because Bluetooth/network/KiezelPay status refresh is delayed.
  if (purchased_pro_is_persisted()) {
    s_kiezelpay_licensed = true;
    s_kiezelpay_status_known = true;
    s_pro_unlocked = true;
    s_settings = s_saved_settings;
    APP_LOG(APP_LOG_LEVEL_INFO,
            "Restored Purchased Pro from persistent marker");
  } else {
    s_kiezelpay_licensed = false;
    s_pro_unlocked = false;
    enforce_free_defaults();
  }

  s_window = window_create();
  window_set_background_color(s_window, s_settings.background_color);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .appear = window_appear,
    .disappear = window_disappear,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  s_second_tick_mode = second_ticks_are_needed();
  tick_timer_service_subscribe(
      s_second_tick_mode ? SECOND_UNIT : MINUTE_UNIT, tick_handler);

  // KiezelPay and Big Time both use AppMessage. pebble-events allows both
  // subscribers to coexist without one replacing the other's callbacks.
  kiezelpay_set_event_handler(kiezelpay_event_callback);
  kiezelpay_init();

  s_appmsg_received_handle =
      events_app_message_register_inbox_received(inbox_received_handler, NULL);
  s_appmsg_dropped_handle =
      events_app_message_register_inbox_dropped(inbox_dropped_handler, NULL);
  s_appmsg_handlers_registered = true;

  // Clay sends the complete configuration dictionary on Save. Time Zone adds
  // four selector keys, which pushes the Pro settings payload beyond the old
  // 256-byte inbox. When that dictionary is too large Pebble rejects/drops the
  // entire message, making it look like none of the settings work.
  //
  // Keep the outbox unchanged; only the incoming configuration needs room.
  events_app_message_request_inbox_size(512);
  events_app_message_request_outbox_size(256);
  events_app_message_open();

  // Restore an explicitly-started Big Time trial across watchface restarts.
  // If there is no active trial, wait for KiezelPay's asynchronous status
  // result instead of immediately advertising Free.
  pro_trial_refresh();
  if (s_trial_active) {
    license_send_status_to_phone();
  }
  battery_state_service_subscribe(battery_handler);
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = connection_handler
  });
  update_bar_input_services();
  update_raise_wake_service();
  app_focus_service_subscribe(focus_handler);
#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  if (s_appmsg_handlers_registered) {
    events_app_message_unsubscribe(s_appmsg_received_handle);
    events_app_message_unsubscribe(s_appmsg_dropped_handle);
    s_appmsg_handlers_registered = false;
  }
  kiezelpay_deinit();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  if (s_backlight_subscribed) backlight_service_unsubscribe();
  if (s_touch_subscribed) touch_service_unsubscribe();
  if (s_sunlight_fallback_timer) {
    app_timer_cancel(s_sunlight_fallback_timer);
    s_sunlight_fallback_timer = NULL;
  }
  s_sunlight_fallback_active = false;
  if (s_raise_accel_subscribed) accel_data_service_unsubscribe();
  app_focus_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) { init(); app_event_loop(); deinit(); }