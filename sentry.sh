#!/bin/bash

ROOT=$(pwd)
LOG="$ROOT/log"

source ./bin/activate

PYTHONPATH=$ROOT/src/python:$ROOT/lib/python/serial-packets/src/python
export PYTHONPATH

cd "$ROOT/src/python/stream-video/" && {
  python main.py > "$LOG/stream-video.log" 2>&1 &
}

cd "$ROOT/src/python/website/" && {
  python main.py > "$LOG/website.log" 2>&1 &
}
