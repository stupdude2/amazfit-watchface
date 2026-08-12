// ── Amazfit Bip Port — PebbleKit JS companion ────────────────────────────────
// Fetches weather from OpenWeatherMap and sends temperature + icon code
// to the watch via AppMessage.
//
// SETUP: Replace YOUR_API_KEY below with a free key from openweathermap.org

// ── Configuration ────────────────────────────────────────────────────────────
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var messageKeys = require('message_keys');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

// Handle Clay explicitly so we can normalize the color and send the generated
// numeric AppMessage key. This avoids depending on string-key translation.
Pebble.addEventListener('showConfiguration', function() {
  console.log('Opening watchface configuration');
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) {
    console.log('Configuration closed without saving');
    return;
  }

  var settings = clay.getSettings(e.response);
  console.log('Clay settings returned: ' + JSON.stringify(settings));

  if (typeof settings.ACCENT_COLOR === 'undefined') {
    console.log('ERROR: ACCENT_COLOR was not returned by Clay');
    return;
  }

  var accent = Number(settings.ACCENT_COLOR);
  if (!isFinite(accent)) {
    console.log('ERROR: ACCENT_COLOR is not numeric: ' + settings.ACCENT_COLOR);
    return;
  }

  // PebbleKit JS accepts numeric AppMessage keys. message_keys is generated
  // from package.json, guaranteeing this matches MESSAGE_KEY_ACCENT_COLOR in C.
  var dict = {};
  dict[messageKeys.ACCENT_COLOR] = accent;

  console.log('Sending ACCENT_COLOR key=' + messageKeys.ACCENT_COLOR +
              ' value=' + accent + ' hex=0x' + accent.toString(16));

  Pebble.sendAppMessage(
    dict,
    function() {
      console.log('ACCENT_COLOR AppMessage ACK from watch');
    },
    function(err) {
      console.log('ACCENT_COLOR AppMessage NACK: ' + JSON.stringify(err));
    }
  );
});

var API_KEY = '18797f22ec59e0b78f4174fef4fb0f2b';
var UNITS   = 'imperial';   // 'imperial' for °F, 'metric' for °C

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
        var temp = Math.round(data.main.temp);
        var icon = iconFromOWM(data.weather[0].id);
        Pebble.sendAppMessage(
          { 'TEMPERATURE': temp, 'WEATHER_ICON': icon },
          function() { console.log('Weather sent: ' + temp + '° icon=' + icon); },
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