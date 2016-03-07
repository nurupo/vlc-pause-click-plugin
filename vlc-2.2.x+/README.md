# Pause Click plugin for VLC
This plugin allows you to pause/play a video by clicking on the video image.

## Supported versions of VLC
Tested to work on VLC 2.2.0 and 3.0.0, so supposedly should work on 2.2.x and 3.0.x versions.

## Install

### Windows
Download an appropriate archive:

VLC version | 32 bit | 64 bit
----------- | ------ | -------
2.2.x | [Tested on 2.2.0](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.0/vlc-2.2.x-32bit-win.zip) | [Tested on 2.2.0](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.0/vlc-2.2.x-64bit-win.zip)
3.0.x | [Tested on 3.0.0](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.0/vlc-3.0.x-32bit-win.zip) | [Tested on 3.0.0](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.3.0/vlc-3.0.x-64bit-win.zip)

Extract the archive at `{VLC}/plugins/video_filter/`

If you want to build the plugin binary yourself, take a look at the [build instructions](/common/building/).

### OS X

***OS X plugins are outdated, we are looking for someone [to contribute them](/common/building/).***

Download an appropriate archive:

VLC version | 64 bit
----------- | -------
2.2.x | [Old version of the plugin](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.2.1/vlc-2.2.x-64bit-osx.zip)
3.0.x | [Old version of the plugin](https://github.com/nurupo/vlc-pause-click-plugin/releases/download/0.2.1/vlc-3.0.x-64bit-osx.zip)

Extract the archive at `/Applications/VLC.app/Contents/MacOS/plugins/video_filter/`

These binaries were kindly contributed by [@xenio](https://github.com/xenio).

If you want to build the plugin binary yourself, take a look at the [build instructions](/common/building/).

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

Note: because of API change in VLC 2.2.0 this build requires `vlc_interface.h`, which is not present in libvlccore-dev package since it's not considered to be a public header by VLC build system. `vlc_interface.h` that is present here is from the current master branch of VLC (3.0.0). It's highly unlikely, but if the plugin doesn't work, you might need to get `vlc_interface.h` appropriate for your VLC version out of the VLC source tree and try rebuilding the plugin with that.

If those instructions don't work for you (perhaps you are using a non-Debian-derived Linux distribuion or don't have `libvlccore` available), [there is another way to build the plugin binary](/common/building/).

## Usage
1. Restart VLC to load the newly added plugin
2. Go into advanced preferences: Tools -> Preferences -> Show settings -> All
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click
4. Enable/Disable the plugin with a checkbox: (in advanced preferences) Interface -> Control Interfaces -> Pause/Play video on mouse click
5. Change mouse button to the one you want: (in advanced preferences) Video -> Filters -> Pause click -> Mouse Button
6. Restart VLC for settings to take place
7. Play a video
8. Click on video picture to pause/play the video
