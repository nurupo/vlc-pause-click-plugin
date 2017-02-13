# Pause Click plugin for VLC
This plugin allows you to pause/play a video by clicking on the video image.

## Supported versions of VLC
Tested to work on VLC 2.1.5, so supposedly should work on 2.1.x.

## Install

### Windows
Download an appropriate archive:

VLC version | 32 bit | 64 bit
----------- | ------ | -------
2.1.x | [Tested on 2.1.5](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.0/vlc-2.1.x-32bit-win.zip) | [Tested on 2.1.5](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.0/vlc-2.1.x-64bit-win.zip)

Extract the archive at `{VLC}\plugins\video_filter\`, where `{VLC}` stands for the directory the VLC was installed to, for example `C:\Program Files (x86)\VideoLAN\VLC\`.

If you want to build the plugin binary yourself, take a look at the [build instructions](/BUILD.md).

### OS X
There are currently no precompiled OS X plugin binaries.

[You are welcome to contribute them though](/BUILD.md).

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

If these instructions don't work for you (perhaps you are using a non-Debian-derived Linux distribuion or don't have `libvlccore` available), [there is another way to build the plugin binary](/BUILD.md).

## Usage
1. Restart VLC to load the newly added plugin [[screenshot]](http://i.imgur.com/6oTRtD8.png)
2. Go into advanced preferences: Tools -> Preferences -> Show settings -> All [[screenshot]](http://i.imgur.com/ONLskiX.png) [[screenshot]](http://i.imgur.com/V7ql7HR.png)
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click [[screenshot]](http://i.imgur.com/U5TZpDp.png)
4. Change mouse button to the one you want: (in advanced preferences) Video -> Filters -> Pause click -> Mouse Button [[screenshot]](http://i.imgur.com/T5yp0jw.png)
5. Restart VLC for settings to take place [[screenshot]](http://i.imgur.com/6oTRtD8.png)
6. Play a video
7. Click on video picture to pause/play the video
