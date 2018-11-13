#!/bin/sh

PYTHONPATH=./libs
export PYTHONPATH
echo "Running static zones generator" &&\
./tools/generators/static_zones.py &&\
echo "Running bad-dnssec zones generator" &&\
./tools/generators/bad_dnssec.py &&\
echo "Running delegations zones generator" &&\
./tools/generators/delegations.py &&\
echo "Running zone completer" &&\
./tools/completer/completer.py &&\
echo "Running zone signer" &&\
./tools/signer/signer.py &&\
echo "Running finalizer" &&\
./tools/finalizer/finalizer.py
