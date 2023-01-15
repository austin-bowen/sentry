#!/bin/bash

ROOT=$(pwd)

source ./bin/activate

PYTHONPATH=$ROOT/src/python:$ROOT/lib/python/serial-packets/src/python
export PYTHONPATH

cd "$ROOT/src/python/stream-video/" && python main.py &

cd "$ROOT/src/python/website/" && python main.py &
