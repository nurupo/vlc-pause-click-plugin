#!/usr/bin/env bash
# shellcheck disable=SC2086
set -euxo pipefail

TARGET_OS="$1"
VLC_VERSION="$2"

NIGHTLY_README="$(
  echo "This plugin was built for a nightly, in-development version of VLC." \
       "The development version of VLC often breaks ABI compatibility, meaning that a plugin targeting a nightly VLC built today might not work with a nightly VLC built tomorrow." \
       "Thus it's recommended that you use the specific nightly VLC build this plugin was made for:" | \
  fold -sw 80
)"

if [ "$TARGET_OS" = "linux" ]; then
  case $VLC_VERSION in
    "2.1") DOCKER_DEBIAN_VERSION="wheezy-backports" ;;
    "2.2") DOCKER_DEBIAN_VERSION="jessie-slim" ;;
    "3.0") DOCKER_DEBIAN_VERSION="bookworm-slim" ;;
    *) echo "Error: we don't support building VLC $VLC_VERSION on Linux" && exit 1 ;;
  esac
  DEBIAN_VERSION="${DOCKER_DEBIAN_VERSION/%-slim/}"

  # start a docker container of a specific version of Debian
  # different Debian versions have different VLC versions packaged
  sudo docker run -it -d --name ci -v "${PWD}":/repo debian:$DOCKER_DEBIAN_VERSION /bin/bash

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
                   deb http://archive.debian.org/debian-security jessie/updates main
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
  sudo docker build -t vlc-pause-click-plugin-windows-build packaging/windows/docker
  mkdir build
  sudo docker run --rm -v "${PWD}":/repo -v "${PWD}"/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 32
  sudo docker run --rm -v "${PWD}":/repo -v "${PWD}"/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 64
  tree build -s --si --du
  # there should be exactly two dlls built, one for 32-bit VLC and another for 64-bit
  [ "$(find ./build -name "*.dll" | wc -l)" = "2" ] || false
  cd build
  if [ "$VLC_VERSION" = "4.0" ]; then
    echo "$NIGHTLY_README" > "$VLC_VERSION/32/README.txt"
    echo "$(cat $VLC_VERSION/32/VLC_DOWNLOAD_URL.txt)" > "$VLC_VERSION/32/README.txt"
    echo "$NIGHTLY_README" > "$VLC_VERSION/64/README.txt"
    echo "$(cat $VLC_VERSION/32/VLC_DOWNLOAD_URL.txt)" > "$VLC_VERSION/64/README.txt"
  fi
  ../packaging/windows/zip.sh
  cd ..
  mkdir artifacts
  cp -a build/*.zip artifacts
elif [ "$TARGET_OS" = "macos" ]; then
  # updating fails sometimes, unable to fetch data off homebrew GitHub repo, so keep retrying
  until brew update; do
    sleep 30
  done
  if [ "$VLC_VERSION" = "4.0" ]; then
    brew install lynx

    build() {
      ARCH="$1"
      URL="$2"
      CC="$3"

      mkdir -p "sdk/$ARCH"
      cd "sdk/$ARCH"

      LATEST_MACOS_DIR_URL=$(lynx -listonly -nonumbers -dump "$URL" | grep -E '[0-9]{8}-[0-9]+' | LC_COLLATE=C sort --stable --ignore-case | tail -n1)
      echo "$LATEST_MACOS_DIR_URL"
      LATEST_MACOS_SDK_URL=$(lynx -listonly -nonumbers -dump "$LATEST_MACOS_DIR_URL" | grep ".tar.gz$" | tail -n1)
      echo "$LATEST_MACOS_SDK_URL"
      if [ "$(echo "$LATEST_MACOS_SDK_URL" | wc -l | awk '{print $1}')" != "1" ]; then
        echo "Error: Weren't able to find the .tar.gz SDK file for VLC $VLC_VERSION"
        exit 1
      fi
      wget "$LATEST_MACOS_SDK_URL"
      tar xvf ./*.tar.gz
      rm ./*.tar.gz

      # fix paths in .pc files
      sed -i "" "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
      export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
      cd ../..
      echo -e "#include <time.h>\n$(cat sdk/$ARCH/include/vlc/plugins/vlc_atomic.h)" > sdk/$ARCH/include/vlc/plugins/vlc_atomic.h
      make OS=macOS CC="$CC"
      echo "$LATEST_MACOS_DIR_URL" > VLC_DOWNLOAD_URL.txt
      mv libpause_click_plugin.dylib VLC_DOWNLOAD_URL.txt "sdk/$ARCH"
      make clean
    }
    build "intel64" "https://artifacts.videolan.org/vlc/nightly-macos-x86_64/" "cc"
    build "arm64"   "https://artifacts.videolan.org/vlc/nightly-macos-arm64/"  "cc -target arm64-apple-macos10.11"
    lipo "sdk/intel64/libpause_click_plugin.dylib" "sdk/arm64/libpause_click_plugin.dylib" -create -output libpause_click_plugin.dylib
    echo "$NIGHTLY_README" > README.txt
    echo "intel64: $(cat "sdk/intel64/VLC_DOWNLOAD_URL.txt")" >> README.txt
    echo "arm64: $(cat "sdk/arm64/VLC_DOWNLOAD_URL.txt")" >> README.txt
    #mv intel64/VLC_DOWNLOAD_URL.txt VLC_DOWNLOAD_URL_INTEL64.txt
    #mv arm64/VLC_DOWNLOAD_URL.txt VLC_DOWNLOAD_URL_ARM64.txt
    mkdir artifacts
    zip -j artifacts/vlc-$VLC_VERSION-macosx-universal.zip libpause_click_plugin.dylib README.txt
  elif [ "$VLC_VERSION" = "3.0" ]; then
    # in contrast to Windows VLC including a very nice and complete sdk directory,
    # macOS VLC doesn't package .pc files, plugin headers and, in 2.1.0, libvlc_version.h is not generated off libvlc_version.h.in.
    # it just packages libvlc.dylib, libvlccore.dylib and libvlc headers.
    # we grab the missing pieces from the Windows VLC, there shouldn't be anything platform-specific in those so it should work
    mkdir windows-sdk
    cd windows-sdk
    wget "https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/win64/vlc-$VLC_VERSION.0-win64.7z" -O vlc-$VLC_VERSION.0-win64.7z
    7z x "*.7z" -o* "*/sdk"
    mkdir -p include/vlc lib
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc/libvlc_version.h include/vlc/
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc/plugins include/vlc
    mv vlc-$VLC_VERSION.0-win64/*/sdk/lib/pkgconfig lib
    rm -rf ./vlc-$VLC_VERSION.0-win64*
    cd ..

    build() {
      ARCH="$1"
      URL="$2"
      CC="$3"

      mkdir -p "sdk/$ARCH"
      cd "sdk/$ARCH"

      mkdir tmp
      cd tmp
      wget "$URL"
      # extracting contents of a .dmg file using 7-zip is more of a hack, and while it works 7z exits with an error code we want to suppress
      7z x "*.dmg" "*/VLC.app/Contents/MacOS" || true
      mv ./*/VLC.app/Contents/MacOS/lib ..
      mv ./*/VLC.app/Contents/MacOS/include ..
      cd ..
      rm -rf ./tmp
      cp -a ../../windows-sdk/* .

      # fix paths in .pc files
      sed -i "" "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
      export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
      # fix library symlink
      cd lib
      ln -sf libvlccore.*.dylib libvlccore.dylib
      cd ..
      cd ../..
      make OS=macOS CC="$CC"
      mv libpause_click_plugin.dylib "sdk/$ARCH"
      make clean
    }
    build "intel64" "https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/macosx/vlc-$VLC_VERSION.0.dmg" "cc"
    # VLC 3.0.13 is the first universal macOS VLC version
    build "arm64"   "https://download.videolan.org/pub/videolan/vlc/3.0.13/macosx/vlc-3.0.13-universal.dmg"       "cc -target arm64-apple-macos10.11"
    lipo "sdk/intel64/libpause_click_plugin.dylib" "sdk/arm64/libpause_click_plugin.dylib" -create -output libpause_click_plugin.dylib
    mkdir artifacts
    zip -j artifacts/vlc-$VLC_VERSION-macosx-universal.zip libpause_click_plugin.dylib
  else
    # 2.* versions are intel64-only
    mkdir -p sdk/intel64/tmp
    cd sdk/intel64/tmp
    wget "https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/macosx/vlc-$VLC_VERSION.0.dmg"
    7z x "*.dmg" "*/VLC.app/Contents/MacOS" || true
    mv ./*/VLC.app/Contents/MacOS/lib ..
    mv ./*/VLC.app/Contents/MacOS/include ..
    cd ..
    rm -rf ./tmp

    wget "https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/win64/vlc-$VLC_VERSION.0-win64.7z" -O vlc-$VLC_VERSION.0-win64.7z
    7z x "*.7z" -o* "*/sdk"
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc/libvlc_version.h include/vlc/
    mv vlc-$VLC_VERSION.0-win64/*/sdk/include/vlc/plugins include/vlc
    mv vlc-$VLC_VERSION.0-win64/*/sdk/lib/pkgconfig lib
    rm -rf ./vlc-$VLC_VERSION.0-win64*

    # fix paths in .pc files
    sed -i "" "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
    export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
    cd lib
    ln -sf libvlccore.*.dylib libvlccore.dylib
    cd ..
    cd ../..
    make OS=macOS

    mkdir artifacts
    zip -j artifacts/vlc-$VLC_VERSION-macosx-intel64.zip libpause_click_plugin.dylib
  fi
fi

exit 0
