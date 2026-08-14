// Big Time — PebbleKit JS companion
// KiezelPay paid licensing follows the official KiezelPay Pebble integration.
// Big Time's 48-hour Pro trial is opt-in and managed separately.

var KIEZELPAY_LOGGING = true; // TEST BUILD: set false before release.
var KiezelPay = require('kiezelpay-core');
var kiezelpay = new KiezelPay(KIEZELPAY_LOGGING);

var Clay = require('@rebble/clay');
var buildConfig = require('./config');
var messageKeys = require('message_keys');

var currentClay = null;
var latestEntitlementStatus = 0;
var pendingStatusOpen = false;
var statusOpenTimer = null;

function customClay(minified) {
  var clayPage = this;

  function setValue(messageKey, value) {
    var item = clayPage.getItemByMessageKey(messageKey);
    if (item && typeof item.set === 'function') {
      item.set(value);
    }
  }

  clayPage.on(clayPage.EVENTS.AFTER_BUILD, function() {
    var timeFormat = clayPage.getItemByMessageKey('TIME_FORMAT');
    var center12h = clayPage.getItemByMessageKey('CENTER_12H');

    function syncCenter12hAvailability() {
      if (!timeFormat || !center12h) return;
      if (String(timeFormat.get()) === '1') {
        center12h.disable();
      } else {
        center12h.enable();
      }
    }

    if (timeFormat && center12h) {
      syncCenter12hAvailability();
      timeFormat.on('change', syncCenter12hAvailability);
    }

    var resetButton = clayPage.getItemById('restore_defaults');
    if (!resetButton) return;

    resetButton.on('click', function() {
      // Avoid window.confirm(): Pebble's embedded webview labels native dialogs
      // "JavaScript" and some versions display escaped newline characters.
      // Use an in-page confirmation card instead so we control the title/text.
      if (typeof document === 'undefined') return;

      var existing = document.getElementById('restore-defaults-confirmation');
      if (existing) {
        existing.parentNode.removeChild(existing);
      }

      var overlay = document.createElement('div');
      overlay.id = 'restore-defaults-confirmation';
      overlay.style.position = 'fixed';
      overlay.style.left = '0';
      overlay.style.top = '0';
      overlay.style.right = '0';
      overlay.style.bottom = '0';
      overlay.style.zIndex = '99999';
      overlay.style.background = 'rgba(0,0,0,0.55)';
      overlay.style.display = 'flex';
      overlay.style.alignItems = 'center';
      overlay.style.justifyContent = 'center';
      overlay.style.padding = '20px';
      overlay.style.boxSizing = 'border-box';

      var card = document.createElement('div');
      card.style.width = '100%';
      card.style.maxWidth = '420px';
      card.style.background = '#ffffff';
      card.style.color = '#222222';
      card.style.borderRadius = '10px';
      card.style.padding = '20px';
      card.style.boxSizing = 'border-box';
      card.style.boxShadow = '0 4px 18px rgba(0,0,0,0.35)';

      var title = document.createElement('div');
      title.textContent = 'Restore Default Settings?';
      title.style.fontSize = '20px';
      title.style.fontWeight = 'bold';
      title.style.marginBottom = '12px';

      var body = document.createElement('div');
      body.textContent =
        'This will reset colors, weather units, top and bottom bar content, ' +
        'visibility, step goal, and step-bar layout.';
      body.style.fontSize = '15px';
      body.style.lineHeight = '1.4';
      body.style.marginBottom = '18px';

      var buttons = document.createElement('div');
      buttons.style.display = 'flex';
      buttons.style.justifyContent = 'center';
      buttons.style.alignItems = 'center';
      buttons.style.width = '100%';
      buttons.style.boxSizing = 'border-box';

      var cancel = document.createElement('button');
      cancel.type = 'button';
      cancel.textContent = 'Cancel';
      cancel.style.width = '92px';
      cancel.style.minWidth = '92px';
      cancel.style.maxWidth = '92px';
      cancel.style.marginRight = '10px';
      cancel.style.padding = '10px 8px';
      cancel.style.boxSizing = 'border-box';
      cancel.style.border = '1px solid #aaaaaa';
      cancel.style.borderRadius = '6px';
      cancel.style.background = '#f4f4f4';
      cancel.style.color = '#222222';

      var restore = document.createElement('button');
      restore.type = 'button';
      restore.textContent = 'Yes';
      restore.style.width = '92px';
      restore.style.minWidth = '92px';
      restore.style.maxWidth = '92px';
      restore.style.padding = '10px 8px';
      restore.style.boxSizing = 'border-box';
      restore.style.border = '0';
      restore.style.borderRadius = '6px';
      restore.style.background = '#d9534f';
      restore.style.color = '#ffffff';

      function closeOverlay() {
        if (overlay && overlay.parentNode) {
          overlay.parentNode.removeChild(overlay);
        }
      }

      cancel.addEventListener('click', closeOverlay);

      restore.addEventListener('click', function() {
        setValue('ACCENT_COLOR', '0000AA');
        setValue('CLOCK_COLOR', 'FFFFFF');
        setValue('TIME_FORMAT', '0');
        setValue('CENTER_12H', false);
        setValue('RAISE_WAKE', '0');
        setValue('BACKGROUND_COLOR', '000000');
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

        closeOverlay();

        // Show a lightweight in-page status rather than another native JS alert.
        var status = document.createElement('div');
        status.textContent = 'Defaults restored. Tap Save Settings to apply them to the watch.';
        status.style.position = 'fixed';
        status.style.left = '16px';
        status.style.right = '16px';
        status.style.bottom = '16px';
        status.style.zIndex = '99999';
        status.style.background = '#333333';
        status.style.color = '#ffffff';
        status.style.padding = '12px 14px';
        status.style.borderRadius = '6px';
        status.style.textAlign = 'center';
        document.body.appendChild(status);

        setTimeout(function() {
          if (status.parentNode) status.parentNode.removeChild(status);
        }, 3000);
      });

      buttons.appendChild(cancel);
      buttons.appendChild(restore);
      card.appendChild(title);
      card.appendChild(body);
      card.appendChild(buttons);
      overlay.appendChild(card);
      document.body.appendChild(overlay);
    });
  });
}


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

function localMinuteOfDay(unixSeconds, timezoneSeconds) {
  var seconds = (unixSeconds + timezoneSeconds) % 86400;
  if (seconds < 0) seconds += 86400;
  return Math.floor(seconds / 60);
}

function sendWeatherPayload(payload) {
  Pebble.sendAppMessage(
    payload,
    function() { console.log('Weather data sent'); },
    function(e) { console.log('Weather send failed: ' + JSON.stringify(e)); }
  );
}

function fetchWeather(lat, lon) {
  var currentUrl = 'https://api.openweathermap.org/data/2.5/weather'
    + '?lat=' + lat
    + '&lon=' + lon
    + '&units=' + UNITS
    + '&appid=' + API_KEY;

  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status !== 200) {
      console.log('Weather HTTP error: ' + xhr.status);
      return;
    }

    try {
      var data = JSON.parse(xhr.responseText);
      var temp = Math.round(data.main.temp * 10);
      var icon = iconFromOWM(data.weather[0].id);
      var timezone = data.timezone || 0;

      var payload = {
        'TEMPERATURE': temp,
        'WEATHER_ICON': icon,
        'SUNRISE': localMinuteOfDay(data.sys.sunrise, timezone),
        'SUNSET': localMinuteOfDay(data.sys.sunset, timezone)
      };

      var forecastUrl = 'https://api.openweathermap.org/data/2.5/forecast'
        + '?lat=' + lat
        + '&lon=' + lon
        + '&units=' + UNITS
        + '&appid=' + API_KEY;

      var forecastXhr = new XMLHttpRequest();
      forecastXhr.onload = function() {
        if (forecastXhr.status === 200) {
          try {
            var forecast = JSON.parse(forecastXhr.responseText);
            var forecastTimezone =
              forecast.city && typeof forecast.city.timezone === 'number'
                ? forecast.city.timezone : timezone;
            var nowUtc = Math.floor(Date.now() / 1000);
            var localDay = Math.floor((nowUtc + forecastTimezone) / 86400);
            var high = data.main.temp;
            var low = data.main.temp;

            for (var i = 0; i < forecast.list.length; i++) {
              var item = forecast.list[i];
              var itemDay = Math.floor((item.dt + forecastTimezone) / 86400);
              if (itemDay !== localDay) continue;

              if (item.main.temp_max > high) high = item.main.temp_max;
              if (item.main.temp_min < low) low = item.main.temp_min;
            }

            payload.HIGH_TEMP = Math.round(high * 10);
            payload.LOW_TEMP = Math.round(low * 10);
          } catch (e) {
            console.log('Forecast parse error: ' + e);
          }
        } else {
          console.log('Forecast HTTP error: ' + forecastXhr.status);
        }

        sendWeatherPayload(payload);
      };

      forecastXhr.onerror = function() {
        console.log('Forecast network error');
        sendWeatherPayload(payload);
      };

      forecastXhr.open('GET', forecastUrl);
      forecastXhr.send();
    } catch (e) {
      console.log('Weather parse error: ' + e);
    }
  };

  xhr.open('GET', currentUrl);
  xhr.send();
}


var TRIAL_STORAGE_KEY = 'bigTimeTrialStartEpoch';

function sendTrialSyncIfNeeded() {
  var stored = parseInt(localStorage.getItem(TRIAL_STORAGE_KEY) || '0', 10);
  if (!stored) return;

  var payload = {};
  payload[messageKeys.TRIAL_SYNC_EPOCH] = stored;
  Pebble.sendAppMessage(payload,
    function() { console.log('Trial marker synced to watch'); },
    function(e) { console.log('Trial sync failed: ' + JSON.stringify(e)); });
}

function openSettingsWithStatus(status) {
  latestEntitlementStatus = status;
  pendingStatusOpen = false;

  if (statusOpenTimer) {
    clearTimeout(statusOpenTimer);
    statusOpenTimer = null;
  }

  currentClay = new Clay(buildConfig(status), customClay, { autoHandleEvents: false });
  Pebble.openURL(currentClay.generateUrl());
}

function requestEntitlementAndOpenSettings() {
  pendingStatusOpen = true;

  var payload = {};
  payload[messageKeys.LICENSE_STATUS_REQUEST] = 1;

  Pebble.sendAppMessage(payload,
    function() {
      console.log('Requested current entitlement from watch');
    },
    function(e) {
      console.log('Entitlement request failed: ' + JSON.stringify(e));
    });

  // If the watch cannot answer, fail closed to the Free UI rather than ever
  // exposing Pro controls from stale phone-side state.
  statusOpenTimer = setTimeout(function() {
    if (pendingStatusOpen) {
      console.log('Entitlement response timed out; opening Free settings');
      openSettingsWithStatus(0);
    }
  }, 1500);
}

Pebble.addEventListener('showConfiguration', function() {
  requestEntitlementAndOpenSettings();
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || !currentClay) return;

  var dict = currentClay.getSettings(e.response);

  // Record the original trial start on the phone before sending it to the
  // watch. This lets a reinstall restore the original one-time trial marker
  // instead of granting a new trial.
  if (dict[messageKeys.TRIAL_START] === 1) {
    var existing = parseInt(localStorage.getItem(TRIAL_STORAGE_KEY) || '0', 10);
    if (!existing) {
      existing = Math.floor(Date.now() / 1000);
      localStorage.setItem(TRIAL_STORAGE_KEY, String(existing));
    }
    dict[messageKeys.TRIAL_SYNC_EPOCH] = existing;
  }

  Pebble.sendAppMessage(dict,
    function() { console.log('Sent config data to Big Time'); },
    function(err) { console.log('Failed to send config data: ' + JSON.stringify(err)); });
});

// Messages from the watch: entitlement response, configuration acknowledgement,
// or weather request.
Pebble.addEventListener('appmessage', function(e) {
  if (!e.payload) return;

  if (typeof e.payload.LICENSE_STATUS !== 'undefined') {
    var status = parseInt(e.payload.LICENSE_STATUS, 10);
    console.log('Big Time entitlement status: ' + status);
    latestEntitlementStatus = status;
    if (pendingStatusOpen) openSettingsWithStatus(status);
    return;
  }

  if (typeof e.payload.CONFIG_ACK !== 'undefined') {
    console.log('WATCH APPLIED ACCENT_COLOR: ' + e.payload.CONFIG_ACK);
    return;
  }

  // The watch requests weather by sending key TEMPERATURE with a dummy value.
  // Do not treat KiezelPay's own AppMessages as weather requests.
  if (typeof e.payload.TEMPERATURE !== 'undefined') {
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
  }
});

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready');
  sendTrialSyncIfNeeded();

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
