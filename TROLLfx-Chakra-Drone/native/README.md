# Chakra Drone Native Plaits DSP — Force-first reference milestone

This milestone compiles the **actual upstream Plaits voice implementation** (including additive, wavetable and chord models) for a desktop reference renderer. It is intentionally not yet an Akai Force VST or a PC plugin. The result is an offline WAV for comparing future Force audio.

## Build in WSL / Linux

```bash
git clone -b project/trollfx-chakra-drone https://github.com/420troll/eurorack-attack.git
cd eurorack-attack/TROLLfx-Chakra-Drone/native
bash build_and_test.sh
```

Requires git, CMake, a C++17 compiler, and an internet connection for one-time upstream downloads.

The script fetches `pichenettes/eurorack` and its `stmlib` submodule into `native/vendor/`. The sources are **not** embedded in this repository; after downloading, pin their SHAs for reproducible packaging.

## Select engines

```bash
./build/chakra_native_render build/root_additive.wav 396 additive 8
./build/chakra_native_render build/heart_chord.wav 639 chord 8
./build/chakra_native_render build/crown_wavetable.wav 963 wavetable 8
```

Engine indices are the upstream Plaits Voice registration values **12 additive, 13 wavetable, 14 chord**. One Plaits voice supports continuous drone rendering at 48 kHz in 12-sample blocks. WAV output uses Plaits OUT as left and AUX as right; they are distinct signals, not a stereo spread. No automatic host resampling is implemented.

## Next tasks

- Run and fix any host compiler issues in WSL. This build is **not yet verified** on a compiling environment because upstream source downloads are unavailable in the current artifact environment.
- Verify Plaits license files and record precise git commits before any binary distribution.
- Build the allocation-free, fixed-size streaming render class with parameter smoothing and note/gate handling.
- Inspect the known-working Force TROLLfx VST project for ABI, MIDI input, plugin metadata and touchscreen conventions.
- ARM32 cross-compile, test with a single engine, then expand to four engines subject to CPU measurements.
- Expose all original Harmonics, Timbre, Morph, model selection, OUT and AUX levels as Force/Q-Link parameters.

**Licensing:** Upstream Plaits / stmlib code is copyright Emilie Gillet and other respective contributors, used under MIT conditions. Include upstream notices and the relevant licenses when distributing any derived binary.

## CPU / sound caution

Plaits' internal tuning uses firmware constants modeled on its hardware sample clock. It may differ slightly from ideal A440 at a true 48 kHz host rate. Validate exact Hz with offline spectrum tests rather than claiming perfect Hz tracking.

The chakra frequency names are artistic presets and are not medical claims.
