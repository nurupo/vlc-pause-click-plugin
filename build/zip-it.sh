#!/usr/bin/env bash

# zips Windows plugin binaries built by Docker to be uploaded on GitHub

rm vlc-*-win.zip

find ./*/ -type f | while read -r line; do
   zip -j vlc-${line:2:5}-${line:8:2}bit-win.zip $line
done
