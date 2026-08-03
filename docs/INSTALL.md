# Installation and setup

## Install the release

Extract the release ZIP to the root of the SD card that contains the `ROMS` and `ports` directories. The archive already contains the correct directory layout.

On a standard single-card muOS setup the files resolve to:

```text
/mnt/mmc/ROMS/Ports/Chiaki.sh
/mnt/mmc/ports/chiaki/
```

If Ports are stored on SD2, copy the two archive directories to the equivalent ROM and port paths used by that card. Make sure `Chiaki.sh`, `chiaki`, and `chiaki-cli` remain executable.

## Register a PS4 or PS5

1. Connect the handheld and console to the same network.
2. Start Retro Chiaki and add the console manually if discovery does not find it.
3. Enter your Base64 PSN Account ID using the on-screen keyboard.
4. On PS5 open **Settings → System → Remote Play → Link Device**. On PS4 open **Settings → Remote Play Connection Settings → Add Device**.
5. Enter the displayed PIN and complete registration.

The upstream helper [`scripts/psn-account-id.py`](../scripts/psn-account-id.py) can obtain an Account ID through PlayStation OAuth. Never publish your registration data or Chiaki configuration file.

## Display mode

In **Settings → Stream Settings → Display Mode**, choose:

- **Keep 16:9** for correct proportions with letterboxing on the 3:2 display.
- **Stretch to Screen** to use the complete 720×480 panel.

## Other devices and resolutions

This package is an RG34XXSP 720×480 build. It is not a generic H700 package, and editing four resolution environment variables is not considered sufficient for a supported installation. Devices with 640×480 displays will receive a separately built and tested package later.

## Troubleshooting

- Restart Chiaki completely after replacing a release binary.
- Verify Wi-Fi and Remote Play are enabled on the console.
- If video works but audio does not, verify muOS PipeWire/WirePlumber are running and the internal speaker sink is selected.
- Session logs are normally under `/root/.local/share/Chiaki/Chiaki/log/`.
- Remove account IDs, IPs, registration keys and other private data before attaching logs to an issue.
