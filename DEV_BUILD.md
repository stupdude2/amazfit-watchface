# Big Time development build

Internal development build based on public version 1.3.1.

## dev-21
- Fixes tap-to-reveal after a light-only wrist raise on Pebble Time 2 watchfaces.
- Keeps the direct TouchService path when firmware delivers it.
- Adds a BacklightService fallback: the first ON event belongs to the wrist raise; a later ON retrigger during that same light-only session is treated as the screen tap and reveals conditional data.
- The reveal remains latched until the backlight turns off.
