FROM debian:stretch-slim

RUN apt-get update && \
    apt-get install -y \
        gcc-mingw-w64-i686 \
        gcc-mingw-w64-x86-64 \
        lynx \
        make \
        p7zip \
        pkg-config \
        wget && \
    wget https://download.videolan.org/pub/videolan/vlc/2.1.0/win32/vlc-2.1.0-win32.7z -O vlc-2.1.0-win32.7z && \
    wget https://download.videolan.org/pub/videolan/vlc/2.1.0/win64/vlc-2.1.0-win64.7z -O vlc-2.1.0-win64.7z && \
    wget https://download.videolan.org/pub/videolan/vlc/2.2.0/win32/vlc-2.2.0-win32.7z -O vlc-2.2.0-win32.7z && \
    wget https://download.videolan.org/pub/videolan/vlc/2.2.0/win64/vlc-2.2.0-win64.7z -O vlc-2.2.0-win64.7z && \
    wget https://download.videolan.org/pub/videolan/vlc/3.0.0/win32/vlc-3.0.0-win32.7z -O vlc-3.0.0-win32.7z && \
    wget https://download.videolan.org/pub/videolan/vlc/3.0.0/win64/vlc-3.0.0-win64.7z -O vlc-3.0.0-win64.7z && \
    7zr x "*.7z" -o* "*/sdk" && \
    rm *.7z && \
    apt-get clean && \
    apt-get autoremove -y && \
    rm -rf \
        /var/lib/apt/lists/* \
        /tmp/* \
        /var/tmp/*

ADD script.sh .

ENTRYPOINT ["/bin/bash", "script.sh"]
