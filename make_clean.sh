#!/bin/sh
#
# Clean it up to an original state
# (quick-n-dirty, might need some extra checks and improvements)
#

rm ./ext/ldns-sign-special/ldns-sign-special
rm ./ext/ldns-3597/ldns-3597

rm -rf ./output
rm -rf ./site

find . -type f -name "*~" | xargs rm
