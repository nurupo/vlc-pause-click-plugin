## Build Guide

### Introduction

If instead of using [the precompiled plugin binaries](https://github.com/nurupo/vlc-pause-click-plugin/releases) you want to build them yourself, you can follow these instructions on building the plugin.

Note that [there is a Dockerfile](./docker) that fully automates building for Windows described in these instructions.
This is what I use to produce the Windows plugin binaries.

Also, if you want to build the plugin for Debian Linux or its derivatives, you can make your life easier if you follow the Debian installation instructions provided in [README](/README.md) instead of following this building guide.

### Building

What we want to do is to build VLC with the plugin integrated into its source tree and build system, so that the plugin would be built together with VLC.

Git clone VLC:

```sh
git clone git://git.videolan.org/vlc.git
```

Checkout the git tag that matches version of VLC that you want to build the plugin for. You can use `git tag` to get a list of all tags available. For example, for 2.2.0 you would do:

```sh
cd vlc
git checkout 2.2.0-git
```

Add the plugin into the VLC build system by copying `pause_click.c`, `vlc_interface-2.1.0-git.h` and `vlc_interface-2.2.0-git.h` into `modules/video_filter/`.

If you are building VLC 2.1.0, add

```
libpause_click_plugin_la_SOURCES = pause_click.c
libpause_click_plugin_la_CFLAGS = $(AM_CFLAGS)
libpause_click_plugin_la_LIBADD = $(AM_LIBADD)
libvlc_LTLIBRARIES += libpause_click_plugin.la
```

to the end of `modules/video_filter/Modules.am` file.

If you are building VLC 2.2.0, add

```
libpause_click_plugin_la_SOURCES = pause_click.c
libpause_click_plugin_la_CFLAGS = $(AM_CFLAGS)
libpause_click_plugin_la_LIBADD = $(AM_LIBADD)
video_filter_LTLIBRARIES += libpause_click_plugin.la
```

to the end of `modules/video_filter/Modules.am` file for VLC 2.2.0.

Build VLC the way you would usually do so.
There should be some instructions on VLC's wiki on how to build it for your target system, you should be able to easily google them.
Note that we don't really care if VLC fails to compile as long as we manage to compile our plugin, so if something fails the build and there is an option to disable that thing that is failing from being built -- there is no harm in doing so.
In fact, you will likely want to disable as many build options as possible so that there would be less things to fail the build.

After the build is done, you should have `libpause_click_plugin.dll` (Windows) or `libpause_click_plugin.so` (Linux) or `libpause_click_plugin.dylib` (macOS) somewhere in the subdirectories of `modules/` of the build tree.
That's the plugin binary that you want.
