var edition = require('./edition');

var config = [
  {
    "type": "heading",
    "defaultValue": edition.isPro ? "Amazfit Bip Port Pro — Diagnostic v1.6" : "Amazfit Bip Port"
  }
];

if (edition.isPro) {
  config.push(
    {
      "type": "text",
      "defaultValue": "Diagnostic build: the watch should turn red about 2 seconds after launch, then green about 6 seconds after PebbleKit JS starts. After that, choose a color here and Save Settings."
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
          "defaultValue": "0000AA",
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
