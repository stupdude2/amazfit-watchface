# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-007
Based on: 1.3-dev-006

Change:
- Forecast Tomorrow and Forecast +2 Hours icons are now rebuilt immediately
  from watch-persisted weather state as soon as their bitmap layers are created.
- Returning from menus/settings no longer depends on a PebbleKit JS weather
  resend for forecast icons to appear.
- Cached icons remain static until the normal weather refresh replaces them.
