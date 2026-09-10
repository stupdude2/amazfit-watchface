# Big Time 1.4.10 Production Release

## Changes
- Added one configurable Custom URL data source for public HTTP/HTTPS plain-text endpoints.
- Added optional custom label; blank label uses the established larger no-label presentation.
- Added independent Custom URL refresh interval: 5, 15, 30, 60, 120, or 360 minutes.
- Custom URL data can be selected in top/bottom left, center, and right data areas.
- Last successful custom value is cached and restored while waiting for the next refresh.

## Production safeguards
- CONFIG_TEST_MODE remains disabled.
- KiezelPay production licensing remains enabled.
- Existing application UUID is retained.
