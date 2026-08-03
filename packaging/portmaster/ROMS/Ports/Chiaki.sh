#!/bin/bash
# PORTMASTER: chiaki, Chiaki.sh

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
source $controlfolder/device_info.txt
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

GAMEDIR="/$directory/ports/chiaki"

> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

cd "$GAMEDIR"

export LD_LIBRARY_PATH="$GAMEDIR/libs:$LD_LIBRARY_PATH"
export LD_PRELOAD="$GAMEDIR/libs/libmaliegl.so"
export QT_PLUGIN_PATH="$GAMEDIR/libs/qt5/plugins"
export XKB_CONFIG_ROOT="$GAMEDIR/xkb"
export QT_XKB_CONFIG_ROOT="$GAMEDIR/xkb"
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_HIDECURSOR=0

# Detect the visible framebuffer size at runtime. virtual_size cannot be used
# for height because fbdev commonly reports two stacked buffers (for example
# 720x960 for a 720x480 panel).
SCREEN_WIDTH=""
SCREEN_HEIGHT=""
if [ -r /sys/class/graphics/fb0/modes ]; then
  SCREEN_MODE=$(head -n 1 /sys/class/graphics/fb0/modes)
  if [[ "$SCREEN_MODE" =~ ([0-9]+)x([0-9]+) ]]; then
    SCREEN_WIDTH="${BASH_REMATCH[1]}"
    SCREEN_HEIGHT="${BASH_REMATCH[2]}"
  fi
fi
if { [ -z "$SCREEN_WIDTH" ] || [ -z "$SCREEN_HEIGHT" ]; } && command -v fbset >/dev/null 2>&1; then
  read -r SCREEN_WIDTH SCREEN_HEIGHT < <(fbset -s 2>/dev/null | awk '/geometry/ { print $2, $3; exit }')
fi
SCREEN_WIDTH=${SCREEN_WIDTH:-640}
SCREEN_HEIGHT=${SCREEN_HEIGHT:-480}

export QT_QPA_EGLFS_WIDTH="$SCREEN_WIDTH"
export QT_QPA_EGLFS_HEIGHT="$SCREEN_HEIGHT"
export QT_QPA_EGLFS_DEPTH=16
export QT_OPENGL=es2
export QT_SCALE_FACTOR=1
export QT_AUTO_SCREEN_SCALE_FACTOR=0
export QT_QPA_GENERIC_PLUGINS=evdevkeyboard,evdevmouse
export QT_ENABLE_HIGHDPI_SCALING=0
# Describe the panel at a consistent 96 DPI so Qt point-sized text remains the
# same physical size across 720x480 and 640x480 displays.
export QT_QPA_EGLFS_PHYSICAL_WIDTH=$((SCREEN_WIDTH * 254 / 960))
export QT_QPA_EGLFS_PHYSICAL_HEIGHT=$((SCREEN_HEIGHT * 254 / 960))
export MALI_WINDOW_WIDTH="$SCREEN_WIDTH"
export MALI_WINDOW_HEIGHT="$SCREEN_HEIGHT"
echo "Detected display: ${SCREEN_WIDTH}x${SCREEN_HEIGHT}"
export SDL_AUDIODRIVER=alsa

# muOS RG34XXSP controller mapping.  The GUID is not present in PortMaster's
# primary SDL database, so SDL otherwise opens the pad without reliable analog
# axis assignments.  Keep a0-a3 as true analog sticks for Remote Play.
export SDL_GAMECONTROLLERCONFIG="19000000010000000100000000010000,ANBERNIC-keys,b:b3,a:b4,dpdown:h0.4,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b13,leftstick:b15,dpleft:h0.8,rightshoulder:b8,leftshoulder:b7,righttrigger:b14,rightstick:b12,dpright:h0.2,back:b9,start:b10,dpup:h0.1,y:b6,x:b5,guide:b11,platform:Linux,"

$GPTOKEYB "chiaki" -c "$GAMEDIR/chiaki.gptk" &
sleep 1

./chiaki
echo "chiaki exited: $?"

$ESUDO kill -9 $(pidof gptokeyb) 2>/dev/null
$ESUDO systemctl restart oga_events 2>/dev/null &
printf "\033c" > /dev/tty0
