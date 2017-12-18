## Build Guide

### Introduction

If instead of using [the precompiled plugin binaries](https://github.com/nurupo/vlc-pause-click-plugin/releases) you want to build them yourself, you can follow these instructions on building the plugin.

These instructions describe out of the tree building of the plugin, since this is the easiest way to build it.

### Prerequisites

The prerequisite for building is having the VLC sdk corresponding to the version of VLC you are building the plugin for.

Specifically, you will need:
  - `vlccore` library.
  - Pkg-config file `vlc-plugin.pc`.
  - Set of `libvlccore` headers. `vlc_filter.h` is one of those plugin headers.
  - Set of `libvlc` headers. `libvlc_version.h` is one of those headers.

The way you get the VLC sdk varies from platform to platform.

On Windows there is a `sdk` directory included with your VLC installation that includes everything you need.
Note that [there is a Dockerfile](./docker) that fully automates building for Windows described in these instructions.
That Docker container is what I use to produce the Windows plugin binaries.

On macOS `vlccore` library and `libvlc` headers are included in the VLC `.dmg` file, but there are `libvlccore` headers and `vlc-plugin.pc` in there.
You will have to get the missing pieces from the `sdk` directory of Windows VLC `.zip` archive of the same version as your `.dmg` VLC.

On Linux you generally just install the packages that contain the mentioned files and you are good to go.
If you have manually built VLC instead of getting it pre-built from the packages, you should probably already have the VLC libraries and headers installed.

If you want to build the plugin for Debian Linux or its derivatives, there are simpler instructions provided in [README](/README.md).

### Fixing pkg-config

Make sure `pkg-config --cflags vlc-plugin` and `pkg-config --libs vlc-plugin` commands print the correct include and library paths.
If they do, you should skip this part and go to building.

Here is an example of how you would fix this for a Windows build (assuming you are on Linux and have Windows VLC sdk)

```
cd sdk
# Set the prefix path to $PWD in vlc-plugin.pc
# It's hardcoded to the path VLC developer installed his VLC to and this is not the path you want
sed -i "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
# Make pkg-config aware of vlc-plugin.pc
export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
```

After this `pkg-config --cflags vlc-plugin` and `pkg-config --libs vlc-plugin` commands should print the correct paths to the `vlccore` library and the headers.

### Building

Once you get pkg-config to find the dependencies, the building is as easy as

```sh
make CC=your-compiler LD=your-linked OS=your-target-os
```

For example, to build 32-bit Windows binaries you would do

```sh
make CC=i686-w64-mingw32-gcc LD=i686-w64-mingw32-ld OS=Windows
```

Makefile defaults to Linux, so for Linux

```sh
make
```

would be enough.

For macOS

```
make OS=macOS
```

should be enough.

`OS` is used to set the correct shared library extension.

After running make you should see `libpause_click_plugin.[dll|so|dylib]` generated, which is the plugin binary ready for use.
You might want to strip it to shave some kilobytes off.
