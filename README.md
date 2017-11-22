# Pause Click plugin for VLC
This plugin allows you to pause/play a video by clicking on the video image.

## Table of contents

- [Supported versions of VLC](#supported-versions-of-vlc)
- [Install](#install)
  - [Windows](#windows)
  - [OS X](#os-x)
  - [Linux](#linux)
    - [Debian](#debian)
    - [Arch Linux](#arch-linux)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)

## Supported versions of VLC
Tested to work on VLC 2.1 and 2.2.

Versions lower than 2.1 are not supported by this plugin, but there is [this](https://github.com/3demax/vlc-clickpause) lua extension that should work with VLC 2.0 and possibly even earlier versions.

## Install

### Windows
Download an appropriate archive:

Version/Bitness | VLC 32 bit | VLC 64 bit
----------- | ------ | -------
VLC 2.1 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.4.0/vlc-2.1-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.4.0/vlc-2.1-64bit-win.zip)
VLC 2.2 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.4.0/vlc-2.2-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.4.0/vlc-2.2-64bit-win.zip)

Extract the archive at `{VLC}\plugins\video_filter\`, where `{VLC}` is the directory the VLC was installed into, for example `C:\Program Files (x86)\VideoLAN\VLC\`.

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### OS X

***OS X plugin is outdated, we are looking for someone [to contribute it](/BUILD.md).***

Download an appropriate archive:

Version/Bitness | VLC 64 bit
----------- | -------
VLC 2.2 | [Download (old version of the plugin)](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.2.1/vlc-2.2.x-64bit-osx.zip)

Extract the archive at `/Applications/VLC.app/Contents/MacOS/plugins/video_filter/`

This plugin binary was kindly contributed by [@xenio](https://github.com/xenio).

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### Linux

#### Debian
Get required libraries and tools:
```bash
sudo apt-get install build-essential pkg-config libvlccore-dev
```

Build and install:
```bash
make
sudo make install
```

If these instructions don't work for you (perhaps you are using a non-Debian-derived Linux distribuion or don't have `libvlccore` available), [there is another way to build the plugin binary](/BUILD.md).

#### Arch Linux
There is a `vlc-pause-click-plugin` package [available](https://aur.archlinux.org/packages/vlc-pause-click-plugin/) in the AUR repository.

## Usage
1. Restart VLC to load the newly added plugin [[screenshot]](http://i.imgur.com/6oTRtD8.png)
2. Go into advanced preferences: Tools -> Preferences -> Show settings -> All [[screenshot]](http://i.imgur.com/ONLskiX.png) [[screenshot]](http://i.imgur.com/V7ql7HR.png)
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Interface -> Control Interfaces -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/aMkGRRK.png)
4. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/U5TZpDp.png)
5. Change the plugin settings however you like: (in advanced preferences) Video -> Filters -> Pause click [[screenshot]](https://i.imgur.com/gPPXjqw.png)
6. Restart VLC for settings to take place [[screenshot]](http://i.imgur.com/6oTRtD8.png)
7. Play a video
8. Click on video picture to pause/play the video

Make sure you have checked both of "Pause/Play video on mouse click" checkboxes per \#3 and \#4!

## Troubleshooting

### I don't see any plugin-related options described in [Usage](#usage) instructions in my VLC preferences

- Make sure to restart VLC after you have put the plugin `.dll` (Widnows) or `.so` (Linux) or `.dylib` (OS X) in the appropriate directory.
- Make sure you have downloaded the appropriate version of the plugin for the VLC you are trying to use it with. Plugin's version and bitness should match the version and bitness of the VLC you are trying to use the plugin with. Plugin's bitness has nothing to do with the operation system bintess. For example, if you use 32-bit VLC, it doesn't matter whether your Windows is 32-bit or 64-bit, you should use 32-bit version of the plugin, because it matches the bitness of the VLC you use.

### I do see the plugin-related options described in [Usage](#usage) instructions in my VLC preferences, but the plugin still doesn't work

- Double-check that you have followed [Usage](#usage) instructions closely, especially 3rd and 4th points. Look at the screenshots. If the issue persists, open an Issue in this repository and I will try to help.

