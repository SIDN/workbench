#!/bin/bash

PYTHONPATH=./libs
export PYTHONPATH

if [ ! command -v ldns-sign-special &> /dev/null ]; then
    echo "ldns-sign-special could not be found, aborting"
    exit
fi

if [ ! command -v ldns-3597 &> /dev/null ]; then
    echo "ldns-3597 could not be found, aborting"
    exit
fi


# Create output dir if it does not exist:
mkdir -p output/final
mkdir -p output/keys
mkdir -p output/signed
mkdir -p output/uncompleted
mkdir -p output/unsigned
mkdir -p output/zone_db

# Do or don't? You pick...
rm -rf output/final/*
rm -rf output/keys/*
rm -rf output/signed/*
rm -rf output/uncompleted/*
rm -rf output/unsigned/*
rm -rf output/zone_db/*
# (removing existing stuff, just to make sure everything is truly regenerated
# Skipping output/server, since it contains configs

echo "Running static zones generator" &&\
./tools/generators/static_zones.py &&\
echo "Running bad-dnssec zones and types zones generator" &&\
./tools/generators/bad_dnssec.py &&\
echo "Running delegations zones generator" &&\
./tools/generators/delegations.py &&\
echo "Running zone completer" &&\
./tools/completer/completer.py &&\
echo "Running zone signer" &&\
./tools/signer/signer.py &&\
echo "Running finalizer" &&\
./tools/finalizer/finalizer.py


if [ ! command -v named-compilezone &> /dev/null ]; then
    echo "named-compilezone could not be found, not prettifying zone files"
else
    echo "Prettify..."
    ./prettify_zones.sh
fi
