# Big Time development build

Development iteration: rectangular analog dev-34
Public/package version remains 1.3.1.

Reverted the analog hour-marker design to the last build before parallelogram markers were introduced (dev-25 baseline).

- Restored the original pre-parallelogram analog tick marks.
- Removed all later parallelogram-specific geometry, thickness, scaling-state, and positioning experiments.
- Preserved every feature and fix that existed through dev-25, including rectangular analog mode, dynamic bar-aware resizing, analog hand colors, second-hand color, automatic dial contrast, and the Separate Hour / Minute Colors reset behavior.
- Wrist gesture/backlight behavior remains the normal known-good behavior restored before this baseline.
