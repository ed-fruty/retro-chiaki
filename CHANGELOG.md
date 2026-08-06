# Changelog

## v0.3.1

- Chiaki.sh now symlinks the device's own Mali GLES driver into place at launch, instead of relying on a static copy bundled at build time. More portable across H700 units with different driver builds, and no longer a blocker for reproducing a release from source.
- Added automated release CI: pushing a version tag now cross-compiles, packages and publishes the release automatically.

## v0.3.0

- Made the Controller Button Mapping settings actually remap the real gamepad; the previous "Key Settings" list only drove an unused keyboard-as-controller fallback and had no effect on real streaming input.
- Added L2/R2 to the remappable list, freely interchangeable with any other button (this handheld's triggers are plain digital switches under the hood, so there's no analog fidelity to protect).
- Replaced the settings dialog's press-to-capture flow, which could only work during an active stream, with a per-button dropdown that works anywhere in the app.
- Grouped L1/L2/L3 and R1/R2/R3 into consistent columns, shortened labels, and added a horizontal-scroll fallback so the dialog fits small screens.

## v0.2.0

- Added original Retro Chiaki artwork and a 320×240 muOS/PortMaster preview.
- Documented bundled Qt components, system library expectations and the remaining PortMaster/gptokeyb dependency.
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
