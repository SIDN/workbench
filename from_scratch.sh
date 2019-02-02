#!/bin/bash
#
#The right order from scratch would be something like:
#
./make_clean.sh
# this order
./create_zones.sh
./create_configs.sh
# Not the other way around!
./create_site.py
sudo ./sitepublish.sh
sudo ./publish_zones.sh

./make_base_config.sh
