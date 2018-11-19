#!/bin/sh

OUT="$1"
shift
cat $* > "$OUT"
