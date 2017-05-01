# Pause Click plugin for VLC
This plugin allows you to pause/play a video by clicking on the video image.

## Supported versions of VLC
Tested to work on VLC 2.2.0, so supposedly should work on 2.2.x versions.

## Install

### Windows
Download an appropriate archive:

Version/Bitness | VLC 32 bit | VLC 64 bit
----------- | ------ | -------
VLC 2.2.x | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.1/vlc-2.2.x-32bit-win.zip) | [Download](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.1/vlc-2.2.x-64bit-win.zip)

Extract the archive at `{VLC}\plugins\video_filter\`, where `{VLC}` stands for the directory the VLC was installed to, for example `C:\Program Files (x86)\VideoLAN\VLC\`.

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### OS X

***OS X plugin is outdated, we are looking for someone [to contribute it](/BUILD.md).***

Download an appropriate archive:

Version/Bitness | VLC 64 bit
----------- | -------
VLC 2.2.x | [Download (old version of the plugin)](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.2.1/vlc-2.2.x-64bit-osx.zip)

Extract the archive at `/Applications/VLC.app/Contents/MacOS/plugins/video_filter/`

This plugin binary was kindly contributed by [@xenio](https://github.com/xenio).

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### Debian
Get required libraries and tools:
```bash
sudo apt-get install build-essential pkg-config libvlccore-dev
```

Build and install:
```bash
make
sudo make install
```

Because of API change that occured in VLC 2.2.0, the plugin now uses things from `vlc_interface.h` header file, which is not present in `libvlccore-dev` package because it's not considered to be a public header by VLC team / their build system.
Meaning that the plugin won't build without that file present.
I have fixed the issue by simply copying `vlc_interface.h` from VLC's repository in this repository.
It's highly unlikely, but if the plugin for some doesn't work, you might need to get `vlc_interface.h` appropriate for your VLC version out of the VLC repository and try rebuilding the plugin with that.

If these instructions don't work for you (perhaps you are using a non-Debian-derived Linux distribuion or don't have `libvlccore` available), [there is another way to build the plugin binary](/BUILD.md).

## Usage
1. Restart VLC to load the newly added plugin [[screenshot]](http://i.imgur.com/6oTRtD8.png)
2. Go into advanced preferences: Tools -> Preferences -> Show settings -> All [[screenshot]](http://i.imgur.com/ONLskiX.png) [[screenshot]](http://i.imgur.com/V7ql7HR.png)
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Interface -> Control Interfaces -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/aMkGRRK.png)
4. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/U5TZpDp.png)
5. Change mouse button to the one you want: (in advanced preferences) Video -> Filters -> Pause click -> Mouse Button [[screenshot]](http://i.imgur.com/T5yp0jw.png)
6. Restart VLC for settings to take place [[screenshot]](http://i.imgur.com/6oTRtD8.png)
7. Play a video
8. Click on video picture to pause/play the video
