#!/bin/bash

ROOT=$(pwd)

echo Installing Sentry service...
sudo systemctl daemon-reload
sudo systemctl enable "$ROOT/sentry.service"
echo

echo To start:      systemctl start sentry.service
echo To get status: systemctl status sentry.service
