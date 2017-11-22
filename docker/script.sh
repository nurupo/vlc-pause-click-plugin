#!/usr/bin/env bash
set -euo pipefail

build()
{
    cp vlc vlc-dirty -R
    cd vlc-dirty
    git checkout $1
    mkdir -p contrib/win32
    cd contrib/win32
    ../bootstrap --host=$2
    cp ../../../vlc-contrib-$2-*.tar.bz2 .
    make prebuilt -j`nproc`
    cd -
    cd modules/video_filter/

    if [[ $1 == *"2.1"* ]]
    then
        cp /repo/pause_click.c .
        cp /repo/vlc_interface-2.1.0-git.h .
        printf "\nlibpause_click_plugin_la_SOURCES = pause_click.c\nlibpause_click_plugin_la_CFLAGS = \$(AM_CFLAGS)\nlibpause_click_plugin_la_LIBADD = \$(AM_LIBADD)\nlibvlc_LTLIBRARIES += libpause_click_plugin.la\n" >> Modules.am
        TARGET="/build/2.1"
        PREFIX="modules/video_filter/.libs/"
    elif [[ $1 == *"2.2"* ]]
    then
        cp /repo/pause_click.c .
        cp /repo/vlc_interface-2.2.0-git.h .
        printf "\nlibpause_click_plugin_la_SOURCES = pause_click.c\nlibpause_click_plugin_la_CFLAGS = \$(AM_CFLAGS)\nlibpause_click_plugin_la_LIBADD = \$(AM_LIBADD)\nvideo_filter_LTLIBRARIES += libpause_click_plugin.la\n" >> Modules.am
        TARGET="/build/2.2"
        PREFIX="modules/video_filter/.libs/"
    fi

    cd -
    ./bootstrap
    export PKG_CONFIG_LIBDIR=${PWD}/contrib/$2/lib/pkgconfig
    mkdir -p win32 && cd win32
    ../extras/package/win32/configure.sh --host=$2 `../extras/package/win32/configure.sh --help | grep "\--enable" | awk '{ print $1 }' | head -n -2 | tail -n +10 | sed 's/--enable/--disable/' | tr '\r\n' ' '` `../extras/package/win32/configure.sh --help | grep "\--disable" | awk '{ print $1 }' | tail -n +5  | tr '\r\n' ' '`
    set +e
    make -j`nproc`
    set -e
    cd ${PREFIX}
    $2-strip ./libpause_click_plugin.dll
    mkdir -p ${TARGET}
    chmod 777 -R ${TARGET}

    if [[ $2 == *"i686"* ]]
    then
        TARGET="${TARGET}/32/"
    else
        TARGET="${TARGET}/64/"
    fi

    mkdir -p ${TARGET}
    cp libpause_click_plugin.dll ${TARGET}
    chmod 777 -R ${TARGET}

    cd /
    rm -rf vlc-dirty
}

if [[ "$1" == "all" ]]
then
    build 2.1.0-git i686-w64-mingw32
    build 2.1.0-git x86_64-w64-mingw32
    build 2.2.0-git i686-w64-mingw32
    build 2.2.0-git x86_64-w64-mingw32
else
    VERSION=$1
    ARCH=$2

    if [ -z "$VERSION" ]
    then
        echo "Error: No VLC version was specified. Please specify either '2.1' or '2.2', as the first argument to the script."
        exit 1
    fi

    if [[ "$VERSION" != "2.1" ]] && [[ "$VERSION" != "2.2" ]]
    then
        echo "Error: Incorrect VLC version was specified. Please specify either '2.1' or '2.2', as the first argument to the script."
        exit 1
    fi

    if [ -z "$ARCH" ]
    then
        echo "Error: No architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the second argument to the script."
        exit 1
    fi

    if [[ "$ARCH" != "i686" ]] && [[ "$ARCH" != "x86_64" ]]
    then
        echo "Error: Incorrect architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the second argument to the script."
        exit 1
    fi

    build $VERSION.0-git $ARCH-w64-mingw32
fi
