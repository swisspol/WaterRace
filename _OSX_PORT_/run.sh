#!/bin/sh
# Build and run the macOS port. The app loads Assets/ and Music.mp3 from the
# current directory, so run it from the package directory.
# Usage: ./run.sh [characters|boats|levels] [-test]
set -e
cd "$(dirname "$0")/MacOS"
exec swift run MacOS "$@"
