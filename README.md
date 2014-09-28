# Pause Click plugin for VLC
This plugin allows you to pause/play a video by clicking on the video image.

## Supported versions of VLC
Tested to work on VLC 2.1.5, so supposedly should work on 2.1.x versions.

Doesn't work on VLC 3.x because of API changes.

## Install

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
1. Go into advanced preferences: Tools -> Preferences -> Show settings -> All
2. Enable/Disable the plugin with a checkbox: (in advanced preferences) Video -> Filters -> Pause/Play video on mouse click
3. Restart VLC
4. Play a video
5. Click on video picture to pause/play the video
