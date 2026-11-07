# SONARIUM — Marine Test Synthesizer

A steampunk marine laboratory synthesizer for Windows VST3 and standalone, developed by ThomC.

## Three sound engines
**BASS:** sine sub-basses, driven oscillators, muted vintage bass tones, and plucked electric-string colour. Controls: SUB, DRIVE, MUTE, ELECTRIC, TONE.

**PAD:** slowly evolving detuned pads, radio static, drifting reception, dusty filtering, and atmospheric bloom. Controls: STATIC, DRIFT, DUST, BLOOM, TEXTURE.

**SONAR:** marine pings, resonant keys, glassy test signals, and Karplus-Strong guitar-like plucks. Controls: PING, DEPTH, ECHO, PLUCK, SEA, SIGNAL.

## Effects
Three switchable tape flavours: WARM, WORN, WARBLE. Each shares TIME, FEEDBACK and MIX controls. Parallel independent ROOM, BLOOM and PLATE reverbs.

## Real animated UI
The editor is **painted using JUCE graphics, not a flattened PNG**: brass knobs rotate, the vintage CRT radar sweeps, tape reels spin, two VU meters respond, and a playable MIDI piano and brass-panel selectors are interactive. The original nautical artwork is a visual design reference and has been interpreted into code.

## Playing and presets
MIDI note input, mod wheel (CC1), sustain (CC64), pitch bend, velocity, adjustable 1–16 polyphony, glide and master output. Nine factory patches; arrows change preset, SAVE/LOAD manage native .sonarium preset files. The DAW stores and restores plugin settings as usual.

## Build
Use GitHub Actions > **Windows VST3 + Standalone**. If the workflow is green, download the **Sonarium-Windows-VST3** artifact, unzip, and copy the intact .vst3 plugin bundle to the Windows VST3 folder, usually C:\Program Files\Common Files\VST3.

Requirements for local builds: Visual Studio 2022 with C++ workload, CMake 3.22+, Windows SDK and Internet access to fetch JUCE.

```sh
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target sonarium_dsp_stress
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target Sonarium_VST3 Sonarium_Standalone
```

JUCE 8.0.16-security is used. The synthesizer uses no external sound libraries, commercial samples, or static UI PNG assets.

## Quality and status
DSP stress tests cover three sound engines, 12 note scenarios, drive/tape automation, non-finite and unbounded sample checks. No filesystem or blocking operations occur in the audio callback.

**Experimental development build.** It is not yet a professionally auditioned, release-ready instrument. Please monitor at conservative levels and report any distortion spikes, clicks, or DAW crashes with reproduction steps.

© 2026 ThomC. Check JUCE upstream licensing obligations before commercial distribution.
