FROM debian:jessie-slim

RUN apt-get update && apt-get install -y \
    gcc-mingw-w64-i686 \
    g++-mingw-w64-i686 \
    gcc-mingw-w64-x86-64 \
    g++-mingw-w64-x86-64 \
    mingw-w64-tools \
    wget \
    libtool \
    automake \
    autoconf \
    autopoint \
    make \
    gettext \
    pkg-config \
    bzip2 \
    git && \
    apt-get clean && \
    rm -rf \
        /var/lib/apt/lists/* \
        /tmp/* \
        /var/tmp/* && \
    wget http://ftp.videolan.org/pub/videolan/contrib/i686-w64-mingw32/vlc-contrib-i686-w64-mingw32-20140507.tar.bz2 \
        -O vlc-contrib-i686-w64-mingw32-latest.tar.bz2 && \
    wget http://ftp.videolan.org/pub/videolan/contrib/x86_64-w64-mingw32/vlc-contrib-x86_64-w64-mingw32-20140114.tar.bz2 \
        -O vlc-contrib-x86_64-w64-mingw32-latest.tar.bz2 && \
    git clone https://github.com/videolan/vlc

ADD script.sh .

CMD ./script.sh
