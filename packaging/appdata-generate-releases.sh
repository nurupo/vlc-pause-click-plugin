#!/usr/bin/env bash
set -euo pipefail

echo "  <releases>"
TZ=UTC0 git tag -l --sort=-creatordate --format='%(creatordate:short) %(refname:short)' | \
  awk '{print "    <release version=\""$2"\" date=\""$1"\"><url>https://github.com/nurupo/vlc-pause-click-plugin/releases/tag/"$2"</url></release>"}'
echo "  </releases>"
