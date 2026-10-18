#!/bin/sh
# Headless screenshots of the UI states (no display needed). Handy while tweaking qml/Theme.qml.
#   tools/screenshots.sh [build-dir] [out-dir]
BUILD=${1:-build}; OUT=${2:-/tmp/zugfish-shots}; mkdir -p "$OUT"
export QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software
run() { n=$1; shift; "$BUILD/zugfish-qt" "$@" --screenshot "$OUT/$n.png" && echo "wrote $OUT/$n.png"; }
run welcome   --size 1360x860
run review    --sample opera    --review --ply 31 --size 1360x860
run blunder   --sample immortal --review --ply 35 --size 1360x860
run moves     --sample immortal --review --ply 22 --tab 1 --size 1360x860
run importing --sample opera    --dialog import   --size 1360x860
run settings  --sample opera    --dialog settings --size 1360x860
run small     --sample immortal --review --ply 40 --size 1040x680
