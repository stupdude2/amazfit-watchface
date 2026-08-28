# Big Time development build

Current public package version remains 1.3.1.

Development iteration: rectangular analog dev-6

- Restores the normal two-bar analog clock frame to the exact legacy clock area (`CLOCK_Y` / `CLOCK_H`).
- Keeps dynamic expansion into released top/bottom space, including full-screen when both data bars are hidden.
- Fixes the 12 px downward visual shift introduced when the analog morph target accidentally included the step-bar area while the bottom bar was visible.

## Rectangular analog dev-7
- Fixed the analog layout regression affecting the step/progress bar.
- Analog geometry now reserves the 12 px progress-bar strip only while that bar is actually visible.
- When the progress bar is hidden, the analog face correctly reclaims that space.
- Above/below progress-bar modes now reserve space on the correct side of the analog dial.
