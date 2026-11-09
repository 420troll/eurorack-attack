# TROLLfx Chakra Plaits — Force build and install

Pinned inputs used for the first ARM32 build:

- `poloq-instruments/mpc-vst-plaits` commit `b113c83dd8d72fc4884888aaeb8d404feced69df`
- `sd88me/mpc-vst-plugins` commit `6634b26142aa54795a6aff36a3467bc5c218d2cb`
- Chakra integration: `chakra-plaits.patch` in this directory

The patch retains the native Plaits engine, all 24 models, six upstream pages, Q-Links and MIDI handling. It appends four parameters: source, named Chakra preset, manual Hz and hold. The held Chakra voice has a private note owner outside MIDI's 0–127 range, so releasing it cannot release an external MIDI note. It does not add subharmonics or pitch glide.

## Build and offline verification

```bash
git clone https://github.com/poloq-instruments/mpc-vst-plaits.git
python3 eurorack-attack/TROLLfx-Chakra-Drone/force/add_chakra_page.py mpc-vst-plaits
cd mpc-vst-plaits
MPC_VST=../mpc-vst-plugins ./build.sh
MPC_VST=../mpc-vst-plugins TEST=tests/chakra.c tests/run_c.sh
MPC_VST=../mpc-vst-plugins ../mpc-vst-plugins/tools/test_port.sh vst/vst.json
```

Expected ARM artifact: `vst/build/chakra_plaits.so`. The generated skin folder is exactly `TROLLfx - VST - Chakra Plaits`, and the VST2 UID is `TcPl`, separate from upstream MPC Plaits (`MiPl`).

## Safe Force deployment

Save the open project before any service restart. First inventory the device and locate `MPC.settings` and the MockbaMod scanner. Back up settings with a UTC timestamp before changing it. Stage every plugin file with a `.new` suffix and atomically rename it only after checksum verification.

Install the generated skin directory as:

```text
/media/662522/Synths/TROLLfx - VST - Chakra Plaits/
```

Run MockbaMod's existing `vstscanner.sh` rather than rebuilding the whole plugin list by hand. Preserve every unrelated plugin-list entry. A restart is only permitted after the open project is saved and `MPC.settings` is backed up.

## Hands-on acceptance test

1. In the Force plugin browser, insert **TROLLfx Chakra Plaits** on a new plugin track.
2. Confirm the CHAKRA page and all original PLAITS, VOICE, ENV, LFO, MOD and ASSIGN pages are present.
3. Set SOURCE to PRESET, select Heart 639 Hz, turn HOLD on, then off. Confirm sound starts and releases without a stuck note.
4. Hold a pad while toggling Chakra HOLD off. The pad note must continue; release the pad and confirm its release tail.
5. Set SOURCE to MANUAL HZ and try 220, 440 and 880 Hz. Record a tuner or analyzer reading; do not call frequency tracking passed without that evidence.
6. Cycle all 24 models and move model, HARMONICS, TIMBRE, MORPH and OUT/AUX from both touch controls and Q-Links.
7. Test velocity, sustain pedal, pitch bend, mod wheel, aftertouch if available, note release, and CC 123/all-notes-off.
8. Save the project, reload it, and confirm SOURCE, preset/manual Hz and HOLD-off state restore safely.
9. Record Force CPU while holding 1, 4 and 8 notes, then inspect the service journal for xruns, load failures or crashes.

No physical-interaction item is considered passed until its observation and diagnostics are recorded.
