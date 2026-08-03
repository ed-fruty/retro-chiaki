# Changelog

## Unreleased

- Expanded project positioning to compatible Allwinner H700 handhelds while keeping RG34XXSP with muOS as the only verified configuration.
- Added runtime framebuffer detection for a single 720×480/640×480 ARM64 build.
- Constrained the main window, settings, registration and on-screen keyboard to the detected display size.
- Made EGLFS, Mali rendering and stream geometry use the detected display instead of fixed 720×480 values.

## v0.1.1

- Added clear PS4, PS4 Pro, PS5, PS5 Digital Edition and PS5 Pro Remote Play positioning.
- Added Anbernic and KNULLI discoverability, with KNULLI clearly marked as untested.
- Changed fresh-install defaults to 540p, 30 FPS and H.264 for RG34XXSP.
- Renamed the `Keep 16:9` display mode to `Original`.

## v0.1.0

First public RG34XXSP 720×480 muOS/PortMaster release based on Chiaki 2.2.0.

- Added H700/Mali eglfs video support.
- Adapted the interface to 720×480 handheld displays.
- Added on-screen registration input.
- Added 16:9 and stretched display modes.
- Added RG34XXSP controller integration and touchpad/exit mappings.
- Added SDL/ALSA audio output for muOS PipeWire.
- Fixed stream cursor and duplicate gptokeyb input.
