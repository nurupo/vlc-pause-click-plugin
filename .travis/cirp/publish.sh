#!/usr/bin/env bash

set -euo pipefail

. .travis/cirp/check_precondition.sh

if [ ! -z "$TRAVIS_TEST_RESULT" ] && [ "$TRAVIS_TEST_RESULT" != "0" ]; then
  echo "Build has failed, skipping publishing"
  exit 0
fi

if [ "$#" != "1" ]; then
  echo "Error: No arguments provided. Please specify a directory containing artifacts as the first argument."
  exit 1
fi

ARTIFACTS_DIR="$1"

. .travis/cirp/install.sh

echo "This is an auto-generated release based on [Travis-CI build #${TRAVIS_BUILD_ID}](${TRAVIS_BUILD_WEB_URL})

---

For a better compatibility, use plugins only with the specified versions of nightly VLC

| Plugin | Nightly VLC Download Link |
|--------|---------------------------|" > RELEASE_BODY.txt

for ARTIFACT in "$ARTIFACTS_DIR"/*.zip; do
  echo "| \`$(basename -- $ARTIFACT)\` | $(unzip -p ${ARTIFACT} VLC_DOWNLOAD_URL.txt) |" >> RELEASE_BODY.txt
done

ci-release-publisher publish --latest-release \
                             --latest-release-prerelease \
                             --latest-release-body "$(cat RELEASE_BODY.txt)" \
                             --latest-release-check-event-type cron \
                             --numbered-release \
                             --numbered-release-keep-count 60 \
                             --numbered-release-prerelease \
                             --numbered-release-body "$(cat RELEASE_BODY.txt)" \
                             "$ARTIFACTS_DIR"
