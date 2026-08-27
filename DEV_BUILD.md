# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-006
Based on: 1.3-dev-005

Changes:
- Dropdown labels renamed to Forecast Tomorrow and Forecast +2 Hours.
- Fresh-but-legacy weather caches that lack +2h data now trigger one immediate
  refresh instead of leaving the new forecast at "--" until the interval expires.
- Added diagnostic logging if the current Open-Meteo hour cannot be matched.
- Custom Raise to Wake now checks Pebble Quiet Time and suppresses the backlight
  while Quiet Time is active.
