#!/bin/bash

PYTHONPATH=./libs
export PYTHONPATH

ldns_sign_special_path=$(which ldns-sign-special)
if [[ ! -x "${ldns_sign_special_path}" ]] ; then
    echo 'File "ldns-sign-special" not found in PATH, aborting.'
    exit
fi


ldns_3597_path=$(which ldns-3597)
if [[ ! -x "${ldns_3597_path}" ]] ; then
    echo 'File "ldns-3597" not found in PATH, aborting.'
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

echo "Prettify..."
./prettify_zones.sh
