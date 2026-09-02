// ── Big Time — PebbleKit JS companion ────────────────────────────────
// Fetches weather from Open-Meteo through PebbleKit JS and sends weather
// data to the watch via AppMessage. No API key is required.

// ── Configuration ────────────────────────────────────────────────────────────
// Clay automatically opens the settings page and sends configured messageKey
// values to the watch when Save Settings is pressed.
var Clay = require('@rebble/clay');

// CloudPebble/RePebble configuration test mode. When enabled, the settings
// page always exposes Pro controls without requiring a KiezelPay entitlement.
// This is for emulator/UI testing only and must be false for publishing builds.
var CONFIG_TEST_MODE = false;

// Set the edition BEFORE config.js is first required. pypkjs/CloudPebble does
// not reliably expose Node's require.cache, so rebuilding config.js later can
// otherwise leave the first (Free) config snapshot cached for the whole run.
var editionState = require('./edition');
if (CONFIG_TEST_MODE) {
  editionState.entitlement = 2;
  editionState.isTrial = false;
  editionState.isPurchased = true;
  editionState.trialRemaining = 0;
  editionState.trialUsed = true;
  editionState.isPro = true;
  editionState.configTest = true;
}

var clayConfig = require('./config');

// Official KiezelPay phone-side companion. Test mode deliberately does not
// start it; licensing traffic is irrelevant in the emulator and can race with
// configuration AppMessages.
var KIEZELPAY_LOGGING = false;
var KiezelPay = require('kiezelpay-core');
var kiezelpay = CONFIG_TEST_MODE ? null : new KiezelPay(KIEZELPAY_LOGGING);

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

    // Show each Hide Label toggle only while its slot contains a value that
    // actually has a descriptive label. Clay's manipulators support hide/show.
    function sideSlotHasLabel(value) {
      value = String(value);
      return value === '0' || value === '1' || value === '3' ||
             value === '8' || value === '9' || value === '10' ||
             value === '11' || value === '12' || value === '15' ||
             value === '17' || value === '18' || value === '19';
    }

    function centerSlotHasLabel(value, topCenter) {
      value = String(value);
      if (topCenter) {
        // Top-center uses SideSlotContent values: Weather=0, HR=3.
        return value === '0' || value === '3' || value === '18';
      }
      // Bottom-center uses CenterSlotContent values: HR=0, Weather=3, Rain=11.
      return value === '0' || value === '3' || value === '11';
    }

    function bindConditionalLabelToggle(
        slotKey, toggleKey, isCenter, isTopCenter, timeZoneKey) {
      var slot = clayPage.getItemByMessageKey(slotKey);
      var toggle = clayPage.getItemByMessageKey(toggleKey);
      var timeZone = timeZoneKey
        ? clayPage.getItemByMessageKey(timeZoneKey)
        : null;
      if (!slot || !toggle) return;

      function sync() {
        var slotValue = String(slot.get());
        var visible = isCenter
          ? centerSlotHasLabel(slotValue, isTopCenter)
          : sideSlotHasLabel(slotValue);

        if (visible) toggle.show();
        else toggle.hide();

        // Side positions use this same listener for the dependent Time Zone
        // selector. No second listener is attached to the slot.
        if (timeZone) {
          if (slotValue === '15') timeZone.show();
          else timeZone.hide();
        }
      }

      sync();
      slot.on('change', sync);
    }

    bindConditionalLabelToggle(
      'TOP_LEFT_SLOT', 'TOP_LEFT_HIDE_LABEL', false, false,
      'TOP_LEFT_TIME_ZONE');
    bindConditionalLabelToggle(
      'TOP_CENTER_SLOT', 'TOP_CENTER_HIDE_LABEL', true, true, null);
    bindConditionalLabelToggle(
      'TOP_RIGHT_SLOT', 'TOP_RIGHT_HIDE_LABEL', false, false,
      'TOP_RIGHT_TIME_ZONE');
    bindConditionalLabelToggle(
      'LEFT_SLOT', 'LEFT_HIDE_LABEL', false, false,
      'LEFT_TIME_ZONE');
    bindConditionalLabelToggle(
      'CENTER_SLOT', 'CENTER_HIDE_LABEL', true, false, null);
    bindConditionalLabelToggle(
      'RIGHT_SLOT', 'RIGHT_HIDE_LABEL', false, false,
      'RIGHT_TIME_ZONE');

    // The Step Progress Bar can track either daily steps or remaining battery.
    // Preserve the user's step goal, but disable it while battery mode is active.
    var progressTrackBattery =
        clayPage.getItemByMessageKey('PROGRESS_TRACK_BATTERY');
    var stepGoal = clayPage.getItemByMessageKey('STEP_GOAL');

    if (progressTrackBattery && stepGoal) {
      function syncProgressSource() {
        if (progressTrackBattery.get()) stepGoal.disable();
        else stepGoal.enable();
      }
      syncProgressSource();
      progressTrackBattery.on('change', syncProgressSource);
    }

    // Migrate the old combined progress-bar mode to the new independent
    // Style + Visibility controls. Clay may still have values 2 or 5-8 saved
    // from v1.4.1, even though those values are no longer style choices.
    var stepbarStyle = clayPage.getItemByMessageKey('STEPBAR_MODE');
    var stepbarVisibility = clayPage.getItemByMessageKey('STEPBAR_VISIBILITY');
    if (stepbarStyle && stepbarVisibility) {
      var legacyStepbarMode = String(stepbarStyle.get());
      var migratedStyle = null;
      var migratedVisibility = null;
      if (legacyStepbarMode === '2') {
        migratedStyle = '0'; migratedVisibility = '2';
      } else if (legacyStepbarMode === '5') {
        migratedStyle = '0'; migratedVisibility = '1';
      } else if (legacyStepbarMode === '6') {
        migratedStyle = '1'; migratedVisibility = '1';
      } else if (legacyStepbarMode === '7') {
        migratedStyle = '3'; migratedVisibility = '1';
      } else if (legacyStepbarMode === '8') {
        migratedStyle = '4'; migratedVisibility = '1';
      }
      if (migratedStyle !== null) {
        stepbarStyle.set(migratedStyle);
        stepbarVisibility.set(migratedVisibility);
      }
    }

    // Split clock colors are opt-in for backward compatibility. Existing users
    // keep their single Clock Color until they explicitly enable this toggle.
    var splitClockColors =
        clayPage.getItemByMessageKey('SPLIT_CLOCK_COLORS');
    var hourColor = clayPage.getItemByMessageKey('HOUR_COLOR');
    var minuteColor = clayPage.getItemByMessageKey('MINUTE_COLOR');
    var clockFace = clayPage.getItemByMessageKey('CLOCK_FACE');
    var analogSecondHand = clayPage.getItemByMessageKey('ANALOG_SECOND_HAND');
    var secondHandColor = clayPage.getItemByMessageKey('SECOND_HAND_COLOR');
    var secondsColon = clayPage.getItemByMessageKey('FLASH_COLON');
    var bluetoothColon = clayPage.getItemByMessageKey('BLUETOOTH_COLON');
    var timeStyle = clayPage.getItemByMessageKey('ROUNDED_TIME');

    // In digital mode, Hour/Minute Color remain governed by the existing split
    // toggle. In analog mode they are the actual hand colors for Pro users.
    // The analog face and its second-hand toggle are available to Free users,
    // while hand-color controls remain part of Pro. Digital-only controls are
    // disabled whenever Analog is selected so their state is preserved but it
    // is clear that they do not apply to the analog face.
    function syncClockColorControls() {
      var analog = clockFace && String(clockFace.get()) === '1';
      var split = splitClockColors && !!splitClockColors.get();

      if (hourColor && minuteColor) {
        if (analog || split) {
          hourColor.show();
          minuteColor.show();
        } else {
          hourColor.hide();
          minuteColor.hide();
        }
      }

      if (analogSecondHand) {
        if (analog) analogSecondHand.show();
        else analogSecondHand.hide();
      }
      if (secondHandColor) {
        if (analog) secondHandColor.show();
        else secondHandColor.hide();
      }

      if (secondsColon) {
        if (analog) secondsColon.disable();
        else secondsColon.enable();
      }
      if (bluetoothColon) {
        if (analog) bluetoothColon.disable();
        else bluetoothColon.enable();
      }
      if (timeStyle) {
        if (analog) timeStyle.disable();
        else timeStyle.enable();
      }
    }

    syncClockColorControls();
    if (splitClockColors) {
      splitClockColors.on('change', function() {
        // Turning separate colors off means the independent values should no
        // longer remain stale behind the toggle. Copy the current Clock Color
        // into both selectors so digital and analog modes share one canonical
        // color until the user enables separation again.
        if (!splitClockColors.get()) {
          var clockColor = clayPage.getItemByMessageKey('CLOCK_COLOR');
          if (clockColor) {
            var unifiedColor = clockColor.get();
            if (hourColor) hourColor.set(unifiedColor);
            if (minuteColor) minuteColor.set(unifiedColor);
          }
        }
        syncClockColorControls();
      });
    }
    if (clockFace) clockFace.on('change', syncClockColorControls);


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
        setValue('SPLIT_CLOCK_COLORS', false);
        setValue('HOUR_COLOR', '0xFFFFFF');
        setValue('MINUTE_COLOR', '0xFFFFFF');
        setValue('SECOND_HAND_COLOR', '0xFFFFFF');
        setValue('FLASH_COLON', false);
        setValue('BLUETOOTH_COLON', false);
        setValue('CLOCK_FACE', '0');
        setValue('ANALOG_SECOND_HAND', false);
        setValue('PROGRESS_TRACK_BATTERY', false);
        setValue('ROUNDED_TIME', '0');
        setValue('TIME_FORMAT', '0');
        setValue('CENTER_12H', false);
        setValue('RAISE_WAKE', '0');
        setValue('BACKGROUND_COLOR', '0x000000');
        setValue('TEMP_UNIT', '0');
        setValue('LANGUAGE', '0');
        setValue('WEATHER_REFRESH', '60');
        setValue('TOP_LEFT_TIME_ZONE', '0');
        setValue('TOP_RIGHT_TIME_ZONE', '0');
        setValue('LEFT_TIME_ZONE', '0');
        setValue('RIGHT_TIME_ZONE', '0');
        setValue('RIGHT_HIDE_LABEL', false);
        setValue('CENTER_HIDE_LABEL', false);
        setValue('LEFT_HIDE_LABEL', false);
        setValue('TOP_RIGHT_HIDE_LABEL', false);
        setValue('TOP_CENTER_HIDE_LABEL', false);
        setValue('TOP_LEFT_HIDE_LABEL', false);

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
        setValue('STEPBAR_VISIBILITY', '0');

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

var sessionProUnlocked = false;
var sessionEntitlement = 0; // 0 Free, 1 Trial, 2 Purchased Pro
var PURCHASED_UI_CACHE_KEY = 'big_time_purchased_ui';

function cachedPurchasedUi() {
  try {
    return localStorage.getItem(PURCHASED_UI_CACHE_KEY) === '1';
  } catch (e) {
    return false;
  }
}

function setCachedPurchasedUi(enabled) {
  try {
    if (enabled) {
      localStorage.setItem(PURCHASED_UI_CACHE_KEY, '1');
    } else {
      localStorage.removeItem(PURCHASED_UI_CACHE_KEY);
    }
  } catch (e) {}
}
var sessionTrialRemaining = 0;

var TRIAL_DURATION_SECONDS = 48 * 60 * 60;
var TRIAL_EXPIRY_STORAGE_KEY = 'big_time_pro_trial_expires';
var TRIAL_USED_STORAGE_KEY = 'big_time_pro_trial_used';

function getStoredTrialExpiry() {
  try {
    var value = parseInt(localStorage.getItem(TRIAL_EXPIRY_STORAGE_KEY), 10);
    return isNaN(value) ? 0 : value;
  } catch (e) {
    return 0;
  }
}

function trialWasUsedOnPhone() {
  try {
    return localStorage.getItem(TRIAL_USED_STORAGE_KEY) === '1';
  } catch (e) {
    return false;
  }
}

function persistNewTrialOnPhone() {
  var now = Math.floor(Date.now() / 1000);
  var existingExpiry = getStoredTrialExpiry();

  // Never create a second trial once one has already existed on this phone.
  if (trialWasUsedOnPhone()) {
    return existingExpiry;
  }

  var expiry = now + TRIAL_DURATION_SECONDS;
  try {
    localStorage.setItem(TRIAL_EXPIRY_STORAGE_KEY, String(expiry));
    localStorage.setItem(TRIAL_USED_STORAGE_KEY, '1');
  } catch (e) {}

  return expiry;
}

function restorePersistedTrialToWatch() {
  if (!trialWasUsedOnPhone()) return;

  var expiry = getStoredTrialExpiry();
  var now = Math.floor(Date.now() / 1000);
  var value = expiry > now ? expiry : -1;

  Pebble.sendAppMessage(
    {'TRY_PRO_FREE': value},
    function() {
      console.log(value > 1
        ? 'Restored existing Pro trial to watch; expiry=' + value
        : 'Restored used/expired Pro trial marker to watch');
    },
    function(e) {
      console.log('Failed to restore Pro trial state to watch: ' + JSON.stringify(e));
    }
  );
}
var sessionLicenseKnown = false;
var sessionTrialUsed = trialWasUsedOnPhone();
var sessionWatchLanguage = null;
var pendingSavedLanguage = null;
var pendingOpenSettings = false;

// In CloudPebble/RePebble config-test mode, let Clay own the configuration
// lifecycle end-to-end. pypkjs reliably supports Clay's built-in
// showConfiguration/webviewclosed handlers, whereas our production manual
// wrapper is designed around the phone app's lifecycle.
var clay = new Clay(clayConfig, customClay, {
  autoHandleEvents: CONFIG_TEST_MODE
});

// Explicitly own the configuration-page lifecycle instead of relying on
// Clay's automatic Pebble event registration. This is more robust alongside
// KiezelPay/other packages that also register PebbleKit JS listeners.
function forceFullLanguageOptions(configItems) {
  var languageOptions = SUPPORTED_LANGUAGES;

  function visit(items) {
    if (!items) return;
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      if (!item) continue;

      if (item.messageKey === 'LANGUAGE') {
        // Always replace the entire array. This deliberately overrides an
        // older cached config module that may still contain only EN/SV.
        item.options = languageOptions.slice();
      }

      if (item.items && item.items.length) {
        visit(item.items);
      }
    }
  }

  visit(configItems);
}

function openSettingsPage() {
  try {
    if (CONFIG_TEST_MODE) {
      sessionEntitlement = 2;
      sessionProUnlocked = true;
      sessionLicenseKnown = true;
    }
    // Rebuild Clay with the current session entitlement so Pro controls are
    // shown only after the watch has actually confirmed a license.
    var editionModule = require('./edition');
    editionModule.entitlement = sessionEntitlement;
    editionModule.isTrial = sessionEntitlement === 1;
    editionModule.isPurchased = sessionEntitlement === 2;
    editionModule.trialRemaining = sessionTrialRemaining;
    editionModule.trialUsed = sessionTrialUsed;
    editionModule.isPro = sessionEntitlement > 0;
    sessionProUnlocked = editionModule.isPro;

    // config.js is loaded once by CommonJS, so create a fresh config module
    // snapshot by clearing its cache when available.
    try {
      delete require.cache[require.resolve('./config')];
    } catch (e) {}

    var liveConfig = require('./config');

    // Some Pebble companion environments retain the previously-loaded config
    // object even after a watchface update. Make the Free language selector
    // authoritative here so all supported languages are always exposed.
    forceFullLanguageOptions(liveConfig);

    clay = new Clay(liveConfig, customClay, { autoHandleEvents: false });

    if (sessionProUnlocked) {
      try {
        if (localStorage.getItem('big_time_pro_step_goal_initialized') !== '1') {
          clay.setSettings('STEP_GOAL', 5000);
          localStorage.setItem('big_time_pro_step_goal_initialized', '1');
        }
      } catch (e) {
        clay.setSettings('STEP_GOAL', 5000);
      }
    } else {
      try {
        localStorage.removeItem('big_time_pro_step_goal_initialized');
      } catch (e) {}
    }

    // Language is a Free preference. Keep the selector available and
    // synchronized regardless of entitlement.
    var languageForSettings =
        pendingSavedLanguage !== null ? pendingSavedLanguage :
        (sessionWatchLanguage !== null
          ? sessionWatchLanguage
          : getStoredLanguage());

    clay.setSettings('LANGUAGE', String(languageForSettings));
    console.log('Settings language selected: ' + languageForSettings);

    // Weather refresh is phone-side only, so keep Clay synchronized with the
    // value stored by Big Time rather than sending it to the watch.
    clay.setSettings('WEATHER_REFRESH', String(getWeatherRefreshMinutes()));

    console.log('Opening Clay configuration page; pro=' + sessionProUnlocked);
    Pebble.openURL(clay.generateUrl());
  } catch (e) {
    console.log('Failed to open configuration: ' + e);
  }
}

Pebble.addEventListener('showConfiguration', function() {
  if (CONFIG_TEST_MODE) {
    // Clay's automatic handler owns this event in emulator test mode.
    // Do not open a second generated URL from the production wrapper.
    console.log('CONFIG TEST MODE: Clay automatic showConfiguration handler active');
    return;
  }

  // Purchased Pro is stable enough to open immediately. This cached value only
  // controls which Clay controls are shown; C-side licensing remains the
  // authority and still rejects locked Pro AppMessage keys.
  if (sessionEntitlement === 2 || cachedPurchasedUi()) {
    // Purchased Pro can use the cached entitlement for UI availability, but
    // Settings must wait for one fresh watch status response so LANGUAGE (and
    // any other watch-authoritative status values) are current before Clay is
    // generated. Previously Settings opened first, then LANGUAGE arrived a
    // moment later, leaving the selector one step behind.
    sessionEntitlement = 2;
    sessionProUnlocked = true;
    sessionLicenseKnown = false;
    pendingOpenSettings = true;

    console.log('Refreshing purchased Pro status before opening Settings');

    Pebble.sendAppMessage(
      {'LICENSE_CHECK': 1},
      function() {},
      function(err) {
        console.log('Purchased status refresh send failed: ' + JSON.stringify(err));
      }
    );

    // Purchased status normally returns almost immediately. Keep a short
    // fallback so Settings remains responsive if AppMessage is delayed.
    setTimeout(function() {
      if (pendingOpenSettings) {
        pendingOpenSettings = false;
        sessionLicenseKnown = true;
        console.log('Purchased status refresh timed out; opening cached Settings');
        openSettingsPage();
      }
    }, 1500);

    return;
  }

  pendingOpenSettings = true;
  sessionLicenseKnown = false;

  Pebble.sendAppMessage(
    {'LICENSE_CHECK': 1},
    function() {},
    function() {
      sessionLicenseKnown = true;
      sessionEntitlement = 0;
      sessionProUnlocked = false;

      if (pendingOpenSettings) {
        pendingOpenSettings = false;
        openSettingsPage();
      }
    }
  );

  // Free/trial/unknown users still wait for the authoritative watch response.
  // Keep a failsafe so Settings can never become permanently unresponsive.
  setTimeout(function() {
    if (pendingOpenSettings) {
      pendingOpenSettings = false;

      if (!sessionLicenseKnown) {
        sessionEntitlement = 0;
        sessionProUnlocked = false;
        sessionTrialRemaining = 0;
      }

      openSettingsPage();
    }
  }, 4000);
});


Pebble.addEventListener('webviewclosed', function(e) {
  if (CONFIG_TEST_MODE) {
    // Clay's automatic handler parses the response and calls sendAppMessage.
    // Avoid a duplicate/manual parse in pypkjs.
    console.log('CONFIG TEST MODE: Clay automatic webviewclosed handler active');
    return;
  }

  if (!e || !e.response) {
    console.log('Configuration closed without saving');
    return;
  }

  try {
    var settings = clay.getSettings(e.response);

    if (settings && typeof settings.LANGUAGE !== 'undefined') {
      var savedLanguage = parseInt(settings.LANGUAGE, 10);
      sessionWatchLanguage = savedLanguage;
      pendingSavedLanguage = savedLanguage;
      storeLanguage(savedLanguage);
      console.log('Language saved; awaiting watch confirmation: ' + savedLanguage);
    }

    if (settings && typeof settings.WEATHER_REFRESH !== 'undefined') {
      setWeatherRefreshMinutes(settings.WEATHER_REFRESH);
      delete settings.WEATHER_REFRESH;
    }

    if (settings && Number(settings.TRY_PRO_FREE) !== 0) {
      var now = Math.floor(Date.now() / 1000);
      var existingExpiry = getStoredTrialExpiry();

      if (trialWasUsedOnPhone()) {
        if (existingExpiry > now) {
          // Trial already exists: restore the original end time, never restart it.
          settings.TRY_PRO_FREE = existingExpiry;
        } else {
          // Trial was already used and has expired. Do not send a new start.
          delete settings.TRY_PRO_FREE;
          console.log('Pro trial already used; refusing to restart');
        }
      } else {
        settings.TRY_PRO_FREE = persistNewTrialOnPhone();
        sessionTrialUsed = true;
        console.log('Stored new Pro trial expiry=' + settings.TRY_PRO_FREE);
      }
    }

    Pebble.sendAppMessage(
      settings,
      function() { console.log('Sent config data to Pebble'); },
      function(err) {
        console.log('Failed to send config data: ' + JSON.stringify(err));
      }
    );
  } catch (err) {
    console.log('Failed to process configuration response: ' + err);
  }
});

var messageKeys = require('message_keys');

// ── Weather via Open-Meteo ───────────────────────────────────────────────────
// Pebble's current C-watchface guidance is to use PebbleKit JS on the phone for
// geolocation/network access, then send weather to the watch with AppMessage.
// Open-Meteo requires no API key, so there is no secret embedded in this app.

// Icon codes sent to watch (must match KEY_WEATHER_ICON cases in main.c)
var ICON_CLEAR   = 0;
var ICON_CLOUDY  = 1;
var ICON_RAIN    = 2;
var ICON_SNOW    = 3;
var ICON_THUNDER = 4;

// LANGUAGE REGISTRY
// Add future languages HERE and in the watch-side checklist in main.c.
// Do not renumber existing values: they are persisted user settings.
var SUPPORTED_LANGUAGES = [
  { label: 'English', value: '0' },
  { label: 'Svenska', value: '1' },
  { label: 'Español', value: '2' },
  { label: 'Français', value: '3' },
  { label: 'Deutsch', value: '4' },
  { label: 'Português', value: '5' },
  { label: 'Català', value: '6' }
];

function isSupportedLanguage(value) {
  value = parseInt(value, 10);
  return !isNaN(value) &&
         value >= 0 &&
         value < SUPPORTED_LANGUAGES.length;
}

var FREE_LANGUAGE_STORAGE_KEY = 'big_time_language_v1';

function getStoredLanguage() {
  try {
    var value = parseInt(localStorage.getItem(FREE_LANGUAGE_STORAGE_KEY), 10);
    if (isSupportedLanguage(value)) return value;
  } catch (e) {}
  return 0;
}

function storeLanguage(value) {
  value = parseInt(value, 10);
  if (!isSupportedLanguage(value)) value = 0;
  try {
    localStorage.setItem(FREE_LANGUAGE_STORAGE_KEY, String(value));
  } catch (e) {}
}

var WEATHER_CACHE_STORAGE_KEY = 'big_time_weather_cache_v1';
var WEATHER_CACHE_TIME_STORAGE_KEY = 'big_time_weather_cache_time_v1';
var WEATHER_REFRESH_SETTING_KEY = 'big_time_weather_refresh_minutes';
var DEFAULT_WEATHER_REFRESH_MINUTES = 60;
var weatherRefreshTimer = null;

function getWeatherRefreshMinutes() {
  try {
    var value = parseInt(
      localStorage.getItem(WEATHER_REFRESH_SETTING_KEY), 10);
    if (value === 15 || value === 30 || value === 60 || value === 120 ||
        value === 180 || value === 360) {
      return value;
    }
  } catch (e) {}
  return DEFAULT_WEATHER_REFRESH_MINUTES;
}

function setWeatherRefreshMinutes(value) {
  value = parseInt(value, 10);
  if (value !== 15 && value !== 30 && value !== 60 && value !== 120 &&
      value !== 180 && value !== 360) {
    value = DEFAULT_WEATHER_REFRESH_MINUTES;
  }

  try {
    localStorage.setItem(
      WEATHER_REFRESH_SETTING_KEY, String(value));
  } catch (e) {}

  // Recalculate the next due time without discarding or re-fetching the
  // existing cached weather.
  scheduleNextWeatherRefresh();
}

function getWeatherRefreshMs() {
  return getWeatherRefreshMinutes() * 60 * 1000;
}

function readCachedWeather() {
  try {
    var raw = localStorage.getItem(WEATHER_CACHE_STORAGE_KEY);
    if (!raw) return null;
    var payload = JSON.parse(raw);
    return payload && typeof payload.TEMPERATURE !== 'undefined'
      ? payload : null;
  } catch (e) {
    return null;
  }
}

function readCachedWeatherTime() {
  try {
    var value = parseInt(
      localStorage.getItem(WEATHER_CACHE_TIME_STORAGE_KEY), 10);
    return isNaN(value) ? 0 : value;
  } catch (e) {
    return 0;
  }
}

function storeCachedWeather(payload) {
  try {
    localStorage.setItem(
      WEATHER_CACHE_STORAGE_KEY, JSON.stringify(payload));
    localStorage.setItem(
      WEATHER_CACHE_TIME_STORAGE_KEY, String(Date.now()));
  } catch (e) {}
}

function weatherRefreshDue() {
  var cached = readCachedWeather();

  // A cache written by an older Big Time build can still be fresh while
  // lacking newly-added forecast fields. Refresh immediately once so +2 Hours
  // does not remain "--" until the user's normal interval expires.
  if (!cached ||
      typeof cached.PLUS2_ICON === 'undefined' ||
      typeof cached.PLUS2_TEMP === 'undefined') {
    return true;
  }

  var timestamp = readCachedWeatherTime();
  return !timestamp || (Date.now() - timestamp) >= getWeatherRefreshMs();
}

function scheduleNextWeatherRefresh() {
  if (weatherRefreshTimer) {
    clearTimeout(weatherRefreshTimer);
    weatherRefreshTimer = null;
  }

  var age = Date.now() - readCachedWeatherTime();
  var delay = getWeatherRefreshMs() - Math.max(0, age);
  if (delay < 1000) delay = 1000;

  weatherRefreshTimer = setTimeout(function() {
    weatherRefreshTimer = null;
    refreshWeatherIfDue();
  }, delay);
}

function iconFromOpenMeteo(code) {
  code = Number(code);

  // WMO weather interpretation codes used by Open-Meteo.
  if (code === 0 || code === 1) return ICON_CLEAR;
  if (code === 2 || code === 3 ||
      code === 45 || code === 48) return ICON_CLOUDY;

  if ((code >= 51 && code <= 67) ||
      (code >= 80 && code <= 82)) return ICON_RAIN;

  if ((code >= 71 && code <= 77) ||
      (code >= 85 && code <= 86)) return ICON_SNOW;

  if (code >= 95 && code <= 99) return ICON_THUNDER;

  return ICON_CLOUDY;
}

function minuteOfDayFromLocalIso(localIso) {
  // Open-Meteo returns daily sunrise/sunset in the requested local timezone,
  // e.g. "2026-08-20T06:11". We only need minutes after local midnight.
  if (!localIso || typeof localIso !== 'string') return -1;

  var match = /T(\d{2}):(\d{2})/.exec(localIso);
  if (!match) return -1;

  var hour = Number(match[1]);
  var minute = Number(match[2]);
  if (isNaN(hour) || isNaN(minute)) return -1;

  return hour * 60 + minute;
}

function sendWeatherPayload(payload) {
  Pebble.sendAppMessage(
    payload,
    function() { console.log('Open-Meteo weather sent'); },
    function(e) { console.log('Weather send failed: ' + JSON.stringify(e)); }
  );
}

function fetchWeather(lat, lon) {
  var url = 'https://api.open-meteo.com/v1/forecast'
    + '?latitude=' + encodeURIComponent(lat)
    + '&longitude=' + encodeURIComponent(lon)
    + '&current=temperature_2m,weather_code'
    + '&hourly=precipitation_probability,temperature_2m,weather_code'
    + '&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,weather_code,precipitation_probability_max'
    + '&temperature_unit=celsius'
    + '&timezone=auto'
    + '&forecast_days=2';

  var xhr = new XMLHttpRequest();

  xhr.onload = function() {
    if (xhr.status !== 200) {
      console.log('Open-Meteo HTTP error: ' + xhr.status);
      return;
    }

    try {
      var data = JSON.parse(xhr.responseText);

      if (!data.current ||
          typeof data.current.temperature_2m !== 'number' ||
          typeof data.current.weather_code === 'undefined') {
        console.log('Open-Meteo response missing current weather');
        return;
      }

      var payload = {
        'TEMPERATURE': Math.round(data.current.temperature_2m * 10),
        'WEATHER_ICON': iconFromOpenMeteo(data.current.weather_code)
      };

      if (data.daily) {
        if (data.daily.temperature_2m_max &&
            typeof data.daily.temperature_2m_max[0] === 'number') {
          payload.HIGH_TEMP =
              Math.round(data.daily.temperature_2m_max[0] * 10);
        }

        if (data.daily.temperature_2m_min &&
            typeof data.daily.temperature_2m_min[0] === 'number') {
          payload.LOW_TEMP =
              Math.round(data.daily.temperature_2m_min[0] * 10);
        }

        // Rain Chance is the probability for the current forecast hour.
        // It updates on the normal weather refresh schedule and is cached with
        // the rest of the weather data.
        if (data.hourly && data.hourly.time &&
            data.hourly.precipitation_probability) {
          var currentTime = data.current && data.current.time;
          var rainIndex = currentTime ? data.hourly.time.indexOf(currentTime) : -1;

          if (rainIndex < 0 && currentTime) {
            var currentHour = currentTime.substring(0, 13);
            for (var ri = 0; ri < data.hourly.time.length; ri++) {
              if (data.hourly.time[ri].substring(0, 13) === currentHour) {
                rainIndex = ri;
                break;
              }
            }
          }

          if (rainIndex >= 0 &&
              typeof data.hourly.precipitation_probability[rainIndex] === 'number') {
            payload.RAIN_CHANCE = Math.max(0, Math.min(100,
                Math.round(data.hourly.precipitation_probability[rainIndex])));
          }

          // +2 Hours uses the forecast point two hourly buckets after the
          // current local forecast hour. It refreshes/caches with weather.
          var plus2Index = rainIndex >= 0 ? rainIndex + 2 : -1;
          if (plus2Index < 0) {
            console.log('Unable to match current hour for +2 Hours forecast: ' +
                        currentTime);
          }
          if (plus2Index >= 0 &&
              plus2Index < data.hourly.time.length) {
            if (data.hourly.weather_code &&
                typeof data.hourly.weather_code[plus2Index] !== 'undefined') {
              payload.PLUS2_ICON =
                  iconFromOpenMeteo(data.hourly.weather_code[plus2Index]);
            }
            if (data.hourly.temperature_2m &&
                typeof data.hourly.temperature_2m[plus2Index] === 'number') {
              payload.PLUS2_TEMP =
                  Math.round(data.hourly.temperature_2m[plus2Index] * 10);
            }
          }
        }

        // Forecast is tomorrow: icon + high/low. Keep it in the same weather
        // payload/cache so all weather-derived values refresh atomically.
        if (data.daily.weather_code &&
            typeof data.daily.weather_code[1] !== 'undefined') {
          payload.FORECAST_ICON = iconFromOpenMeteo(data.daily.weather_code[1]);
        }
        if (data.daily.temperature_2m_max &&
            typeof data.daily.temperature_2m_max[1] === 'number') {
          payload.FORECAST_HIGH_TEMP =
              Math.round(data.daily.temperature_2m_max[1] * 10);
        }
        if (data.daily.temperature_2m_min &&
            typeof data.daily.temperature_2m_min[1] === 'number') {
          payload.FORECAST_LOW_TEMP =
              Math.round(data.daily.temperature_2m_min[1] * 10);
        }

        if (data.daily.sunrise && data.daily.sunrise[0]) {
          var sunriseMinute = minuteOfDayFromLocalIso(data.daily.sunrise[0]);
          if (sunriseMinute >= 0) payload.SUNRISE = sunriseMinute;
        }

        if (data.daily.sunset && data.daily.sunset[0]) {
          var sunsetMinute = minuteOfDayFromLocalIso(data.daily.sunset[0]);
          if (sunsetMinute >= 0) payload.SUNSET = sunsetMinute;
        }
      }

      storeCachedWeather(payload);
      sendWeatherPayload(payload);
      scheduleNextWeatherRefresh();
    } catch (e) {
      console.log('Open-Meteo parse error: ' + e);
    }
  };

  xhr.onerror = function() {
    console.log('Open-Meteo network error');
  };

  xhr.open('GET', url);
  xhr.send();
}

function fetchWeatherForCurrentLocation() {
  navigator.geolocation.getCurrentPosition(
    function(pos) {
      fetchWeather(pos.coords.latitude, pos.coords.longitude);
    },
    function(err) {
      console.log('Geolocation error: ' + err.message);
      scheduleNextWeatherRefresh();
    },
    { timeout: 15000, maximumAge: 300000 }
  );
}

function refreshWeatherIfDue() {
  if (!weatherRefreshDue()) {
    scheduleNextWeatherRefresh();
    return;
  }

  console.log('Weather cache due for refresh');
  fetchWeatherForCurrentLocation();
}

// Messages from the watch: configuration acknowledgement or weather request.
Pebble.addEventListener('appmessage', function(e) {
  if (!e || !e.payload) return;

  // The watch is authoritative for the active language. Capture it whenever
  // present so the next Settings page reflects what is actually displayed.
  if (typeof e.payload.LANGUAGE !== 'undefined') {
    var reportedLanguage = parseInt(e.payload.LANGUAGE, 10);

    if (pendingSavedLanguage !== null &&
        reportedLanguage !== pendingSavedLanguage) {
      // A status response that was already in flight can still contain the
      // language from before the most recent Settings Save. Do not let that
      // stale response overwrite the user's new selection.
      console.log(
        'Ignoring stale watch language ' + reportedLanguage +
        '; waiting for saved language ' + pendingSavedLanguage);
    } else {
      sessionWatchLanguage = reportedLanguage;
      storeLanguage(reportedLanguage);

      if (pendingSavedLanguage !== null &&
          reportedLanguage === pendingSavedLanguage) {
        console.log('Watch confirmed saved language: ' + reportedLanguage);
        pendingSavedLanguage = null;
      } else {
        console.log('Synced language from watch: ' + reportedLanguage);
      }
    }
  }

  if (typeof e.payload.CONFIG_ACK !== 'undefined') {
    console.log('WATCH APPLIED ACCENT_COLOR: ' + e.payload.CONFIG_ACK);
    return;
  }

  // KiezelPay also uses AppMessage. Only WEATHER_REQUEST belongs to Big Time's
  // weather bridge; ignore all other package traffic here.
  if (typeof e.payload.WEATHER_REQUEST === 'undefined') {
    return;
  }

  console.log('Watch requested weather update');
  var cached = readCachedWeather();
  if (cached) {
    sendWeatherPayload(cached);
  }
  refreshWeatherIfDue();
});

// Fetch on launch.
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready');

  // Licensing is intentionally absent in emulator config-test mode. Sending
  // trial/license traffic here only creates unnecessary AppMessage contention.
  if (!CONFIG_TEST_MODE) {
    // PebbleKit JS localStorage survives watchapp uninstall/reinstall. If an
    // existing trial is still active, restore its ORIGINAL expiry to the watch.
    // If it already expired, do nothing so reinstalling cannot grant another one.
    restorePersistedTrialToWatch();
  }

  var cachedWeather = readCachedWeather();
  if (cachedWeather) {
    console.log('Sending cached weather on PebbleKit ready');
    sendWeatherPayload(cachedWeather);
  }

  refreshWeatherIfDue();
});

// Runtime Pro status is authoritative from the watch/C side.
Pebble.addEventListener('appmessage', function(e) {
  if (!e || !e.payload || typeof e.payload.PRO_LICENSE === 'undefined') return;

  if (CONFIG_TEST_MODE) {
    sessionEntitlement = 2;
    sessionProUnlocked = true;
    sessionLicenseKnown = true;
    console.log('CONFIG TEST MODE: ignoring runtime license state for settings UI');
    if (pendingOpenSettings) {
      pendingOpenSettings = false;
      openSettingsPage();
    }
    return;
  }

  sessionEntitlement = Number(e.payload.PRO_LICENSE);
  if (isNaN(sessionEntitlement) || sessionEntitlement < 0 || sessionEntitlement > 2) {
    sessionEntitlement = 0;
  }
  sessionProUnlocked = sessionEntitlement > 0;
  sessionLicenseKnown = true;

  if (sessionEntitlement === 2) {
    setCachedPurchasedUi(true);
  } else if (sessionEntitlement === 0) {
    // Only an authoritative Free response clears the purchased UI cache.
    setCachedPurchasedUi(false);
  }

  console.log('Fresh watch entitlement received: ' + sessionEntitlement);

  if (typeof e.payload.LICENSE_CHECK !== 'undefined') {
    var remaining = Number(e.payload.LICENSE_CHECK);
    sessionTrialRemaining = (!isNaN(remaining) && remaining > 0) ? remaining : 0;
    if (!isNaN(remaining) && remaining < 0) {
      sessionTrialUsed = true;
      try { localStorage.setItem(TRIAL_USED_STORAGE_KEY, '1'); } catch (e) {}
    }
  } else if (sessionEntitlement !== 1) {
    sessionTrialRemaining = 0;
  }

  var entitlementLabel =
      sessionEntitlement === 2 ? 'purchased Pro' :
      sessionEntitlement === 1 ? 'free trial' : 'free';
  console.log('Big Time entitlement from watch: ' + entitlementLabel);

  if (pendingOpenSettings) {
    pendingOpenSettings = false;
    openSettingsPage();
  }
});
