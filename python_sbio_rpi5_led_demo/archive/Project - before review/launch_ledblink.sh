crank_dir=/home/pi/crank
RUNTIME=$crank_dir/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj

APP=$crank_dir/apps/sb_led_app/LedBlinking.gapp

export SBENGINE=$RUNTIME/bin/sbengine
export SB_PLUGINS=$RUNTIME/plugins
export LD_LIBRARY_PATH=$RUNTIME/lib:$SB_PLUGINS
export XDG_RUNTIME_DIR=/run/user/0
#export DISPLAY=:0

#export OPTIONS='-vvvv -ogreio,channel=tcp://55555 -omtdev,device=/dev/input/event5 -orender_mgr,fullscreen,scale'
export OPTIONS='-vvvv -ogreio,channel=sbio_frontend -omtdev,device=/dev/input/event5 -orender_mgr,fullscreen,scale'

killall sbengine

$SBENGINE $OPTIONS $APP