var edition = require('./edition');

var sideOptions = [
  { "label": "Weather", "value": "0" },
  { "label": "Steps", "value": "1" },
  { "label": "Battery", "value": "2" },
  { "label": "Heart Rate", "value": "3" },
  { "label": "Bluetooth Connected", "value": "4" }
];

var centerOptions = [
  { "label": "Heart Rate", "value": "0" },
  { "label": "Battery", "value": "1" },
  { "label": "Bluetooth Connected", "value": "2" }
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
          { "label": "Show 5 Seconds on Double Tap", "value": "1" },
          { "label": "Off", "value": "2" }
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
          { "label": "Mirrored from Center", "value": "0" },
          { "label": "Left to Right", "value": "1" },
          { "label": "Hidden", "value": "2" }
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
