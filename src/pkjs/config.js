var edition = require('./edition');

var sideOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Steps", "value": "1" },
  { "label": "Battery (icon + %)", "value": "2" },
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

var topCenterOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Battery (icon + %)", "value": "2" },
  { "label": "Heart Rate", "value": "3" },
  { "label": "Bluetooth", "value": "4" },
  { "label": "Date", "value": "6" }
];

var centerOptions = [
  { "label": "Heart Rate", "value": "0" },
  { "label": "Battery (icon + %)", "value": "1" },
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
      { "type": "color", "messageKey": "CLOCK_COLOR", "defaultValue": "0xFFFFFF", "label": "Clock Color", "layout": "COLOR" },
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
      { "type": "toggle", "messageKey": "TOP_LEFT_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "TOP_CENTER_SLOT", "defaultValue": "6", "serializeValueAs": "integer", "label": "Center", "options": topCenterOptions },
      { "type": "toggle", "messageKey": "TOP_CENTER_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "TOP_RIGHT_SLOT", "defaultValue": "7", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
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
      { "type": "toggle", "messageKey": "LEFT_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "CENTER_SLOT", "defaultValue": "0", "serializeValueAs": "integer", "label": "Center", "options": centerOptions },
      { "type": "toggle", "messageKey": "CENTER_HIDE_LABEL", "defaultValue": false, "label": "Hide Label", "description": "Enlarge this value by hiding its label." },
      { "type": "select", "messageKey": "RIGHT_SLOT", "defaultValue": "1", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
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
