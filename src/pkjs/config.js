var edition = require('./edition');

var config = [
  {
    "type": "heading",
    "defaultValue": edition.isPro ? "Amazfit Bip Port Pro" : "Amazfit Bip Port"
  }
];

if (edition.isPro) {
  config.push(
    {
      "type": "text",
      "defaultValue": "Customize your Pebble Time 2 watchface. More Pro options will be added as they are tested."
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Appearance"
        },
        {
          "type": "color",
          "messageKey": "ACCENT_COLOR",
          "defaultValue": "0x0000AA",
          "label": "Accent Color"
        }
      ]
    }
  );
} else {
  config.push({
    "type": "text",
    "defaultValue": "Standard edition. Pro customization options are not enabled in this build."
  });
}

config.push({
  "type": "submit",
  "defaultValue": "Save Settings"
});

module.exports = config;
