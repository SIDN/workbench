#!/bin/sh

rm -rf output/servers/*

PYTHONPATH=libs
export PYTHONPATH
./tools/config_builders/config_builder.py
