#!/usr/bin/env bash
set -euo pipefail

build()
{
    VERSION=$1
    BITS=$2
    if [[ $BITS == "32" ]]
    then
        TOOLCHAIN=i686-w64-mingw32
    elif [[ $BITS == "64" ]]
    then
        TOOLCHAIN=x86_64-w64-mingw32
    fi
    DESTDIR="/build/$VERSION/$BITS"

    cd /vlc-$VERSION.0-win$BITS/*/sdk
    sed -i "s|^prefix=.*|prefix=$PWD|g" lib/pkgconfig/*.pc
    export PKG_CONFIG_PATH="${PWD}/lib/pkgconfig"
    if [ ! -f lib/vlccore.lib ]
    then
        echo "INPUT(libvlccore.lib)" > lib/vlccore.lib
    fi

    cd /repo
    make CC=$TOOLCHAIN-gcc LD=$TOOLCHAIN-ld OS=Windows
    $TOOLCHAIN-strip libpause_click_plugin.dll

    mkdir -p $DESTDIR
    cp libpause_click_plugin.dll $DESTDIR
    chmod 777 -R "/build/$VERSION"

    make clean OS=Windows

    cd /
}

if [[ "$1" == "all" ]]
then
    build 2.1 32
    build 2.1 64
    build 2.2 32
    build 2.2 64
    build 3.0 32
    build 3.0 64
else
    VERSION=$1
    BITS=$2

    if [ -z "$VERSION" ]
    then
        echo "Error: No VLC version was specified. Please specify either '2.1', '2.2' or '3.0', as the first argument to the script."
        exit 1
    fi

    if [[ "$VERSION" != "2.1" ]] && [[ "$VERSION" != "2.2" ]] && [[ "$VERSION" != "3.0" ]]
    then
        echo "Error: Incorrect VLC version was specified. Please specify either '2.1', '2.2' or '3.0', as the first argument to the script."
        exit 1
    fi

    if [ -z "$BITS" ]
    then
        echo "Error: No bitness was specified. Please specify either '32' or '64', as the second argument to the script."
        exit 1
    fi

    if [[ "$BITS" != "32" ]] && [[ "$BITS" != "64" ]]
    then
        echo "Error: Incorrect bitness was specified. Please specify either '32' or '64', as the second argument to the script."
        exit 1
    fi

    build $VERSION $BITS
fi
