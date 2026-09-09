![Trackerboy](/src/resources/images/logo.png)
---

[![build-lib][build-badge]][build-link]
[![Discord](https://img.shields.io/discord/770034905231917066?svg=true)](https://discord.gg/m6wcAK3)


Trackerboy is a tracker program for producing music for the gameboy / gameboy color
consoles.

WIP! This project is currently in the development phase (major version 0).

If you have any questions or would like to contribute, feel free to contact me on discord.
My tag is stoneface#7646 or you can join the server, https://discord.gg/m6wcAK3

## Getting Started

 * __Build:__ To compile Trackerboy from source, see the [build instructions](BUILD.md).
 * __Contribute:__ For contributing to this repo, see the [contributing guidelines](CONTRIBUTING.md) and [source code organization](ORGANIZATION.md).
 * __Use:__ For details on how to use the tracker, see the [manual](https://www.trackerboy.org/manual).

## Command line

Trackerboy can also export a module to WAV from the command line, without
starting the editor (no display is required):

```sh
# export the whole song to a single file
trackerboy song.tbm --export-wav song.wav

# export channels 1, 2 and 4, each to its own file: out/song.ch1.wav, out/song.ch2.wav, ...
trackerboy song.tbm --export-wav out/ --separate --channels 1,2,4

# play the second song of the module 3 times, at 48000 Hz
trackerboy song.tbm --export-wav song.wav --song 2 --loops 3 --samplerate 48000

# play for 1 minute and 30 seconds instead of looping
trackerboy song.tbm --export-wav song.wav --duration 01:30
```

Run `trackerboy --help` for the full list of options.

## Status

[v0.6.3](https://github.com/stoneface86/trackerboy/releases/tag/v0.6.3) is now available.

![Demo](/.github/screenshot.png "Trackerboy application demo")

## Versioning

This project uses Semantic Versioning v2.0.0

## Authors

 * stoneface ([@stoneface86](https://github.com/stoneface86)) - Owner

# License

This project is licensed under the MIT License - See [LICENSE](LICENSE) for more details

[build-badge]: https://github.com/stoneface86/trackerboy/workflows/build/badge.svg
[build-link]: https://github.com/stoneface86/trackerboy/actions?query=workflow%3Abuild
