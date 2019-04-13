#!/usr/bin/env bash
set -euxo pipefail

if [ "$TARGET" = "Linux" ]; then
  # start a docker container of a specific version of Debian
  # different Debian versions have different VLC versions packaged: 2.1, 2.2 and 3.0
  sudo docker run -it -d --name ci -v ${PWD}:/repo debian:$DEBIAN /bin/bash
  # runs a command in the started continer
  RUN() {
    sudo docker exec ci "$@"
  }
  if [ "$DEBIAN" = "wheezy-backports" ]; then
    # Debian Wheezy package repository was moved to Archive but the Docker image still links to the main repository
    RUN bash -c 'echo "
                 deb http://archive.debian.org/debian wheezy main
                 deb http://archive.debian.org/debian-security wheezy/updates main
                 " > /etc/apt/sources.list'
    RUN bash -c 'echo "deb http://archive.debian.org/debian wheezy-backports main" > /etc/apt/sources.list.d/backports.list'
    RUN bash -c 'echo "Acquire::Check-Valid-Until false;" > /etc/apt/apt.conf.d/10-nocheckvalid'
  fi
  RUN apt-get update
  # for some reason apt is having issues with verifying packages due to old/missing keys?
  RUN apt-get install -y gnupg
  RUN apt-key update
  RUN apt-get install -y -t $DEBIAN build-essential pkg-config libvlccore-dev libvlc-dev
  # print the version before testing it as we won't see which version it is if the test fails
  RUN pkg-config --modversion libvlc
  RUN pkg-config --modversion libvlc | grep "^$VLC_VERSION"
  RUN pkg-config --modversion vlc-plugin
  RUN pkg-config --modversion vlc-plugin | grep "^$VLC_VERSION"
  if [ "$DEBIAN" = "wheezy-backports" ]; then
    # gcc on wheezy-backports doesn't support c11
    RUN sed -i 's/gnu11/gnu99/g' /repo/Makefile
  fi
  RUN make -C /repo install
elif [ "$TARGET" = "Windows" ]; then
  sudo apt-get update
  sudo apt-get install -y tree
  # use our own docker image for testing building of Windows binaries
  # makes sure that our docker image is not broken too
  sudo docker build -t vlc-pause-click-plugin-windows-build docker
  sudo docker run --rm -v `pwd`:/repo -v `pwd`/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 32
  sudo docker run --rm -v `pwd`:/repo -v `pwd`/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 64
  tree build -s --si --du
  # there should be exactly two dlls built, one for 32-bit VLC and another for 64-bit
  [ $(find ./build -name "*.dll" | wc -l) = "2" ] || false
elif [ "$TARGET" = "macOS" ]; then
  brew update
  brew install p7zip
  mkdir -p tmp/tmp
  cd tmp/tmp
  wget https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/macosx/vlc-$VLC_VERSION.0.dmg
  # extracting contents of a .dmg file using 7-zip is more of a hack, and while it works 7z exits with an error code we want to supress
  7z x "*.dmg" "*/VLC.app/Contents/MacOS" || true
  mv */VLC.app/Contents/MacOS/lib ..
  mv */VLC.app/Contents/MacOS/include ..
  rm -rf ./*
  cd ..
  rm -rf ./tmp
  # in contrast to Windows VLC including a very nice and complete sdk directory,
  # macOS VLC doesn't package .pc files, plugin headers and in 2.1.0 libvlc_version.h is not generated off libvlc_version.h.in.
  # it just packags libvlc.dylib, libvlccore.dylib and libvlc headers.
  # honestly, wtf VLC team, get your game together.
  # we grab the missing pieces from the Windows VLC, there shouldn't be anything platform-specific in those so it should work
  wget https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/win32/vlc-$VLC_VERSION.0-win32.7z -O vlc-$VLC_VERSION.0-win32.7z
  7z x "*.7z" -o* "*/sdk"
  mv vlc-$VLC_VERSION.0-win32/*/sdk/include/vlc/libvlc_version.h include/vlc/
  mv vlc-$VLC_VERSION.0-win32/*/sdk/include/vlc/plugins include/vlc/plugins
  mv vlc-$VLC_VERSION.0-win32/*/sdk/lib/pkgconfig lib/pkgconfig
  rm -rf ./vlc-$VLC_VERSION.0-win32
  # fix paths in .pc files
  sed -i "" "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
  export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
  cd lib
  # fix library symlink
  ln -sf libvlccore.*.dylib libvlccore.dylib
  cd ../..
  make OS=macOS
  zip -j vlc-$VLC_VERSION-macosx.zip libpause_click_plugin.dylib
fi
