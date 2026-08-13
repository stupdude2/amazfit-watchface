// ── Amazfit Bip Port — PebbleKit JS companion ────────────────────────────────
// Fetches weather from OpenWeatherMap and sends temperature + icon code
// to the watch via AppMessage.
//
// SETUP: Replace YOUR_API_KEY below with a free key from openweathermap.org

// ── Configuration ────────────────────────────────────────────────────────────
// Clay automatically opens the settings page and sends configured messageKey
// values to the watch when Save Settings is pressed.
var Clay = require('@rebble/clay');
var clayConfig = require('./config');

// Injected into Clay's generated settings page. Clay's custom-function API
// exposes each config item through getItemByMessageKey()/getItemById(), and
// each item supports set()/get(). This lets us provide a real Restore Defaults
// button without adding another AppMessage key.
function customClay(minified) {
  var clayPage = this;

  function setValue(messageKey, value) {
    var item = clayPage.getItemByMessageKey(messageKey);
    if (item && typeof item.set === 'function') {
      item.set(value);
    }
  }

  clayPage.on(clayPage.EVENTS.AFTER_BUILD, function() {
    var resetButton = clayPage.getItemById('restore_defaults');
    if (!resetButton) return;

    resetButton.on('click', function() {
      var confirmed = true;
      if (typeof window !== 'undefined' && typeof window.confirm === 'function') {
        confirmed = window.confirm(
          'Restore all watchface settings to their defaults?\\n\\n' +
          'This will reset colors, weather units, top and bottom bar content, ' +
          'visibility, step goal, and step-bar layout.'
        );
      }
      if (!confirmed) return;

      setValue('ACCENT_COLOR', '0x0000AA');
      setValue('CLOCK_COLOR', '0xFFFFFF');
      setValue('BACKGROUND_COLOR', '0x000000');
      setValue('TEMP_UNIT', '0');

      setValue('TOP_LEFT_SLOT', '5');
      setValue('TOP_CENTER_SLOT', '6');
      setValue('TOP_RIGHT_SLOT', '7');
      setValue('HEADER_MODE', '0');

      setValue('LEFT_SLOT', '0');
      setValue('CENTER_SLOT', '0');
      setValue('RIGHT_SLOT', '1');
      setValue('FOOTER_MODE', '0');

      setValue('STEP_GOAL', 5000);
      setValue('STEPBAR_MODE', '0');

      if (typeof window !== 'undefined' && typeof window.alert === 'function') {
        window.alert('Defaults restored. Tap Save Settings to apply them to the watch.');
      }
    });
  });
}

var clay = new Clay(clayConfig, customClay);
var messageKeys = require('message_keys');

var API_KEY = '18797f22ec59e0b78f4174fef4fb0f2b';
var UNITS   = 'metric';     // Always fetch Celsius; watch converts to the user's unit

// Icon codes sent to watch (must match KEY_WEATHER_ICON cases in main.c)
var ICON_CLEAR   = 0;
var ICON_CLOUDY  = 1;
var ICON_RAIN    = 2;
var ICON_SNOW    = 3;
var ICON_THUNDER = 4;

function iconFromOWM(id) {
  if (id >= 200 && id < 300) return ICON_THUNDER;
  if (id >= 300 && id < 600) return ICON_RAIN;
  if (id >= 600 && id < 700) return ICON_SNOW;
  if (id >= 800 && id < 802) return ICON_CLEAR;
  return ICON_CLOUDY;
}

function fetchWeather(lat, lon) {
  var url = 'https://api.openweathermap.org/data/2.5/weather'
    + '?lat=' + lat
    + '&lon=' + lon
    + '&units=' + UNITS
    + '&appid=' + API_KEY;

  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        var data = JSON.parse(xhr.responseText);
        // Send Celsius in tenths of a degree. This preserves enough precision
        // for the watch to switch between °F and °C locally.
        var temp = Math.round(data.main.temp * 10);
        var icon = iconFromOWM(data.weather[0].id);
        Pebble.sendAppMessage(
          { 'TEMPERATURE': temp, 'WEATHER_ICON': icon },
          function() { console.log('Weather sent: ' + (temp / 10) + '°C icon=' + icon); },
          function(e) { console.log('Weather send failed: ' + JSON.stringify(e)); }
        );
      } catch(e) {
        console.log('Weather parse error: ' + e);
      }
    } else {
      console.log('Weather HTTP error: ' + xhr.status);
    }
  };
  xhr.open('GET', url);
  xhr.send();
}

// Messages from the watch: configuration acknowledgement or weather request.
Pebble.addEventListener('appmessage', function(e) {
  if (e.payload && typeof e.payload.CONFIG_ACK !== 'undefined') {
    console.log('WATCH APPLIED ACCENT_COLOR: ' + e.payload.CONFIG_ACK);
    return;
  }

  console.log('Watch requested weather update');
  navigator.geolocation.getCurrentPosition(
    function(pos) {
      fetchWeather(pos.coords.latitude, pos.coords.longitude);
    },
    function(err) {
      console.log('Geolocation error: ' + err.message);
    },
    { timeout: 15000, maximumAge: 60000 }
  );
});

// Fetch on launch
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready');

  navigator.geolocation.getCurrentPosition(
    function(pos) {
      fetchWeather(pos.coords.latitude, pos.coords.longitude);
    },
    function(err) {
      console.log('Geolocation error on ready: ' + err.message);
    },
    { timeout: 15000, maximumAge: 300000 }
  );
});