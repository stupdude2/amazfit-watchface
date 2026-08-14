// ── Big Time — PebbleKit JS companion ───────────────────────────────────────
// Fetches weather from OpenWeatherMap and sends temperature + icon code
// to the watch via AppMessage.
//
// SETUP: Replace YOUR_API_KEY below with a free key from openweathermap.org

// ── Configuration ────────────────────────────────────────────────────────────
// Clay automatically opens the settings page and sends configured messageKey
// values to the watch when Save Settings is pressed.
var Clay = require('@rebble/clay');
var buildClayConfig = require('./config');

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
    // Trial/purchase controls are actions, not persistent preferences. Reset
    // them whenever Settings opens so a later Save cannot accidentally repeat.
    var trialAction = clayPage.getItemByMessageKey('TRIAL_START');
    var purchaseAction = clayPage.getItemByMessageKey('PURCHASE_PRO');
    if (trialAction) trialAction.set(false);
    if (purchaseAction) purchaseAction.set(false);

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
        setValue('ACCENT_COLOR', '0x0000AA');
        setValue('CLOCK_COLOR', '0xFFFFFF');
        setValue('TIME_FORMAT', '0');
        setValue('CENTER_12H', false);
        setValue('RAISE_WAKE', '0');
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

// Build Settings only after the watch reports its current entitlement.
// Phone localStorage is a cache only; it must never decide whether Pro controls
// are shown because it can survive a watchface uninstall/reinstall.
var clay = new Clay(buildClayConfig(false, 0, 0), customClay, { autoHandleEvents: false });
var settingsOpenPending = false;
var settingsOpenTimer = null;
var currentProUnlocked = false;
var currentTrialState = 0;
var currentTrialRemaining = 0;

function openSettingsWithCurrentEntitlement() {
  if (!settingsOpenPending) return;
  settingsOpenPending = false;
  if (settingsOpenTimer) {
    clearTimeout(settingsOpenTimer);
    settingsOpenTimer = null;
  }

  clay.config = buildClayConfig(currentProUnlocked, currentTrialState, currentTrialRemaining);
  console.log('Opening Settings as ' + (currentProUnlocked ? 'PRO' : 'FREE') +
              ', trialState=' + currentTrialState);
  Pebble.openURL(clay.generateUrl());
}

Pebble.addEventListener('showConfiguration', function() {
  settingsOpenPending = true;

  // Pessimistic fallback: if the watch cannot answer, never expose Pro controls
  // based solely on stale phone-side state.
  currentProUnlocked = false;
  currentTrialState = 0;
  currentTrialRemaining = 0;

  Pebble.sendAppMessage({'LICENSE_CHECK': 1}, function() {
    console.log('Requested current entitlement before opening Settings');
  }, function() {
    console.log('Could not request entitlement; opening Settings in Free mode');
    openSettingsWithCurrentEntitlement();
  });

  settingsOpenTimer = setTimeout(function() {
    console.log('Entitlement response timeout; opening Settings in Free mode');
    openSettingsWithCurrentEntitlement();
  }, 1500);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) return;
  var dict = clay.getSettings(e.response);
  Pebble.sendAppMessage(dict, function() {
    console.log('Sent config data to Pebble');
  }, function(err) {
    console.log('Failed to send config data: ' + JSON.stringify(err));
  });
});
var messageKeys = require('message_keys');

// KiezelPay phone-side bridge. Keep logging enabled while this package is in
// test mode; set it to false for the Store release.
var KIEZELPAY_LOGGING = true;
var KiezelPay = require('kiezelpay-core');
var kiezelpay = new KiezelPay(KIEZELPAY_LOGGING);

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

// Messages from the watch: configuration acknowledgement or weather request.
Pebble.addEventListener('appmessage', function(e) {
  if (e.payload && typeof e.payload.CONFIG_ACK !== 'undefined') {
    console.log('WATCH APPLIED ACCENT_COLOR: ' + e.payload.CONFIG_ACK);
    return;
  }

  // Entitlement/trial status messages are not weather requests.
  if (e.payload && (typeof e.payload.PRO_LICENSE !== 'undefined' ||
                    typeof e.payload.TRIAL_STATE !== 'undefined' ||
                    typeof e.payload.TRIAL_REMAINING !== 'undefined')) {
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
  Pebble.sendAppMessage({'LICENSE_CHECK': 1}, function(){}, function(){});
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

// Runtime Pro status is authoritative from the watch/C side.
// The product-specific KiezelPay library will call watchface_kiezelpay_set_licensed()
// in C; the watch mirrors that state to JS with PRO_LICENSE.
Pebble.addEventListener('appmessage', function(e) {
  if (!e || !e.payload) return;

  var receivedEntitlement = false;

  if (typeof e.payload.PRO_LICENSE !== 'undefined') {
    currentProUnlocked = Number(e.payload.PRO_LICENSE) === 1;
    try { localStorage.setItem('big_time_pro', currentProUnlocked ? '1' : '0'); } catch (err) {}
    console.log('Pro entitlement from watch: ' + (currentProUnlocked ? 'unlocked' : 'free'));
    receivedEntitlement = true;
  }

  if (typeof e.payload.TRIAL_STATE !== 'undefined') {
    currentTrialState = Number(e.payload.TRIAL_STATE) || 0;
    try {
      localStorage.setItem('big_time_trial_state', String(currentTrialState));
      if (currentTrialState === 1 || currentTrialState === 2 || currentTrialState === 3) {
        localStorage.setItem('big_time_trial_ever_used', '1');
      }
    } catch (err2) {}
    console.log('Pro trial state from watch: ' + currentTrialState);
    receivedEntitlement = true;
  }

  if (typeof e.payload.TRIAL_REMAINING !== 'undefined') {
    currentTrialRemaining = Number(e.payload.TRIAL_REMAINING) || 0;
    try { localStorage.setItem('big_time_trial_remaining', String(currentTrialRemaining)); } catch (err3) {}
    receivedEntitlement = true;
  }

  if (settingsOpenPending && receivedEntitlement &&
      typeof e.payload.PRO_LICENSE !== 'undefined' &&
      typeof e.payload.TRIAL_STATE !== 'undefined') {
    // A watchface reinstall clears Pebble persistent storage, but PebbleKit JS
    // localStorage normally survives. Use that surviving marker only to prevent
    // a second developer-managed trial; it never grants Pro entitlement.
    var trialEverUsed = false;
    try { trialEverUsed = localStorage.getItem('big_time_trial_ever_used') === '1'; } catch (err4) {}
    if (!currentProUnlocked && currentTrialState === 0 && trialEverUsed) {
      currentTrialState = 2;
      try { localStorage.setItem('big_time_trial_state', '2'); } catch (err5) {}
      Pebble.sendAppMessage({'TRIAL_USED_HINT': 1}, function() {
        console.log('Restored used-trial marker to watch after reinstall');
      }, function() {
        console.log('Could not restore used-trial marker to watch');
      });
    }
    openSettingsWithCurrentEntitlement();
  }
});
