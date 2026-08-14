
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

function clockSection() {
  return {
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
        "defaultValue": false,
        "label": "Center 12 Hour Clock",
        "description": "Centers H:MM when the hour is 1-9. Times 10-12 keep the original spacing."
      }
    ]
  };
}

function proSections() {
  return [
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Appearance" },
        { "type": "color", "messageKey": "ACCENT_COLOR", "defaultValue": "0000AA", "label": "Accent Color" },
        { "type": "color", "messageKey": "CLOCK_COLOR", "defaultValue": "FFFFFF", "label": "Clock Color" },
        { "type": "color", "messageKey": "BACKGROUND_COLOR", "defaultValue": "000000", "label": "Background Color" }
      ]
    },
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Raise to Wake" },
        {
          "type": "select", "messageKey": "RAISE_WAKE", "defaultValue": "0",
          "serializeValueAs": "integer", "label": "Wrist Raise Backlight",
          "description": "Uses wrist orientation and motion to turn on the backlight when you raise the watch to read it.",
          "options": [
            { "label": "Off", "value": "0" },
            { "label": "Normal", "value": "1" },
            { "label": "Sensitive", "value": "2" }
          ]
        }
      ]
    },
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Weather" },
        {
          "type": "select", "messageKey": "TEMP_UNIT", "defaultValue": "0",
          "serializeValueAs": "integer", "label": "Temperature Unit",
          "options": [
            { "label": "Fahrenheit (°F)", "value": "0" },
            { "label": "Celsius (°C)", "value": "1" }
          ]
        }
      ]
    },
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Top Bar" },
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
    },
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Bottom Bar" },
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
    },
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Step Progress Bar" },
        {
          "type": "slider", "messageKey": "STEP_GOAL", "defaultValue": 5000,
          "label": "Daily Step Goal",
          "description": "Sets the goal represented by a full progress bar.",
          "min": 1000, "max": 30000, "step": 500
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
    },
    {
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Restore" },
        {
          "type": "button", "id": "restore_defaults",
          "defaultValue": "Restore Default Settings",
          "description": "Resets customization controls to the original watchface defaults. Tap Save Settings afterward."
        }
      ]
    }
  ];
}

module.exports = function(status) {
  // 0 = Free, 1 = Big Time trial, 2 = KiezelPay licensed Pro.
  var isTrial = status === 1;
  var isPro = status === 2;
  var trialEnded = status === 3;
  var entitled = isTrial || isPro;
  var config = [
    { "type": "heading", "defaultValue": isPro ? "Big Time Pro" : "Big Time" }
  ];

  if (isPro) {
    config.push({
      "type": "text",
      "defaultValue": "Thank you for purchasing Big Time Pro! Your support is greatly appreciated."
    });
  }

  config.push(clockSection());

  if (isTrial) {
    config.push({
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Pro Trial Active" },
        { "type": "text", "defaultValue": "All Big Time Pro customization is unlocked during your 48-hour trial." }
      ]
    });
  }

  if (!entitled) {
    config.push({
      "type": "section",
      "items": [
        { "type": "heading", "defaultValue": "Big Time Pro" },
        {
          "type": "text",
          "defaultValue": "The free version includes the 5,000-step goal plus 12/24-hour and centered 12-hour clock options. Try all Pro customization free for 48 hours or unlock Pro with KiezelPay."
        },
        trialEnded ? {
          "type": "text",
          "defaultValue": "Your Pro trial has ended."
        } : {
          "type": "toggle",
          "messageKey": "TRIAL_START",
          "defaultValue": false,
          "label": "Try Pro Free — 48 Hours",
          "description": "The trial starts only when you enable this and tap Save Settings. It can be used once."
        },
        {
          "type": "toggle",
          "messageKey": "PURCHASE_REQUEST",
          "defaultValue": false,
          "label": "Unlock Pro with KiezelPay",
          "description": "Enable this and tap Save Settings. KiezelPay will restore an existing purchase when recognized; otherwise it will show a purchase code on your watch."
        }
      ]
    });
  }

  if (entitled) {
    var sections = proSections();
    for (var i = 0; i < sections.length; i++) config.push(sections[i]);
  }

  config.push({ "type": "submit", "defaultValue": "Save Settings" });
  return config;
};
