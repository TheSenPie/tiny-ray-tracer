#!/bin/bash

set -e

echo "Downloading library dependencies for samples..."
git submodule init
git submodule update
echo "Setting up folder for build files..."
mkdir -p out
cd out
if  [ "`uname -s`" = "Darwin" ]; then
  TRT_GENERATOR="-GXcode"
else
  TRT_GENERATOR=
fi

cmake -DASSIMP_INSTALL=OFF .. ${TRT_GENERATOR} $@
echo Finished successfully!
