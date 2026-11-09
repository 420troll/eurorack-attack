# TROLLfx Chakra Drone — Force-first architecture v0.3

## Product target

**Primary target:** Akai Force, OpenMPC / MockbaMod 4.51, firmware 3.10.0.49, Linux armv7l and 48 kHz stereo instrument plug-in.

**Secondary target:** native PC DSP harness / reference render application. Not a separate user-facing product. Pixel HTML prototype is historical and is not the reference audio engine.

## Core choice

Port **original Mutable Instruments Plaits C++ DSP**, not browser-oscillator approximations. Upstream: https://github.com/pichenettes/eurorack/tree/master/plaits . Preserve upstream copyright and MIT license notices for imported Plaits/stmlib code. Pin upstream commit prior to integration. No imported Plaits files are included yet.

## Voice architecture

- Four independently selectable Plaits engines as an initial target; actual voice count determined by Force CPU benchmarks.
- Per voice: engine/model selection; HARMONICS, TIMBRE, MORPH, frequency offset, level and pan; both OUT and AUX accessible.
- Per chord: major, minor, sus2, sus4, fifths and custom inversion/spread as later milestones.
- The seven named Chakra frequencies (396, 417, 528, 639, 741, 852, 963 Hz) are artistic root presets. MIDI-note pitch and exact Hz drone modes must be distinct so MIDI tracking does not silently override exact frequency.
- Even/odd harmonic control where engine supports it; an optional post-synthesis harmonic layer is a distinct feature rather than misrepresenting Plaits' native controls.
- Root-tracking sub registers at /2, /3, /4 and /8. Budget CPU cost and low-frequency playback quality. The divide-by-three register relates to a fifth after octave translation, but is not an equal-tempered fifth.
- Main + aux mix, gain smoothing, modulation routing, envelopes and stereo effects later. No safety/brightness controls from the lighting projects belong here.

## Force integration

- Native ARM32 .so instrument with real-time-safe render callback. No allocations, file I/O, locks or GUI actions on audio thread.
- Keep DSP independent from the wrapper. Use our existing TROLLfx/OpenMPC toolchain and plugin metadata only after reading the known-working wrapper code.
- On-screen tabs: ENGINE, CHAKRA STACK, MODULATION. Touchscreen-first controls and Q-Link mapping.
- Persist plugin state in Force projects and support programmatic recall.
- Verify host sample rate, block sizes, note handling and VST ABI against known-working Force plugins; do not assume desktop VST3 compatibility.

## PC emulation / test harness

- Build the **same DSP files** into a Linux/Windows executable.
- MIDI note/gate and fixed Hz preset tests; WAV reference rendering.
- Automated per-engine snapshots, max level / clipping / NaN detection, CPU timing, state save/restore.
- PC harness is not a hardware emulator. Force GUI/host behavior must be checked on Force.

## Integration milestones

1. Verify Force instrument wrapper API using the existing working TROLLfx plugin project.
2. Vendor pinned original Plaits source and dependencies with licenses and commit references.
3. Compile one native Plaits engine through a portable, allocation-free C++ DSP facade.
4. Build a PC reference renderer for matching 48 kHz waveforms.
5. Build ARM32 instrument .so with MIDI, note-off, host state and one engine.
6. Test on Force and measure CPU. Expand to four independently parameterized engines and chord/sub layers.
7. Expose touchscreen controls + Q-Links and add preset persistence and modulation.

## Status

Architecture defined, **not yet compiled**, **no original Plaits DSP imported**, no Force build or hardware install performed. The pre-existing mobile version uses a separate Web Audio implementation and is not authoritative for Force synthesis.
