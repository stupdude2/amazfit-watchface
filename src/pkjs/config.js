var edition = require('./edition');

var sideOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Steps", "value": "1" },
  { "label": "Battery (icon + %)", "value": "2" },
  { "label": "Heart Rate", "value": "3" },
  { "label": "Bluetooth", "value": "4" },
  { "label": "Day of Week", "value": "5" },
  { "label": "Date", "value": "6" },
  { "label": "Month", "value": "7" }
];

var centerOptions = [
  { "label": "Heart Rate", "value": "0" },
  { "label": "Battery (icon + %)", "value": "1" },
  { "label": "Bluetooth", "value": "2" },
  { "label": "Weather", "value": "3" },
  { "label": "Steps", "value": "4" },
  { "label": "Day of Week", "value": "5" },
  { "label": "Date", "value": "6" },
  { "label": "Month", "value": "7" }
];

var config = [
  {
    "type": "heading",
    "defaultValue": edition.isPro ? "Amazfit Bip Port Pro" : "Amazfit Bip Port"
  }
];

if (edition.isPro) {
  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Appearance" },
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
        "type": "color",
        "messageKey": "ACCENT_COLOR",
        "defaultValue": "0x0000AA",
        "label": "Accent Color"
      },
      {
        "type": "color",
        "messageKey": "CLOCK_COLOR",
        "defaultValue": "0xFFFFFF",
        "label": "Clock Color"
      },
      {
        "type": "color",
        "messageKey": "BACKGROUND_COLOR",
        "defaultValue": "0x000000",
        "label": "Background Color"
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Weather" },
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
      { "type": "heading", "defaultValue": "Top Bar" },
      { "type": "select", "messageKey": "TOP_LEFT_SLOT", "defaultValue": "5", "serializeValueAs": "integer", "label": "Left Side", "options": sideOptions },
      { "type": "select", "messageKey": "TOP_CENTER_SLOT", "defaultValue": "6", "serializeValueAs": "integer", "label": "Center", "options": sideOptions },
      { "type": "select", "messageKey": "TOP_RIGHT_SLOT", "defaultValue": "7", "serializeValueAs": "integer", "label": "Right Side", "options": sideOptions },
      {
        "type": "select",
        "messageKey": "HEADER_MODE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Top Bar Visibility",
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
      {
        "type": "select",
        "messageKey": "LEFT_SLOT",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Left Side",
        "options": sideOptions
      },
      {
        "type": "select",
        "messageKey": "CENTER_SLOT",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Center",
        "options": centerOptions
      },
      {
        "type": "select",
        "messageKey": "RIGHT_SLOT",
        "defaultValue": "1",
        "serializeValueAs": "integer",
        "label": "Right Side",
        "options": sideOptions
      },
      {
        "type": "select",
        "messageKey": "FOOTER_MODE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Bottom Bar Visibility",
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
        "type": "slider",
        "messageKey": "STEP_GOAL",
        "defaultValue": 5000,
        "label": "Daily Step Goal",
        "description": "Sets the goal represented by a full progress bar.",
        "min": 1000,
        "max": 30000,
        "step": 500
      },
      {
        "type": "select",
        "messageKey": "STEPBAR_MODE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Progress Style",
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
} else {
  config.push({
    "type": "text",
    "defaultValue": "Standard edition. Pro customization options are not enabled in this build."
  });
}

if (edition.isPro) {
  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Restore" },
      {
        "type": "button",
        "id": "restore_defaults",
        "defaultValue": "Restore Default Settings",
        "description": "Resets all customization controls to the original watchface defaults. You will still need to tap Save Settings."
      }
    ]
  });
}

config.push({ "type": "submit", "defaultValue": "Save Settings" });
module.exports = config;
