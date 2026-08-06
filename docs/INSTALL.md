# Installation and setup

## Install the release

Download `retro-chiaki-v0.3.1-portmaster-muos-h700.zip` and extract it to the root of the SD card that contains the `ROMS` and `ports` directories. Do not extract it inside `ROMS`, `ROMS/Ports` or `ports`; the archive already contains the complete directory layout.

After extraction, verify these relative paths on that card:

```text
ROMS/Ports/Chiaki.sh
ports/chiaki/chiaki
ports/chiaki/chiaki-cli
ports/chiaki/chiaki.gptk
ports/chiaki/libs/
ports/chiaki/xkb/
```

For a standard single-card setup this is normally `/mnt/mmc`; if your ROM storage is on SD2, extract the archive to the root of that card instead. Keep `Chiaki.sh`, `chiaki`, and `chiaki-cli` executable. PortMaster must already be installed because the launcher uses its controller helper and device integration.

## Register a PS4 or PS5

1. Connect the handheld and console to the same network.
2. Start Retro Chiaki and add the console manually if discovery does not find it.
3. Enter your Base64 PSN Account ID using the on-screen keyboard.
4. On PS5 open **Settings → System → Remote Play → Link Device**. On PS4 open **Settings → Remote Play Connection Settings → Add Device**.
5. Enter the displayed PIN and complete registration.

The upstream helper [`scripts/psn-account-id.py`](../scripts/psn-account-id.py) can obtain an Account ID through PlayStation OAuth. Never publish your registration data or Chiaki configuration file.

## Display mode

Recommended H700 stream settings are **540p**, **30 FPS**, and **H.264**. In **Settings → Stream Settings → Display Mode**, choose:

- **Original** for correct 16:9 proportions with letterboxing on the 3:2 display.
- **Stretch to Screen** to use the complete detected panel.

## Other devices and resolutions

This package is designed for compatible Allwinner H700 handhelds and detects 720×480 or 640×480 framebuffer geometry at runtime. It has currently been tested only on the Anbernic RG34XXSP (720×480) with muOS 2601 Jacaranda. Other H700 models may use different controller, audio, GPU or firmware integration and should be treated as unverified until reported by users.

## Planned Applications package

A separate native muOS Applications package is planned, but is not distributed with v0.3.1. For this release, start Retro Chiaki from **Ports → Chiaki**. Do not expect a `MUOS/application/Retro Chiaki` directory in the v0.3.1 archive.

## PortMaster and system dependencies

You do not need to install a PortMaster runtime such as `mono`, `love` or `weston`. Retro Chiaki ships its own Qt 5 libraries, Qt EGLFS and image plugins, XKB data and Mali EGL shim inside `ports/chiaki/`.

The tested muOS firmware provides SDL2, FFmpeg, OpenSSL, Opus, evdev/udev and the audio stack. PortMaster itself provides `gptokeyb`, controller discovery and the usual exit-hotkey lifecycle used by the launcher. Therefore removing PortMaster currently prevents Retro Chiaki from launching even though Qt itself is bundled with the package.

## Troubleshooting

- Restart Chiaki completely after replacing a release binary.
- Verify Wi-Fi and Remote Play are enabled on the console.
- **`Unknown ctrl error`:** if the log shows `InvalidSessionId`, `Ctrl did not receive session id`, or a Takion handshake timeout after a successful login, the console's Remote Play session may be temporarily busy or stuck. Wait and retry, close other Remote Play clients, then fully restart the PS4/PS5 if necessary. Do not immediately reinstall Chiaki or register the console again.
- If video works but audio does not, verify muOS PipeWire/WirePlumber are running and the internal speaker sink is selected.
- Session logs are normally under `/root/.local/share/Chiaki/Chiaki/log/`.
- Remove account IDs, IPs, registration keys and other private data before attaching logs to an issue.
