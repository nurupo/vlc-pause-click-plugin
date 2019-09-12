# Install verifying the hash

# Verifying PGP signture on CI is error-prone, keyservers often fail to return
# the key and even if they do, `gpg --verify` returns success with a revoked
# or expired key. Thus it's probably better to verify the signature yourself,
# on your local machine, and then rely on the hash on the CI.

# Get Python >=3.5
if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  until brew update; do
    sleep 30
  done

  # Upgrade Python 2 to Python 3
  brew upgrade python || true

  # Print python versions
  python --version || true
  python3 --version || true
  pyenv versions || true

  # make sha256sum available
  export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # Print python versions
  python --version || true
  python3 --version || true
  pyenv versions || true

  pyenv global 3.6.7
fi

pip3 install --upgrade pip

check_sha256()
{
  if ! ( echo "$1  $2" | sha256sum -c --status - ); then
    echo "Error: sha256 of $2 doesn't match the known one."
    echo "Expected: $1  $2"
    echo -n "Got: "
    sha256sum "$2"
    exit 1
  else
    echo "sha256 matches the expected one: $1"
  fi
}

# Don't install again if already installed.
# OSX keeps re-installing it tough, as it uses a temp per-script virtualenv.
if ! pip3 list --format=columns | grep '^ci-release-publisher '; then
  cd .
  cd "$(mktemp -d)"
  VERSION="0.2.0a4"
  FILENAME="ci_release_publisher-$VERSION-py3-none-any.whl"
  HASH="0d5dff991bad83b2f06bfac8fa5ddd4b9789a076950adc15eb3838a7665bf964"
  pip3 download ci_release_publisher==$VERSION
  check_sha256 "$HASH" "$FILENAME"
  pip3 install --no-index --find-links "$PWD" "$FILENAME"
  cd -
fi
