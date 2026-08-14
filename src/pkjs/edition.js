// Single-build runtime licensing.
// The watch is the authority. index.js stores the last status received from C.
var unlocked = false;
try {
  unlocked = localStorage.getItem('amazfit_bip_port_pro') === '1';
} catch (e) {}

module.exports = {
  isPro: unlocked
};
