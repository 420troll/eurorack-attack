# TROLLfx Chakra Page for poloq MPC Plaits

Upstream: https://github.com/poloq-instruments/mpc-vst-plaits

The upstream plugin already exposes real Plaits DSP, 24 models, polyphony and touchscreen/Q-Links. Add a seventh CHAKRA tab **without replacing existing tabs**.

## Trigger design

- Popup select: Root 396 Hz, Sacral 417 Hz, Solar 528 Hz, Heart 639 Hz, Throat 741 Hz, Third Eye 852 Hz, Crown 963 Hz.
- Touch **TRIGGER** button: note-on at selected chakra frequency; touch again: note-off.
- Trigger via existing `ppf_engine_t::note_on` and `note_off` (no outbound MIDI bridge necessary).
- Normal Force MIDI/pads and sequencer continue to work.
- Keep HARMONICS, TIMBRE, MORPH, OUT/AUX, and original model selector fully accessible.
- For base pitch, trigger MIDI note 69 with a plugin pitch offset of `12*log2(chakra_hz / 440)` semitones, using the existing `pitch` parameter. The upstream engine is calibrated to 44.1 kHz; verify actual output frequency by spectral test.
- On changing selected chakra while trigger active, send note-off before changing the pitch, then retrigger note 69.
- On plugin close, transport reset, or all-notes-off, clear touch gate. Persist state without leaving stuck notes.
- **Important:** changing the global pitch parameter will also affect external MIDI notes. A proper dedicated per-trigger tuning path is preferred for the final implementation, so touchscreen and normal MIDI can coexist.
- **Important:** a toggle is a held note, not a momentary tap. Add a dedicated OFF and ALL NOTES OFF control before release.

## Sound plans

Phase 1: one selected Plaits engine with chakra trigger and native controls.
Phase 2: chord intervals / inversions using native polyphony, routing 4 note-ons and matching note-offs. Budget Plaits 8 voices total.
Phase 3: independent additive EVEN/ODD partial bank + root/2, /3, /4, /8 subregisters. These need separate synth voices; stock HARMONICS does **not** provide independent even/odd level control.

## Minimum native source edit points

- `src/module.json`: append `chakra_root` enum and `chakra_gate` on/off parameter to `chain_params`, preserving existing parameter order.
- `vst/layout.conf`: add a `[tab CHAKRA]` with a root selection popup, a trigger toggle, Harmonics/Timbre/Morph and OUT/AUX knobs; add page Q-Link assignments.
- `src/dsp/freak_plugin.cpp`: maintain per-instance chakra root and gate; intercept these two parameter keys in `set_param`, report values in `get_param`, and invoke native `inst->engine.note_on(69, velocity)` / `note_off(69)`.
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

Build needs Docker. This is the upstream build command, **not yet a Chakra build**. Poloq reports support for Force by architecture but only verifies hardware on MPC One 3.9.1.

## Status

Architecture and integration points have been verified against upstream sources. No Chakra fork, compiled binary, or Force hardware test is claimed in this document.
