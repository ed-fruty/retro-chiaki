#!/bin/sh
# HELP: Launch the installed Retro Chiaki PortMaster port
# ICON: retro_chiaki
# GRID: Retro Chiaki

. /opt/muos/script/var/func.sh

APP_BIN="chiaki"
SETUP_APP "$APP_BIN" ""

# The application entry is only a native muOS shortcut. The executable and its
# bundled libraries remain in the PortMaster installation on the active ROM SD.
ROM_ROOT="$(GET_VAR "device" "storage/rom/mount")"
PORT_LAUNCHER="$ROM_ROOT/ROMS/Ports/Chiaki.sh"

if [ ! -x "$PORT_LAUNCHER" ]; then
	/opt/muos/frontend/muxmessage 0 "Retro Chiaki is not installed. Install the PortMaster package first, then try again."
	exit 1
fi

exec "$PORT_LAUNCHER"
