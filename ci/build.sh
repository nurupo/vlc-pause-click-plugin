#!/usr/bin/env bash
# shellcheck disable=SC2086
set -euxo pipefail

TARGET_OS="$1"
VLC_VERSION="$2"

echo_nightly_readme() {
  echo "This plugin was built for a nightly, in-development version of VLC." \
       "The development version of VLC often breaks ABI, making the plugin built for it non-functional in newer VLC builds." \
       "Meaning that a plugin targeting a nightly VLC built today might not work with a nightly VLC built tomorrow." \
       "Thus it's recommended that you use the specific nightly VLC build this plugin was made for: $1" | fold -sw 80
}

if [ "$TARGET_OS" = "linux" ]; then
  case $VLC_VERSION in
    "2.1") DEBIAN_VERSION="wheezy-backports" ;;
    "2.2") DEBIAN_VERSION="jessie" ;;
    "3.0") DEBIAN_VERSION="bullseye" ;;
    *) echo "Error: we don't support building VLC $VLC_VERSION on Linux" && exit 1 ;;
  esac

  # start a docker container of a specific version of Debian
  # different Debian versions have different VLC versions packaged
  sudo docker run -it -d --name ci -v "${PWD}":/repo debian:$DEBIAN_VERSION /bin/bash

  # runs a command in the started continer
  RUN() {
    sudo docker exec ci "$@"
  }

  # older versions have their repositories archived
  case $DEBIAN_VERSION in
    "wheezy-backports")
      RUN bash -c 'echo "
                   deb http://archive.debian.org/debian wheezy main
                   deb http://archive.debian.org/debian-security wheezy/updates main
                   " > /etc/apt/sources.list'
      RUN bash -c 'echo "deb http://archive.debian.org/debian wheezy-backports main" > /etc/apt/sources.list.d/backports.list'
      APT_OPTIONS="--force-yes -t $DEBIAN_VERSION"
      ;;
    "jessie")
      RUN bash -c 'echo "
                   deb http://archive.debian.org/debian jessie main
                   deb http://security.debian.org jessie/updates main
                   " > /etc/apt/sources.list'
      APT_OPTIONS="--force-yes"
      ;;
    *)
      APT_OPTIONS=""
  esac

  RUN apt-get update
  RUN apt-get install -y $APT_OPTIONS build-essential pkg-config libvlccore-dev libvlc-dev
  # print the version before testing as we won't see which version it is if the test fails
  RUN pkg-config --modversion libvlc
  RUN pkg-config --modversion libvlc | grep "^$VLC_VERSION"
  RUN pkg-config --modversion vlc-plugin
  RUN pkg-config --modversion vlc-plugin | grep "^$VLC_VERSION"
  if [ "$DEBIAN_VERSION" = "wheezy-backports" ]; then
    # gcc on wheezy-backports doesn't support c11
    RUN sed -i 's/gnu11/gnu99/g' /repo/Makefile
    # and -fdiagnostics-color
    RUN sed -i 's/-fdiagnostics-color//g' /repo/Makefile
  fi
  RUN make -C /repo install
elif [ "$TARGET_OS" = "windows" ]; then
  sudo apt-get update
  sudo apt-get install -y tree zip
  # use our own docker image for testing building of Windows binaries
  # has the benefit of making sure that our docker image is not broken
  sudo docker build -t vlc-pause-click-plugin-windows-build docker
  sudo docker run --rm -v "${PWD}":/repo -v "${PWD}"/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 32
  sudo docker run --rm -v "${PWD}":/repo -v "${PWD}"/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 64
  tree build -s --si --du
  # there should be exactly two dlls built, one for 32-bit VLC and another for 64-bit
  [ "$(find ./build -name "*.dll" | wc -l)" = "2" ] || false
  cd build
  if [ "$VLC_VERSION" = "4.0" ]; then
    echo_nightly_readme "$(cat $VLC_VERSION/32/VLC_DOWNLOAD_URL.txt)" > "$VLC_VERSION/32/README.txt"
    echo_nightly_readme "$(cat $VLC_VERSION/64/VLC_DOWNLOAD_URL.txt)" > "$VLC_VERSION/64/README.txt"
  fi
  ./zip-it.sh
  cd ..
  mkdir artifacts
  cp -a build/*.zip artifacts
elif [ "$TARGET_OS" = "macos" ]; then
  # updating fails sometimes, unable to fetch data off homebrew GitHub repo, so keep retrying
  until brew update; do
    sleep 30
  done
  mkdir -p tmp/tmp
  cd tmp/tmp
  if [ "$VLC_VERSION" = "4.0" ]; then
    brew install lynx
    LATEST_MACOS_DIR_URL=$(lynx -listonly -nonumbers -dump "https://artifacts.videolan.org/vlc/nightly-macos-x86_64/" | grep -E '[0-9]{8}-[0-9]+' | LC_COLLATE=C sort --stable --ignore-case | tail -n1)
    echo "$LATEST_MACOS_DIR_URL"
    LATEST_MACOS_FILE_URL=$(lynx -listonly -nonumbers -dump "$LATEST_MACOS_DIR_URL" | grep ".dmg$" | tail -n1)
    echo "$LATEST_MACOS_FILE_URL"
    if [ "$(echo "$LATEST_MACOS_FILE_URL" | wc -l | awk '{print $1}')" != "1" ]; then
      echo "Error: Weren't able to find the .dmg file for VLC $VLC_VERSION"
      exit 1
    fi
    wget "$LATEST_MACOS_FILE_URL"
    7z x "*.dmg" "*/VLC.app/Contents/Frameworks" || true
    mkdir ../lib
    mv ./*/VLC.app/Contents/Frameworks/*.dylib ../lib
    cd ..
    rm -rf ./tmp
    LATEST_WIN64_DIR_URL=$(lynx -listonly -nonumbers -dump "https://artifacts.videolan.org/vlc/nightly-win64/" | grep -E '[0-9]{8}-[0-9]+' | LC_COLLATE=C sort --stable --ignore-case | tail -n1)
    echo "$LATEST_WIN64_DIR_URL"
    LATEST_WIN64_FILE_URL=$(lynx -listonly -nonumbers -dump "$LATEST_WIN64_DIR_URL" | grep ".7z$" | grep -v "debug" | tail -n1)
    echo "$LATEST_WIN64_FILE_URL"
    if [ "$(echo "$LATEST_WIN64_FILE_URL" | wc -l | awk '{print $1}')" != "1" ]; then
      echo "Error: Weren't able to find the .7z file for VLC $VLC_VERSION"
      exit 1
    fi
    wget "$LATEST_WIN64_FILE_URL" -O vlc-$VLC_VERSION.0-win64.7z
    7z x "*.7z" -o* "*/sdk"
    mkdir -p include
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc include
    mv vlc-$VLC_VERSION.0-win64/*/sdk/lib/pkgconfig lib
    rm -rf ./vlc-$VLC_VERSION.0-win64*
  else
    wget "https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/macosx/vlc-$VLC_VERSION.0.dmg"
    # extracting contents of a .dmg file using 7-zip is more of a hack, and while it works 7z exits with an error code we want to suppress
    7z x "*.dmg" "*/VLC.app/Contents/MacOS" || true
    mv ./*/VLC.app/Contents/MacOS/lib ..
    mv ./*/VLC.app/Contents/MacOS/include ..
    cd ..
    rm -rf ./tmp
    # in contrast to Windows VLC including a very nice and complete sdk directory,
    # macOS VLC doesn't package .pc files, plugin headers and in 2.1.0 libvlc_version.h is not generated off libvlc_version.h.in.
    # it just packages libvlc.dylib, libvlccore.dylib and libvlc headers.
    # we grab the missing pieces from the Windows VLC, there shouldn't be anything platform-specific in those so it should work
    wget "https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/win64/vlc-$VLC_VERSION.0-win64.7z" -O vlc-$VLC_VERSION.0-win64.7z
    7z x "*.7z" -o* "*/sdk"
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc/libvlc_version.h include/vlc/
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc/plugins include/vlc
    mv vlc-$VLC_VERSION.0-win64/*/sdk/lib/pkgconfig lib
    rm -rf ./vlc-$VLC_VERSION.0-win64*
  fi
  # fix paths in .pc files
  sed -i "" "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
  export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
  cd lib
  # fix library symlink
  ln -sf libvlccore.*.dylib libvlccore.dylib
  cd ../..
  make OS=macOS
  if [ "$VLC_VERSION" = "4.0" ]; then
    echo "$LATEST_MACOS_DIR_URL" > VLC_DOWNLOAD_URL.txt
    echo_nightly_readme "$(cat VLC_DOWNLOAD_URL.txt)" > "README.txt"
    zip -j vlc-$VLC_VERSION-macosx.zip libpause_click_plugin.dylib VLC_DOWNLOAD_URL.txt README.txt
  else
    zip -j vlc-$VLC_VERSION-macosx.zip libpause_click_plugin.dylib
  fi
  mkdir artifacts
  cp -a vlc-$VLC_VERSION-macosx.zip artifacts
fi

exit 0
