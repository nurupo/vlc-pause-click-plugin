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
  - [I don't see any plugin-related options described in Usage instructions](#i-dont-see-any-plugin-related-options-described-in-usage-instructions)
  - [I do see the plugin-related options described in Usage instructions, but the plugin still doesn't work](#i-do-see-the-plugin-related-options-described-in-usage-instructions-but-the-plugin-still-doesnt-work)

## Supported versions of VLC
Tested to work on VLC 2.1.5, so supposedly should work on 2.1.x.

## Install

### Windows
Download an appropriate archive:

Version/Bitness | VLC 32 bit | VLC 64 bit
----------- | ------ | -------
VLC 2.1.x | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.1/vlc-2.1.x-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.1/vlc-2.1.x-64bit-win.zip)

Extract the archive at `{VLC}\plugins\video_filter\`, where `{VLC}` stands for the directory the VLC was installed to, for example `C:\Program Files (x86)\VideoLAN\VLC\`.

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### OS X
There are currently no precompiled OS X plugin binaries.

[You are welcome to contribute them though](/BUILD.md).

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
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/U5TZpDp.png)
4. Change mouse button to the one you want: (in advanced preferences) Video -> Filters -> Pause click -> Mouse Button [[screenshot]](http://i.imgur.com/T5yp0jw.png)
5. Restart VLC for settings to take place [[screenshot]](http://i.imgur.com/6oTRtD8.png)
6. Play a video
7. Click on video picture to pause/play the video

## Troubleshooting

### I don't see any plugin-related options described in [Usage](#usage) instructions

- Make sure to restart VLC after you have put the plugin `.dll` (Widnows) or `.so` (Linux) or `.dylib` (OS X) in the appropriate directory.
- Make sure you have downloaded the appropriate version of the plugin for the VLC you are trying to use it with. Plugin's version and bitness should match the version and bitness of the VLC you are trying to use the plugin with. Plugin's bitness has nothing to do with the operation system bintess. For example, if use 32-bit VLC, it doesn't matter whether your Windows is 32-bit or 64-bit, you should use 32-bit version of the plugin, because it matches the bitness of the VLC you use.

### I do see the plugin-related options described in [Usage](#usage) instructions, but the plugin still doesn't work

- Double-check that you have followed [Usage](#usage) instructions closely, look at the screenshots. If the issue persists, open an Issue in this repository and I will try to help.
