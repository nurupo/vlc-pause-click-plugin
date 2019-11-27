#!/usr/bin/env bash
set -euxo pipefail

echo_nightly_readme() {
  echo "This plugin was built for a nightly, in-development version of VLC." \
       "The development version of VLC often breaks ABI, making the plugin built for it non-functional in newer VLC builds." \
       "Meaning that a plugin targeting a nightly VLC built today might not work with a nightly VLC built tomorrow." \
       "Thus it's recommended that you use the specific nightly VLC build this plugin was made for: $1" | fold -sw 80
}

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
  sudo apt-get install -y tree zip
  # use our own docker image for testing building of Windows binaries
  # makes sure that our docker image is not broken too
  sudo docker build -t vlc-pause-click-plugin-windows-build docker
  sudo docker run --rm -v `pwd`:/repo -v `pwd`/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 32
  sudo docker run --rm -v `pwd`:/repo -v `pwd`/build:/build vlc-pause-click-plugin-windows-build $VLC_VERSION 64
  tree build -s --si --du
  # there should be exactly two dlls built, one for 32-bit VLC and another for 64-bit
  [ $(find ./build -name "*.dll" | wc -l) = "2" ] || false
  cd build
  echo_nightly_readme "$(cat $VLC_VERSION/32/VLC_DOWNLOAD_URL.txt)" > "$VLC_VERSION/32/README.txt"
  echo_nightly_readme "$(cat $VLC_VERSION/64/VLC_DOWNLOAD_URL.txt)" > "$VLC_VERSION/64/README.txt"
  ./zip-it.sh
  cd ..
  mkdir artifacts
  cp -a build/*.zip artifacts
elif [ "$TARGET" = "macOS" ]; then
  # updating fails sometimes, unable to fetch data off homebrew GitHub repo, so keep retrying
  until brew update; do
    sleep 30
  done
  brew install p7zip
  mkdir -p tmp/tmp
  cd tmp/tmp
  if [ "$VLC_VERSION" = "4.0" ]; then
    brew install lynx
    LATEST_MACOS_DIR_URL=$(lynx -listonly -nonumbers -dump "https://nightlies.videolan.org/build/macosx-intel/" | grep "vlc-$VLC_VERSION.0-" | LC_COLLATE=C sort --stable --ignore-case | tail -n1)
    echo "$LATEST_MACOS_DIR_URL"
    LATEST_MACOS_FILE_URL=$(lynx -listonly -nonumbers -dump "$LATEST_MACOS_DIR_URL" | grep ".dmg$" | tail -n1)
    echo "$LATEST_MACOS_FILE_URL"
    if [ "$(echo "$LATEST_MACOS_FILE_URL" | wc -l | awk '{print $1}')" != "1" ]; then
      echo "Error: Weren't able to find the .dmg file for VLC $VLC_VERSION"
      exit 1
    fi
    brew reinstall wget openssl
    wget "$LATEST_MACOS_FILE_URL"
    7z x "*.dmg" "*/VLC.app/Contents/Frameworks" || true
    mkdir ../lib
    mv */VLC.app/Contents/Frameworks/*.dylib ../lib
    cd ..
    rm -rf ./tmp
    LATEST_WIN64_DIR_URL=$(lynx -listonly -nonumbers -dump "https://nightlies.videolan.org/build/win64/" | grep "vlc-$VLC_VERSION.0-" | LC_COLLATE=C sort --stable --ignore-case | tail -n1)
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
    wget https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/macosx/vlc-$VLC_VERSION.0.dmg
    # extracting contents of a .dmg file using 7-zip is more of a hack, and while it works 7z exits with an error code we want to suppress
    7z x "*.dmg" "*/VLC.app/Contents/MacOS" || true
    mv */VLC.app/Contents/MacOS/lib ..
    mv */VLC.app/Contents/MacOS/include ..
    cd ..
    rm -rf ./tmp
    # in contrast to Windows VLC including a very nice and complete sdk directory,
    # macOS VLC doesn't package .pc files, plugin headers and in 2.1.0 libvlc_version.h is not generated off libvlc_version.h.in.
    # it just packages libvlc.dylib, libvlccore.dylib and libvlc headers.
    # honestly, wtf VLC team, get your game together.
    # we grab the missing pieces from the Windows VLC, there shouldn't be anything platform-specific in those so it should work
    wget https://download.videolan.org/pub/videolan/vlc/$VLC_VERSION.0/win64/vlc-$VLC_VERSION.0-win64.7z -O vlc-$VLC_VERSION.0-win64.7z
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
