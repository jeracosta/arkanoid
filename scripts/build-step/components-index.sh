#!/usr/bin/env bash
set -euo pipefail

SRC_DIR="${1:?source components dir required}"
OUT_DIR="${2:?output dir required}"

mkdir -p "$OUT_DIR"

DEF_OUT="$OUT_DIR/index.def"
HDR_OUT="$OUT_DIR/index.hpp"

: >"$DEF_OUT"
: >"$HDR_OUT"

shopt -s nullglob

files=("$SRC_DIR"/*.hpp)
count=${#files[@]}

for i in "${!files[@]}"; do
  f="${files[$i]}"
  name=$(basename "$f" .hpp)

  macro="X"
  if ((i == count - 1)); then
    macro="X_LAST"
  fi

  printf '%s(%s)\n' "$macro" "${name^}" >>"$DEF_OUT"
  printf '#include "oh-my-engine/entity-component-system/components/%s.hpp"\n' "$name" >>"$HDR_OUT"
done
