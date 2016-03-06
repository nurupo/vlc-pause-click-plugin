## Building (all platforms)

If instead of using the precompiled plugin binaries you want to build Windows or OS X plugin binary yourself, or for some reason Debian instructions don't work for you, you can follow these instructions on building the plugin.

(Note that [there is a Dockerfile](./docker) that automates building of Windows binaries of this plugin. I highly recommend using it if you want to build Windows binaries. Other platforms might benefit from reading what the Docker file does.)

What these instructions sum up to is building VLC itself with the plugin integrated into its source tree / build system, so that the plugin would be built together with VLC.

Git clone VLC:

```sh
git clone git://git.videolan.org/vlc.git
```

Checkout the git tag that matches VLC version that you want to build the plugin for. You can use `git tag` to get a list of all tags available. As of writing this there is no tag for 3.0 version yet, so `master` branch should be used. For example, for 2.2.0 you would do:

```sh
cd vlc
git checkout 2.2.0-git
```

Add the plugin into the VLC build system by copying the appropriate version of `pause_click.c` (there are [vlc-2.1.x](/vlc-2.1.x) and [vlc-2.2.x+](/vlc-2.2.x+) versions) into `modules/video_filter/`.

Add

```
libpause_click_plugin_la_SOURCES = pause_click.c
libpause_click_plugin_la_CFLAGS = $(AM_CFLAGS)
libpause_click_plugin_la_LIBADD = $(AM_LIBADD)
libvlc_LTLIBRARIES += libpause_click_plugin.la
```

to the end of `modules/video_filter/Modules.am` file for VLC 2.1.0

or


```
libpause_click_plugin_la_SOURCES = pause_click.c
libpause_click_plugin_la_CFLAGS = $(AM_CFLAGS)
libpause_click_plugin_la_LIBADD = $(AM_LIBADD)
video_filter_LTLIBRARIES += libpause_click_plugin.la
```

to the end of `modules/video_filter/Modules.am` file for VLC 2.2.0

or

```
libpause_click_plugin_la_SOURCES = video_filter/pause_click.c
libpause_click_plugin_la_CFLAGS = $(AM_CFLAGS)
libpause_click_plugin_la_LIBADD = $(AM_LIBADD)
video_filter_LTLIBRARIES += libpause_click_plugin.la
```

to the end of `modules/video_filter/Makefile.am` file for VLC 3.0.0

Build VLC the way you would usually do so. There should be some instruction on VLC wiki, you should be able to easily google that. Note that you don't care for VLC itself, you just care for the plugin to be built, so if something fails your build and there is an option to disable that thing that is failing from being built -- there is no harm in doing so.

After the build is done, you should have `libpause_click_plugin.dll` or `libpause_click_plugin.so` or `libpause_click_plugin.dylib` (the extension depends on the target platform) somewhere in the subdirectories of `modules/` of the build tree. That's the plugin binary that you want.
