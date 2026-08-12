var edition = require('./edition');

var sideOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Steps", "value": "1" },
  { "label": "Battery (icon + %)", "value": "2" },
  { "label": "Heart Rate", "value": "3" },
  { "label": "Bluetooth", "value": "4" }
];

var centerOptions = [
  { "label": "Heart Rate", "value": "0" },
  { "label": "Battery (icon + %)", "value": "1" },
  { "label": "Bluetooth", "value": "2" }
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
        "type": "color",
        "messageKey": "ACCENT_COLOR",
        "defaultValue": "0x0000AA",
        "label": "Accent Color"
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Top Bar" },
      {
        "type": "select",
        "messageKey": "HEADER_MODE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Top Bar Visibility",
        "options": [
          { "label": "Always Visible", "value": "0" },
          { "label": "Show With Backlight", "value": "1" }
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
          { "label": "Show With Backlight", "value": "1" }
        ]
      }
    ]
  });

  config.push({
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Step Progress Bar" },
      {
        "type": "select",
        "messageKey": "STEPBAR_MODE",
        "defaultValue": "0",
        "serializeValueAs": "integer",
        "label": "Progress Style",
        "options": [
          { "label": "Mirrored Below Time", "value": "0" },
          { "label": "Left to Right Below Time", "value": "1" },
          { "label": "Hidden", "value": "2" },
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

config.push({ "type": "submit", "defaultValue": "Save Settings" });
module.exports = config;
