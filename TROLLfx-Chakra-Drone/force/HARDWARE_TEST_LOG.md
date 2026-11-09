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

## 2026-11-09 — Akai Force hardware deployment

- Device: Akai Force firmware `3.10.0.49`, Linux `armv7l`, MockbaMod `4.51`; SSH alias `akai-force` at its existing address.
- Storage: `/media/662522` had 27.9 GB free. `/media/662522/Synths` was already listed in `SynthContentLocations`.
- Backup: `/data/Settings/MPC/MPC.settings.chakra-plaits-backup-20261109T0707Z`, also copied off-device to `/tmp/MPC.settings.chakra-plaits-backup-20261109T0707Z`; both SHA-256 `da27a19d16e6572025bcbeab98eef38861de425971eb1ea450d4d6eb83242e96`.
- Install: PASS. Added the separate folder `/media/662522/Synths/TROLLfx - VST - Chakra Plaits`; no prior target or `TcPl` entry existed. All 333 packaged files matched the local tree checksum `36558547a253154056c432af2327adb3b0a2e09106f9ba690b7d3533a2fad28b`. Installed `.so` SHA-256: `6336bb4fd9617e1d25be24164d94ad19f8b73c79914f49d560eefe909a1b539e`.
- MockbaMod scanner: PASS, exit 0. It rebuilt the plugin list from 581 to 582 entries, restarted `acvs`, and registered exactly one `TcPl` entry as category `Synth`, `isInstrument="1"`, `numInputs="0"`, `numOutputs="2"`, at the separate Chakra path. `acvs` returned active and `/usr/bin/MPC` relaunched.
- Native plugin benchmark on the Force: the actual ARM32 `.so` loaded in 1.1 ms and produced nonzero output from notes. p99 CPU: idle 0.7%, 1 voice 2.4%, 4 voices 6.7%, 8 voices 6.6%, release tail 11.2%, trigger 12.3%, all-parameter sweep 41.1%. The benchmark verdict is **FAIL** because the deliberately aggressive every-block parameter sweep exceeds its threshold; the worst measured block was 48.0% of one audio period. No audible result is inferred from sample peaks.
- Native ARM integration probe run directly on the Force: PASS. It verified the VST instrument/stereo flags, all 24 Plaits model options, preset/manual fundamental parameters, Heart 639 trigger producing stereo samples, held-voice manual-Hz retune, external MIDI surviving Chakra release, CC 123 clearing HOLD, and release decay.
- Post-scan diagnostics: `acvs` remained active. The journal contained no Chakra error, `dlopen`/undefined-symbol failure, segfault, core dump, xrun, underrun, OOM or kernel I/O error after the scan. Startup did report unrelated existing MockbaMagic firmware-support and Sentry-backend warnings.
- Not physically verified: plugin-browser visibility on the touchscreen, audible playback, tuner accuracy of preset/manual Hz, touch controls, physical Q-Links, pads, sustain/pitch-bend/aftertouch gestures, or project save/reload. These require the hands-on procedure in `INSTALL_FORCE.md` and are not reported as passed.
