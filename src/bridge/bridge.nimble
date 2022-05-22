# Package

# this package will never be published/installed so
# there's no need to update this metadata
version       = "0.1.0"
author        = "stoneface"
description   = "Trackerboy C to Nim Bridge"
license       = "MIT"


# Dependencies

requires "nim >= 1.6.0"
# libtrackerboy, installed via cmake to a separate nimbleDir
# once libtrackerboy is published we can just grab it from the official repository
requires "trackerboy"
