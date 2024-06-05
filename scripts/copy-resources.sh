#!/bin/bash

echo "[COPYING LATREN RESOURCES]"
SCRIPT_DIR=$(dirname -- "$(readlink -f -- "$BASH_SOURCE")")2
rm -fr res/.latren/
cp -TR $(dirname $SCRIPT_DIR)/res/.latren res/.latren/ && echo "[RESOURCES COPIED]"