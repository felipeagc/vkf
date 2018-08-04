#!/bin/sh

mkdir -p "${MESON_BUILD_ROOT}/shaders"

for i in $*; do
	glslangValidator -V $i -o "${MESON_BUILD_ROOT}/shaders/$i.spv"
done
