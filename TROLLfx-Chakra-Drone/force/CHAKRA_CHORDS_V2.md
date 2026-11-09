# Chakra Chords SUB V2 — experimental build

Base: `feature/chakra-sub-engine-v1` (TcSb). This is a **new plugin**, not an update in place.

## Design

- CHAKRA tab: Chakra preset/manual Hz, HOLD, Plaits CHORDS HARMONICS/TIMBRE/MORPH/AUX.
- SUB tab: sine oscillator ON, division 1–10, pitch slew, wavefold, sub mix, harmonic integer lock, phase reset.
- No MODEL popup or CHAKRA MODEL Q-Link. DSP is fixed at Plaits model 6 (CHORDS), even for restored host preset values.
- No shared CHAKRA/SUB Q-Link parameter keys. SUB MIDI mapping must be regression-tested; isolating tabs alone does **not** guarantee no host MIDI mapping collisions.
- Existing parameter IDs are retained from TcSb; separate VST UID `TcC2`.
- Original TcPl and TcSb remain installed, untouched.

## Reproducible build (in WSL, Codex)

```bash
# Create a fresh disposable worktree of poloq mpc-vst-plaits at b113c83.
# Do not reuse a worktree which already has these transforms.
python3 TROLLfx-Chakra-Drone/force/add_chakra_page.py /path/to/clean-plaits
python3 TROLLfx-Chakra-Drone/force/apply_sub_engine.py /path/to/clean-plaits
python3 TROLLfx-Chakra-Drone/force/apply_chakra_chords_v2.py /path/to/clean-plaits
# From within /path/to/clean-plaits:
MPC_VST=/mnt/c/Users/Gamer/Desktop/akaiVibe/mpc-vst-plugins bash build.sh
```

## Required validation before Force deployment

1. Run Python compile and layout/skin generation; inspect preview for clipping/overlay warnings.
2. Run existing DSP, host and original Chakra regression suites. Update expectations for fixed Chords model (6), unique UID TcC2 and seven SUB params.
3. Test setting model via host automation/preset restore cannot escape Chords.
4. Test each CHAKRA Q-Link never moves sub_mix, sub_fold or sub_division; each SUB Q-Link never moves timbre or morph.
5. Run MIDI Learn and repeated pad assignments without hanging. Test note on/off, HOLD, sustain, fallback and CC120/123.
6. Build and inspect ARMv7 ELF and export, calculate hash. Test performance and check all 24 models are no longer exposed through CHAKRA UI (other upstream pages may still contain model switching; DSP lock must override these).
7. Back up Force settings, stage as separate `TROLLfx - VST - Chakra Chords SUB V2` after validation. Preserve original TcPl and TcSb. Scan/restart only with user authorization.

**Status:** V2 source overlay committed, NOT built or installed. No hardware or touchscreen tests have been performed for V2.
