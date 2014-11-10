# Pause Click plugin for VLC
This plugin allows you to pause/play a video by clicking on the video image.

## Supported versions of VLC
Tested to work on VLC 2.1.5, so supposedly should work on 2.1.x.

## Install

### Windows
To be added soon.

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

## Usage
1. Restart VLC after installing the plugin
2. Go into advanced preferences: Tools -> Preferences -> Show settings -> All
3. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click
4. Restart VLC
5. Play a video
6. Click on video picture to pause/play the video
