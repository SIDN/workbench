#!/bin/bash

PYTHONPATH=./libs
export PYTHONPATH

if [[ ! -f ./ext/ldns-sign-special/ldns-sign-special ]] ; then
    echo 'File "ext/ldns-sign-special/ldns-sign-special" is not there, aborting.'
    exit
fi

if [[ ! -f ./ext/ldns-3597/ldns-3597 ]] ; then
    echo 'File "ext/ldns-3597/ldns-3597" is not there, aborting.'
    exit
fi

# Create output dir if it does not exist:
mkdir -p ./output

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

echo "Prettify..."
./prettify_zones.sh
