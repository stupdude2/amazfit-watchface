var edition = require('./edition');

var sideOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Steps", "value": "1" },
  { "label": "Battery (icon + %)", "value": "2" },
  { "label": "Battery Icon", "value": "13" },
  { "label": "Battery %", "value": "14" },
  { "label": "Time Zone", "value": "15" },
  { "label": "Seconds", "value": "16" },
  { "label": "Tomorrow", "value": "17" },
  { "label": "Rain Chance", "value": "18" },
  { "label": "+2 Hours", "value": "19" },
  { "label": "Heart Rate", "value": "3" },
  { "label": "Bluetooth", "value": "4" },
  { "label": "Day of Week", "value": "5" },
  { "label": "Date", "value": "6" },
  { "label": "Month", "value": "7" },
  { "label": "Calories", "value": "8" },
  { "label": "Distance", "value": "9" },
  { "label": "Sunrise", "value": "10" },
  { "label": "Sunset", "value": "11" },
  { "label": "High / Low Temp", "value": "12" }
];

var timeZoneOptions = [
  { "label": "Local Time", "value": "0" },
  { "label": "UTC-12:00", "value": "1" },
  { "label": "UTC-11:00", "value": "2" },
  { "label": "UTC-10:00", "value": "3" },
  { "label": "UTC-09:00", "value": "4" },
  { "label": "UTC-08:00", "value": "5" },
  { "label": "UTC-07:00", "value": "6" },
  { "label": "UTC-06:00", "value": "7" },
  { "label": "UTC-05:00", "value": "8" },
  { "label": "UTC-04:00", "value": "9" },
  { "label": "UTC-03:00", "value": "10" },
  { "label": "UTC-02:00", "value": "11" },
  { "label": "UTC-01:00", "value": "12" },
  { "label": "UTC", "value": "13" },
  { "label": "UTC+01:00", "value": "14" },
  { "label": "UTC+02:00", "value": "15" },
  { "label": "UTC+03:00", "value": "16" },
  { "label": "UTC+03:30", "value": "17" },
  { "label": "UTC+04:00", "value": "18" },
  { "label": "UTC+04:30", "value": "19" },
  { "label": "UTC+05:00", "value": "20" },
  { "label": "UTC+05:30", "value": "21" },
  { "label": "UTC+05:45", "value": "22" },
  { "label": "UTC+06:00", "value": "23" },
  { "label": "UTC+06:30", "value": "24" },
  { "label": "UTC+07:00", "value": "25" },
  { "label": "UTC+08:00", "value": "26" },
  { "label": "UTC+08:45", "value": "27" },
  { "label": "UTC+09:00", "value": "28" },
  { "label": "UTC+09:30", "value": "29" },
  { "label": "UTC+10:00", "value": "30" },
  { "label": "UTC+10:30", "value": "31" },
  { "label": "UTC+11:00", "value": "32" },
  { "label": "UTC+12:00", "value": "33" },
  { "label": "UTC+12:45", "value": "34" },
  { "label": "UTC+13:00", "value": "35" },
  { "label": "UTC+14:00", "value": "36" }
];

var topCenterOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Battery (icon + %)", "value": "2" },
  { "label": "Battery Icon", "value": "13" },
  { "label": "Battery %", "value": "14" },
  { "label": "Seconds", "value": "16" },
  { "label": "Rain Chance", "value": "18" },
  { "label": "Heart Rate", "value": "3" },
  { "label": "Bluetooth", "value": "4" },
  { "label": "Date", "value": "6" }
];

var centerOptions = [
  { "label": "Heart Rate", "value": "0" },
  { "label": "Battery (icon + %)", "value": "1" },
  { "label": "Battery Icon", "value": "8" },
  { "label": "Battery %", "value": "9" },
  { "label": "Seconds", "value": "10" },
  { "label": "Rain Chance", "value": "11" },
  { "label": "Bluetooth", "value": "2" },
  { "label": "Weather", "value": "3" },
  { "label": "Date", "value": "6" }
];

function formatTrialRemaining(totalSeconds) {
  var seconds = Math.max(0, Number(totalSeconds) || 0);
  var hours = Math.floor(seconds / 3600);
  var minutes = Math.floor((seconds % 3600) / 60);

  if (hours > 0) {
    return hours + " hr " + minutes + " min remaining";
  }
  if (minutes > 0) {
    return minutes + " min remaining";
  }
  return "Less than 1 min remaining";
}

var statusText =
  edition.isPurchased
    ? "Pro — Thank you for purchasing Big Time Pro!"
    : edition.isTrial
      ? "Free Trial — Your Big Time Pro trial is currently active. " +
        formatTrialRemaining(edition.trialRemaining) + "."
      : "Free";

var config = [
  {
    "type": "heading",
    "defaultValue": "Big Time"
  },
  {
    "type": "text",
    "defaultValue": statusText
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Language" },
      // LANGUAGE MAINTENANCE:
      // When adding a language, also follow the REQUIRED CHECKLIST beside
      // WatchLanguage in main.c and add it to SUPPORTED_LANGUAGES in index.js.
      {
        "type": "select",
        "messageKey": "LANGUAGE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Watchface Language",
        "description": "Changes text displayed on the watchface.",
        "options": [
          { "label": "English", "value": "0" },
          { "label": "Svenska", "value": "1" },
          { "label": "Español", "value": "2" },
          { "label": "Français", "value": "3" },
          { "label": "Deutsch", "value": "4" },
          { "label": "Português", "value": "5" },
          { "label": "Català", "value": "6" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Clock" },
      {
        "type": "select",
        "messageKey": "TIME_FORMAT",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Time Format",
        "options": [
          { "label": "12 Hour", "value": "0" },
          { "label": "24 Hour", "value": "1" }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "CENTER_12H",
        "defaultValue": true,
        "label": "Center 12 Hour Clock",
        "description": "Centers H:MM when the hour is 1-9. Available in the free version."
      }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Weather" },
      {
        "type": "select",
        "messageKey": "TEMP_UNIT",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Temperature Unit",
        "description": "Available in the free version.",
        "options": [
          { "label": "Fahrenheit (°F)", "value": "0" },
          { "label": "Celsius (°C)", "value": "1" }
        ]
      },
      {
        "type": "select",
        "messageKey": "WEATHER_REFRESH",
        "defaultValue": "60",
        "serializeValueAs": "integer",
        "label": "Weather Refresh Interval",
        "description": "Cached weather remains visible between refreshes.",
        "options": [
          { "label": "15 Minutes", "value": "15" },
          { "label": "30 Minutes", "value": "30" },
          { "label": "1 Hour", "value": "60" },
          { "label": "2 Hours", "value": "120" },
          { "label": "3 Hours", "value": "180" },
          { "label": "6 Hours", "value": "360" }
        ]
      }
    ]
  }
];

if (!edition.isPro) {
  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Upgrade to Pro" },
      {
        "type": "text",
        "defaultValue": edition.trialUsed
          ? "Your 48-hour Pro trial has ended."
          : "48-hour Pro trial available. The trial does not start automatically."
      }
    ].concat(edition.trialUsed ? [] : [
      {
        "type": "toggle",
        "messageKey": "TRY_PRO_FREE",
        "defaultValue": false,
        "label": "Try Pro Free",
        "description": "Enable and tap Save Settings to start your one-time 48-hour Pro trial."
      }
    ]).concat([
      {
        "type": "toggle",
        "messageKey": "UNLOCK_PRO",
        "defaultValue": false,
        "label": "Unlock Pro with KiezelPay",
        "description": "Enable and tap Save Settings. Your purchase code will appear on the watch. This also restores an existing purchase."
      },
      {
        "type": "text",
        "defaultValue": "Pro unlocks colors, layouts, custom step goals, advanced data placement, Raise to Wake, and all advanced customization."
      }
    ])
  });
}

if (edition.isPro) {
  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Appearance" },
      { "type": "color", "messageKey": "ACCENT_COLOR", "defaultValue": "0x0000AA", "label": "Accent Color", "layout": "COLOR" },
      { "type": "color", "messageKey": "CLOCK_COLOR", "defaultValue": "0xFFFFFF", "label": "Clock Color / Colon Color", "layout": "COLOR",
        "description": "Controls the entire clock unless separate hour/minute colors are enabled. When enabled, this color controls the colon." },
      { "type": "toggle", "messageKey": "SPLIT_CLOCK_COLORS", "defaultValue": false,
        "label": "Separate Hour / Minute Colors",
        "description": "Use independent colors for the hour and minute digits." },
      { "type": "color", "messageKey": "HOUR_COLOR", "defaultValue": "0xFFFFFF", "label": "Hour Color", "layout": "COLOR" },
      { "type": "color", "messageKey": "MINUTE_COLOR", "defaultValue": "0xFFFFFF", "label": "Minute Color", "layout": "COLOR" },
      { "type": "toggle", "messageKey": "FLASH_COLON", "defaultValue": false,
        "label": "Seconds Colon",
        "description": "Flash the main clock colon on and off once per second to indicate seconds." },
      { "type": "select", "messageKey": "ROUNDED_TIME", "defaultValue": "0",
        "label": "Time Style",
        "description": "Choose the shape of the main clock digits.",
        "options": [
          { "label": "Square", "value": "0" },
          { "label": "Rounded", "value": "1" },
          { "label": "Slightly Rounded", "value": "2" }
        ] },
      { "type": "color", "messageKey": "BACKGROUND_COLOR", "defaultValue": "0x000000", "label": "Background Color", "layout": "COLOR" }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Raise to Wake" },
      {
        "type": "select",
        "messageKey": "RAISE_WAKE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Wrist Raise Backlight",
        "options": [
          { "label": "Off", "value": "0" },
          { "label": "Normal", "value": "1" },
          { "label": "Sensitive", "value": "2" }
        ]
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Top Bar" },
      { "type": "select", "messageKey": "TOP_LEFT_SLOT", "defaultValue": "5", "serializeValueAs": "integer", "label": "Left Side", "options": sideOptions },
      { "type": "select", "messageKey": "TOP_LEFT_TIME_ZONE", "defaultValue": "0", "serializeValueAs": "integer", "label": "Top Left Time Zone", "options": timeZoneOptions },
      { "type": "toggle", "messageKey": "TOP_LEFT_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "TOP_CENTER_SLOT", "defaultValue": "6", "serializeValueAs": "integer", "label": "Center", "options": topCenterOptions },
      { "type": "toggle", "messageKey": "TOP_CENTER_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "TOP_RIGHT_SLOT", "defaultValue": "7", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
      { "type": "select", "messageKey": "TOP_RIGHT_TIME_ZONE", "defaultValue": "0", "serializeValueAs": "integer", "label": "Top Right Time Zone", "options": timeZoneOptions },
      { "type": "toggle", "messageKey": "TOP_RIGHT_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      {
        "type": "select", "messageKey": "HEADER_MODE", "defaultValue": "0",
        "serializeValueAs": "integer", "label": "Top Bar Visibility",
        "options": [
          { "label": "Always Visible", "value": "0" },
          { "label": "Show With Backlight", "value": "1" },
          { "label": "Always Hidden", "value": "2" }
        ]
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Bottom Bar" },
      { "type": "select", "messageKey": "LEFT_SLOT", "defaultValue": "0", "serializeValueAs": "integer", "label": "Left Side", "options": sideOptions },
      { "type": "select", "messageKey": "LEFT_TIME_ZONE", "defaultValue": "0", "serializeValueAs": "integer", "label": "Bottom Left Time Zone", "options": timeZoneOptions },
      { "type": "toggle", "messageKey": "LEFT_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "CENTER_SLOT", "defaultValue": "0", "serializeValueAs": "integer", "label": "Center", "options": centerOptions },
      { "type": "toggle", "messageKey": "CENTER_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "RIGHT_SLOT", "defaultValue": "1", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
      { "type": "select", "messageKey": "RIGHT_TIME_ZONE", "defaultValue": "0", "serializeValueAs": "integer", "label": "Bottom Right Time Zone", "options": timeZoneOptions },
      { "type": "toggle", "messageKey": "RIGHT_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      {
        "type": "select", "messageKey": "FOOTER_MODE", "defaultValue": "0",
        "serializeValueAs": "integer", "label": "Bottom Bar Visibility",
        "options": [
          { "label": "Always Visible", "value": "0" },
          { "label": "Show With Backlight", "value": "1" },
          { "label": "Always Hidden", "value": "2" }
        ]
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Step Progress Bar" },
      {
        "type": "toggle", "messageKey": "PROGRESS_TRACK_BATTERY",
        "defaultValue": false,
        "label": "Track Battery Instead of Steps",
        "description": "Use the progress bar to show remaining battery life from full to empty."
      },
      {
        "type": "slider", "messageKey": "STEP_GOAL", "defaultValue": 5000,
        "label": "Daily Step Goal", "min": 1000, "max": 30000, "step": 500
      },
      {
        "type": "select", "messageKey": "STEPBAR_MODE", "defaultValue": "0",
        "serializeValueAs": "integer", "label": "Progress Style",
        "options": [
          { "label": "Mirrored Below Time", "value": "0" },
          { "label": "Left to Right Below Time", "value": "1" },
          { "label": "Always Hidden", "value": "2" },
          { "label": "Mirrored Above Time", "value": "3" },
          { "label": "Left to Right Above Time", "value": "4" },
          { "label": "Mirrored Below Time — With Backlight", "value": "5" },
          { "label": "Left to Right Below Time — With Backlight", "value": "6" },
          { "label": "Mirrored Above Time — With Backlight", "value": "7" },
          { "label": "Left to Right Above Time — With Backlight", "value": "8" }
        ]
      }
    ]
  });

  if (edition.isTrial) {
    config.push({
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "KiezelPay" },
        {
          "type": "toggle",
          "messageKey": "UNLOCK_PRO",
          "defaultValue": false,
          "label": "Purchase / Restore Pro",
          "description": "Enable and tap Save Settings to purchase Pro now or restore an existing purchase."
        }
      ]
    });
  }

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Restore" },
      {
        "type": "button",
        "id": "restore_defaults",
        "defaultValue": "Restore Default Settings"
      }
    ]
  });
}

config.push({ "type": "submit", "defaultValue": "Save Settings" });
module.exports = config;
