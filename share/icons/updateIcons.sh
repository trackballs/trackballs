#!/bin/sh

# Construct png icons from svg base
all_sizes="16 22 24 32 36 48 64 72 96 128 192 256 384 512"
key_sizes="32 48 64 128 512"
inkscape trackballs.svg -e trackballs-512x512.png --export-dpi=69.1308 
for sz in ${key_sizes}; do
  convert trackballs-512x512.png -colorspace RGB \
          -resize ${sz} -colorspace sRGB \
          trackballs-${sz}x${sz}.png
done
