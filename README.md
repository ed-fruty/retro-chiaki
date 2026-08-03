# Retro Chiaki

Retro Chiaki is a handheld-focused fork of [Chiaki 2.2.0](https://git.sr.ht/~thestr4ng3r/chiaki), the free and open-source PS4/PS5 Remote Play client. This fork packages Chiaki as a ready-to-use PortMaster-style port for small ARM64 Linux handhelds running muOS.

> This project is not endorsed or certified by Sony Interactive Entertainment. You need your own PS4 or PS5 and PSN account.

## Who this is for

The current release is built and tested specifically for:

- Anbernic RG34XXSP
- Allwinner H700 / Mali GPU
- 720×480 (3:2) display
- muOS 2601 Jacaranda

This build is **not intended for 640×480 devices**. Those devices need a separate build and testing pass; a dedicated release may be added later. Do not assume that sharing the H700 SoC is sufficient for compatibility.

## What differs from upstream Chiaki

- eglfs/Mali framebuffer support through a bundled EGL compatibility shim.
- UI resized and made scrollable for a 720×480 screen.
- On-screen keyboard for PSN Account ID and registration PIN entry.
- Working OpenGL ES shaders and video rendering on the H700 Mali stack.
- `Keep 16:9` and `Stretch to Screen` display modes.
- SDL controller mapping tailored for the RG34XXSP/muOS input device.
- Analog sticks, triggers, shoulder buttons and L3/R3 Remote Play input fixes.
- Select is mapped to the DualSense touchpad click.
- Select + Start exits the application, matching the usual muOS/PortMaster convention.
- gptokeyb is used for the connection UI and paused while streaming, preventing mouse and duplicate controller events.
- Cursor is forcibly hidden while streaming.
- Stream audio uses SDL/ALSA, which routes correctly through muOS PipeWire.
- Bundled Qt 5 runtime and dependencies: no package installation on the handheld is required.

## Controls

Before streaming, the left stick controls the mouse; D-pad and face buttons navigate the desktop-style interface. During streaming, Chiaki uses the controller directly through SDL.

| Handheld input | Remote Play action |
|---|---|
| A/B/X/Y | Cross/Circle/Square/Triangle |
| D-pad | D-pad |
| L1/R1, L2/R2 | PS shoulder buttons and triggers |
| Stick clicks | L3/R3 |
| Start | Options |
| Select | Touchpad click |
| Select + Start | Exit Retro Chiaki |

## Installation

1. Download `retro-chiaki-v0.1.0-portmaster-muos-rg34xxsp-720x480.zip` from [Releases](https://github.com/ed-fruty/retro-chiaki/releases).
2. Extract the archive to the root of the SD card containing your muOS ROMs.
3. Confirm that these paths exist:
   - `/mnt/mmc/ports/chiaki/chiaki`
   - `/mnt/mmc/ROMS/Ports/Chiaki.sh`
4. Refresh Ports or reboot the handheld, then launch **Chiaki** from Ports.
5. Add/register your console and start streaming.

See [the detailed installation and setup guide](docs/INSTALL.md) for SD2 setups, registration, resolution changes and troubleshooting.

## Feedback

This port was developed and tested on real RG34XXSP hardware. Please [open an issue](https://github.com/ed-fruty/retro-chiaki/issues) with:

- device model and muOS version;
- what works and what does not;
- a photo/video when the issue is visual;
- the Chiaki session log when relevant, after removing any private information.

Feature requests and control-layout suggestions are welcome too.

## Building

The ARM64 cross-build files and EGL shim source are in [`packaging/build`](packaging/build). See [BUILDING.md](docs/BUILDING.md).

## Upstream and license

Retro Chiaki is based on Chiaki by Florian Märkl and retains its GNU AGPL v3 license with the upstream OpenSSL linking exception. See [COPYING](COPYING) and [LICENSES](LICENSES). The original project and its contributors deserve credit for the Remote Play implementation; this fork focuses on retro-handheld integration and packaging.
