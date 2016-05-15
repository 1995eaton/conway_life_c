#!/usr/bin/env bash

FILE=$(find ./boards/rle -print | shuf -n1)

./game_of_life "$FILE"
