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
  { "label": "Steps", "value": "1" },
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
  { "label": "Steps", "value": "4" },
  { "label": "Date", "value": "6" }
];

var config = [
  {
    "type": "heading",
    "defaultValue": edition.isPro ? "Amazfit Bip Port Pro" : "Amazfit Bip Port"
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
  }
];

if (!edition.isPro) {
  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Pro Customization" },
      {
        "type": "text",
        "defaultValue": "Unlock Pro with KiezelPay for colors, bar layouts, weather units, step-goal customization, raise-to-wake, and all advanced options."
      }
    ]
  });
}

if (edition.isPro) {
  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Appearance — Pro" },
      { "type": "color", "messageKey": "ACCENT_COLOR", "defaultValue": "0x0000AA", "label": "Accent Color" },
      { "type": "color", "messageKey": "CLOCK_COLOR", "defaultValue": "0xFFFFFF", "label": "Clock Color" },
      { "type": "color", "messageKey": "BACKGROUND_COLOR", "defaultValue": "0x000000", "label": "Background Color" }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Raise to Wake — Pro" },
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
      { "type": "heading", "defaultValue": "Weather — Pro" },
      {
        "type": "select",
        "messageKey": "TEMP_UNIT",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Temperature Unit",
        "options": [
          { "label": "Fahrenheit (°F)", "value": "0" },
          { "label": "Celsius (°C)", "value": "1" }
        ]
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Top Bar — Pro" },
      { "type": "select", "messageKey": "TOP_LEFT_SLOT", "defaultValue": "5", "serializeValueAs": "integer", "label": "Left Side", "options": sideOptions },
      { "type": "select", "messageKey": "TOP_CENTER_SLOT", "defaultValue": "6", "serializeValueAs": "integer", "label": "Center", "options": topCenterOptions },
      { "type": "select", "messageKey": "TOP_RIGHT_SLOT", "defaultValue": "7", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
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
      { "type": "heading", "defaultValue": "Bottom Bar — Pro" },
      { "type": "select", "messageKey": "LEFT_SLOT", "defaultValue": "0", "serializeValueAs": "integer", "label": "Left Side", "options": sideOptions },
      { "type": "select", "messageKey": "CENTER_SLOT", "defaultValue": "0", "serializeValueAs": "integer", "label": "Center", "options": centerOptions },
      { "type": "select", "messageKey": "RIGHT_SLOT", "defaultValue": "1", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
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
      { "type": "heading", "defaultValue": "Step Progress Bar — Pro" },
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
