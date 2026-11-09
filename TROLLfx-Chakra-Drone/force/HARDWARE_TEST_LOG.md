# TROLLfx Chakra Plaits — test log

## 2026-11-09 — WSL/offline

- Source integration: native upstream Plaits engine retained; 24 model enum verified.
- Shared VST host test: PASS, 154 parameters, stereo note audio, state chunk, Q-Link/option stepping, MIDI CC and NRPN.
- Chakra ASan/UBSan integration test: PASS. Verified VST instrument flag, zero audio inputs, two audio outputs, preset/manual controls, held native Plaits audio, retune, Chakra release with ordinary MIDI still active, and CC 123 clearing HOLD.
- ARM32 build: PASS. ELF32 ARM EABI5 hard-float VST2, sole exported plugin symbol `VSTPluginMain`, highest required glibc `2.27`, MD5 `e84f418c6b7d913a36a9e7401551ef42`.
- Generated registration metadata: category `Synth`, `isInstrument="1"`, `numInputs="0"`, `numOutputs="2"`, manufacturer `TROLLfx`, UID `TcPl` (`5463506c`).
- Shared wrapper host suite: PASS (154 parameters, state chunk, note audio, CC/NRPN and Q-Link stepping). Upstream `tests/controls.c` has one deterministic polyphony sweep expectation failure with the current `mpc-vst-plugins` main branch; the identical failure reproduces on untouched upstream Plaits, so it is not introduced by the Chakra patch. All other controls assertions pass.
- Release package: `Chakra-Plaits-1.0.0-mpc-armv7.zip` (8.7 MB), targeting MPC OS 3.x skins.
- No extra pitch-glide or subharmonic DSP was integrated.
- Force connection: BLOCKED. `ssh akai-force` resolved to `192.168.50.47` but returned `No route to host`; that IP was absent from the Windows LAN neighbor table, and none of the three visible LAN peers exposed SSH. No device files were changed.
- Hardware install, scan, browser visibility, audio/tuner observations, CPU, xruns and crash checks: NOT RUN. They must not be reported as passed.
