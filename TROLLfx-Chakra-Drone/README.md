# TROLLfx Chakra Drone — prototype v0.1

An experimental chakra-frequency, chord and wavetable drone instrument for future **Windows/Linux VST** and **Akai Force ARM32/OpenMPC** builds. This directory is isolated from the existing Eurorack Attack project.

## What works now
- Original C++17 stereo DSP prototype (not copied from Plaits or Cardinal)
- Seven selectable root frequencies: 396, 417, 528, 639, 741, 852, 963 Hz
- Four-voice major chord (root, major third, fifth, octave)
- Linear-interpolated wavetable with sine-to-harmonic morph
- Independent slow LFO sweep of table position
- 48 kHz 16-bit stereo offline WAV renderer (desktop test)

These are tuning presets for artistic use, not medically established treatments.

## Quick test on Windows WSL/Ubuntu
```sh
cd TROLLfx-Chakra-Drone
cmake -S . -B build
cmake --build build -j
./build/chakra_render heart.wav 4 12 0.5 0.15
```

Run `./build/chakra_render` to see the arguments. On Windows, use the CMake build tools or WSL; on Linux use g++ and CMake.

## Next milestones
1. Extract streaming DSP with note-on/off and no audio allocations in the callback.
2. Add voice stealing, selectable chord types/inversions, detune, envelope, filter, polyphony.
3. Compare the original Norns scripts with this prototype (source not yet imported).
4. Evaluate permitted use of Mutable Instruments Plaits and Cardinal/Surge-derived code; include upstream attribution/licences before importing.
5. Wrap the shared DSP for a desktop VST3 using a suitable plugin SDK.
6. Adapt separate ARM32 plugin wrapper and touchscreen/Q-Link metadata for the established Force/OpenMPC toolchain.
7. Test on actual hardware.

## Important
**Not yet a playable Force VST or PC VST.** This first Git commit provides a compilable sound prototype and a tracked development plan. No hardware install was attempted.

Copyright 2026 TROLLfx contributors. Original prototype source released under MIT, see LICENSE.
