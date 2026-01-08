#!/bin/bash

# Build script for Gravity Strike using U++
# This script sets up the proper environment and builds the project

echo "Building Gravity Strike with U++ build system..."

# Set up environment variables for proper include paths
export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/usr/share/pkgconfig"
export CPLUS_INCLUDE_PATH="/usr/include/lua5.4:/usr/include/allegro5:$CPLUS_INCLUDE_PATH"
export C_INCLUDE_PATH="/usr/include/lua5.4:/usr/include/allegro5:$C_INCLUDE_PATH"

# Run U++ build command with explicit include paths
umk /home/sblo/Topside/uppsrc /home/sblo/GravityForce/GravityStrike.upp ~/.config/u++/theide/CLANG.bm \
    -v +out=/home/sblo/GravityForce/gslnx_umk_clang \
    +CFLAGS="-I/usr/include/lua5.4 -I/usr/include/allegro5"