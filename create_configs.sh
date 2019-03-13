#!/bin/sh

# Create output dir if it does not exist:
mkdir -p output/servers

rm -rf output/servers/*

PYTHONPATH=libs
export PYTHONPATH
./tools/config_builders/config_builder.py

echo "Done."
echo "Please note: make sure you run this after create_zones.sh (if you have changed things there) and note before"
