# TROLLfx Chakra Page for poloq MPC Plaits

Upstream: https://github.com/poloq-instruments/mpc-vst-plaits

The upstream plugin already exposes real Plaits DSP, 24 models, polyphony and touchscreen/Q-Links. Add a seventh CHAKRA tab **without replacing existing tabs**.

## Trigger design

- Popup select: Root 396 Hz, Sacral 417 Hz, Solar 528 Hz, Heart 639 Hz, Throat 741 Hz, Third Eye 852 Hz, Crown 963 Hz.
- Touch **TRIGGER** button: note-on at selected chakra frequency; touch again: note-off.
- Trigger via existing `ppf_engine_t::note_on` and `note_off` (no outbound MIDI bridge necessary).
- Normal Force MIDI/pads and sequencer continue to work.
- Keep HARMONICS, TIMBRE, MORPH, OUT/AUX, and original model selector fully accessible.
- Convert the selected frequency to a fractional MIDI pitch with `69 + 12*log2(chakra_hz / 440)` and apply it only to the owned Chakra voice. The upstream compensation for its corrected Plaits sample rate remains active; verify actual output frequency by spectral test.
- Changing a preset or manual Hz while HOLD is active retunes the owned voice without adding another glide stage or retriggering it.
- On plugin close or all-notes-off, clear the held voice. Persist state without leaving stuck notes.
- The Chakra voice uses owner ID 128, outside external MIDI's 0–127 range, so Chakra release cannot release a same-pitch external note. It still shares the normal eight-voice allocator and native mono/poly/unison rules.
- HOLD is a held note, not a momentary tap. Turning it off, CC 120/123 and the wrapper's all-notes-off path release it.

## Sound plans

Phase 1: one selected Plaits engine with chakra trigger and native controls.
Phase 2: chord intervals / inversions using native polyphony, routing 4 note-ons and matching note-offs. Budget Plaits 8 voices total.
Phase 3: independent additive EVEN/ODD partial bank + root/2, /3, /4, /8 subregisters. These need separate synth voices; stock HARMONICS does **not** provide independent even/odd level control.

## Minimum native source edit points

- `src/module.json`: append `chakra_source`, `chakra_root`, `chakra_hz` and `chakra_gate` to `chain_params`, preserving existing parameter order.
- `vst/layout.conf`: add a `[tab CHAKRA]` with a root selection popup, a trigger toggle, Harmonics/Timbre/Morph and OUT/AUX knobs; add page Q-Link assignments.
- `src/dsp/plaits_move_engine.*`: own the private Chakra voice and fractional fundamental while leaving external MIDI note ownership unchanged.
- `vst/vst.json`: rename plugin and change UID from `MiPl` to unique TROLLfx ID to avoid colliding with installed MPC Plaits. Change .so output.
- `src/module.json`: also rename plugin and its unique ID.
- Retain upstream MIT notices, derived artwork CC BY-SA requirements, and records of source commit.

## Build prerequisites

```bash
git clone https://github.com/poloq-instruments/mpc-vst-plaits.git
git clone -b poloq-dev https://github.com/sd88me/mpc-vst-plugins.git
cd mpc-vst-plaits
./build.sh
```

Build needs Docker. Apply `add_chakra_page.py` before the upstream build command. Poloq reports Force architecture support, but upstream hardware verification is on MPC One 3.9.1; Chakra Force verification remains separately tracked.

## Current implementation

`chakra-plaits.patch` is the reproducible integration against upstream commit `b113c83`. It adds preset/manual fundamental control using a private voice owner, preserves the original parameter order by appending controls, and adds offline regression coverage. See `INSTALL_FORCE.md` and `HARDWARE_TEST_LOG.md` for exact evidence and remaining hardware checks.
