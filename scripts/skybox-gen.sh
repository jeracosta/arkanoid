#!/bin/sh

set -eu

input="${1:-}"
output_dir="${2:-}"
prefix="${3:-}"

if [ -z "$input" ]; then
  printf 'Usage: %s <cross-image> [output-dir] [prefix]\n' "$0" >&2
  printf 'Example: %s assets/textures/skybox/day.png assets/textures/skybox day_\n' "$0" >&2
  exit 1
fi

if ! command -v magick >/dev/null 2>&1; then
  printf 'Error: ImageMagick is required. Install it and ensure `magick` is on PATH.\n' >&2
  exit 1
fi

if [ ! -f "$input" ]; then
  printf 'Error: input image not found: %s\n' "$input" >&2
  exit 1
fi

if [ -z "$output_dir" ]; then
  output_dir=$(dirname "$input")
fi

mkdir -p "$output_dir"

tmp_base="$output_dir/.skybox-gen-tile"

rm -f "$tmp_base"_*.png

magick "$input" -crop 4x3@ +repage -write "$tmp_base"_%02d.png null:

mv "$tmp_base"_01.png "$output_dir/${prefix}top.png"
mv "$tmp_base"_04.png "$output_dir/${prefix}left.png"
mv "$tmp_base"_05.png "$output_dir/${prefix}front.png"
mv "$tmp_base"_06.png "$output_dir/${prefix}right.png"
mv "$tmp_base"_07.png "$output_dir/${prefix}back.png"
mv "$tmp_base"_09.png "$output_dir/${prefix}bottom.png"

rm -f \
  "$tmp_base"_00.png \
  "$tmp_base"_02.png \
  "$tmp_base"_03.png \
  "$tmp_base"_08.png \
  "$tmp_base"_10.png \
  "$tmp_base"_11.png

printf 'Generated skybox faces in %s\n' "$output_dir"
printf 'Convention used: top / left front right back / bottom\n'
