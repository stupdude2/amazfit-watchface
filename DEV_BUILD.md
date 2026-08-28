# Development Build

Big Time v1.3.1 rectangular analog development build 12.

Changes in dev-12:
- When the lower steps bar is hidden while the bottom data bar remains visible, the analog clock now splits the former 12 px step-bar allowance into matching top and bottom margins.
- With both top and bottom bars visible, the analog face is vertically centered in the available region with 6 px of breathing room above and below.
- If the top bar is hidden, only the lower 6 px footer gap is retained so the analog face can continue expanding upward.


## Rectangular analog dev-13
Removed the extra upper half-gap when the lower step bar is hidden. The corrected lower margin above a visible footer is retained, while the analog face starts at the normal top clock position.

## Analog dev-14
- Tightened spacing for an always/backlight-visible step bar positioned above the analog clock.
- Restores the legacy 5 px upward step-bar tuck and 1 px clock gap when the header is visible, preventing the analog dial from being pushed too far down.

- dev-15: Tightened spacing between an above-clock steps bar and the analog dial by allowing the analog frame to overlap the unused lower portion of the 12 px step-bar layer. This preserves the step graphic while making the visible top/bottom dial margins more even.
## dev-17 — Analog hand colors
- Analog hour and minute hands now use the existing Hour Color and Minute Color selectors directly.
- Hour/Minute color controls are shown whenever Analog is selected, even when digital split colors are disabled.
- Added a Second Hand Color selector (default white), shown only for the Analog face.
- Analog second-hand visibility remains independently toggleable.
- Removed the accent-colored second-hand tip; the full second hand is now one continuous selected color.
- Analog hand defaults are white.


- dev-18: Analog numerals and hour-marker dashes now use the existing background contrast rule (white on dark backgrounds, black on sufficiently light backgrounds).
