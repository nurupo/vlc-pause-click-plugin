# Pause Click plugin for VLC
VLC plugin that allows you to pause/play a video by clicking on the video image.

Works nicely with "double-click to fullscreen" too, if you enable ["Ignore double clicks" option in the settings](http://i.imgur.com/gPPXjqw.png).

## Table of contents

- [Supported versions of VLC](#supported-versions-of-vlc)
- [Install](#install)
  - [Windows](#windows)
  - [macOS](#macos)
  - [Linux](#linux)
    - [Debian](#debian)
    - [Arch Linux](#arch-linux)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)

## Supported versions of VLC
VLC 2.1, 2.2 and 3.0 are supported.

VLC versions below 2.1 are not supported.
However, there is [a lua extension](https://github.com/3demax/vlc-clickpause) that should work with VLC 2.0 and possibly even earlier versions of VLC.

Support for the newer VLC versions will be available as they get released and as I get time to work on it.

Nightly development version of VLC (VLC 4.0) is not officially supported as it hasn't been released yet.
The development version of VLC often breaks ABI, making the plugin built for it non-functional in newer VLC builds.
Meaning that if I release the plugin targeting VLC 4.0 build that was done today, the plugin might not work with tomorrow's build of VLC 4.0 that you might use.
If you feel adventurous, you could try building this plugin for the specific VLC 4.0 revision that you use yourself, it might require some code fixes but shouldn't be very hard if you are familiar with C development.

## Install

### Windows
Download an appropriate archive:

Version/Bitness | VLC 32 bit | VLC 64 bit
----------- | ------ | -------
VLC 2.1 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-2.1-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-2.1-64bit-win.zip)
VLC 2.2 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-2.2-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-2.2-64bit-win.zip)
VLC 3.0 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-3.0-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-3.0-64bit-win.zip)

Extract the archive at `{VLC}\plugins\video_filter\`, where `{VLC}` is the directory the VLC was installed into, for example `C:\Program Files (x86)\VideoLAN\VLC\`.

Then follow [the usage instructions](#usage) below.

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### macOS

Download an appropriate archive:

Version/Bitness | VLC 64 bit
----------- | -------
VLC 2.1 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-2.1-macosx.zip)
VLC 2.2 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-2.2-macosx.zip)
VLC 3.0 | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/1.0.0/vlc-3.0-macosx.zip)

Extract the archive at `/Applications/VLC.app/Contents/MacOS/plugins/`

Then follow [the usage instructions](#usage) below.

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### Linux

Note that Snap versions of VLC are not supported.
Not supported in a sense that I haven't figured out how to build the plugin such that it would be ABI compatible with the Snap VLC and how to make the Snap VLC load the plugin.
If someone could figure that out and contribute their findings - that would be very helpful.
[This Snap VLC discussion](https://github.com/nurupo/vlc-pause-click-plugin/issues/33) might be useful.

#### Debian
Get required libraries and tools:
```bash
sudo apt-get install build-essential pkg-config libvlccore-dev libvlc-dev
```

Build and install:
```bash
make
sudo make install
```

Then follow [the usage instructions](#usage) below.

If these build instructions don't work for you (perhaps you are using a non-Debian-derived Linux distribuion), [there are more generic build instructions available](/BUILD.md).

#### Arch Linux
There is a `vlc-pause-click-plugin` package [available](https://aur.archlinux.org/packages/vlc-pause-click-plugin/) in the AUR repository.

## Usage
1. Restart VLC to load the newly added plugin [[screenshot]](http://i.imgur.com/6oTRtD8.png)
2. Go into advanced preferences: Tools -> Preferences -> Show settings -> All [[screenshot]](http://i.imgur.com/ONLskiX.png) [[screenshot]](http://i.imgur.com/V7ql7HR.png)
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Interface -> Control Interfaces -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/aMkGRRK.png)
4. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/U5TZpDp.png)
5. Change the plugin settings however you like: (in advanced preferences) Video -> Filters -> Pause click [[screenshot]](http://i.imgur.com/gPPXjqw.png)
6. Restart VLC for settings to take place [[screenshot]](http://i.imgur.com/6oTRtD8.png)
7. Play a video
8. Click on video picture to pause/play the video

Make sure you have checked both of "Pause/Play video on mouse click" checkboxes per \#3 and \#4!
It's a very common mistake to skip one of those.

## Troubleshooting

### I don't see any plugin-related options described in [Usage](#usage) instructions in my VLC preferences

- Make sure to restart VLC after you have put the plugin `.dll` (Windows) or `.so` (Linux) or `.dylib` (macOS) in the appropriate directory.
- Make sure you have downloaded the appropriate version of the plugin for the VLC you are trying to use it with.
Plugin's version and bitness should match the version and bitness of the VLC you are trying to use the plugin with.
Plugin's bitness has nothing to do with the operation system bintess.
For example, if you use 32-bit VLC, it doesn't matter whether your Windows is 32-bit or 64-bit, you should use 32-bit version of the plugin, because it matches the bitness of the VLC you use.
- Some users [have reported](https://github.com/nurupo/vlc-pause-click-plugin/issues/32#issue-296248669) that they had to run the VLC executable with `--reset-plugins-cache` flag once for the plugin to appear in the GUI.

### I do see the plugin-related options described in [Usage](#usage) instructions in my VLC preferences, but the plugin still doesn't work

- Double-check that you have followed [Usage](#usage) instructions closely, especially 3rd and 4th points.
Look at the screenshots.
- Some users [have reported](https://github.com/nurupo/vlc-pause-click-plugin/issues/45#issuecomment-418964880) that they had to re-install VLC and select "Delete preferences and cache" in the installer for the plugin to work.

If the issue persists, open an Issue in this repository and I will try to help.
