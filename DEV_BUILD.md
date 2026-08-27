# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-012
Based on: 1.3-dev-011

Bluetooth Colon:
- Optional Bluetooth connectivity indicator using the bottom clock colon dot.
- Connected: normal filled bottom colon dot.
- Disconnected: colon-color outline with background visible through the center.
- Seconds Colon still flashes the entire colon normally.
- Square, Slightly Rounded, and Rounded Time Styles are respected.
- Bluetooth connection changes redraw the clock immediately.
- dev-011 repeat-save/idempotence safeguards are retained.
