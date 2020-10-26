# Trackerboy

[![build-lib][build-lib-badge]][build-lib-link]
[![tests][tests-badge]][tests-link]
[![codecov](https://codecov.io/gh/stoneface86/trackerboy/branch/develop/graph/badge.svg)](https://codecov.io/gh/stoneface86/trackerboy)
[![Discord](https://img.shields.io/discord/770034905231917066?svg=true)](https://discord.gg/m6wcAK3)


Trackerboy is a tracker program for producing music for the gameboy / gameboy color
consoles.

WIP! This project is currently in the development phase (major version 0).

## Getting Started

 * __Build:__ To compile Trackerboy from source, see the [build instructions](BUILD.md)
 * __Contribute:__ For contributing to this repo, see the [contributing guidelines](CONTRIBUTING.md) and [source code organization](ORGANIZATION.md)
 * __Use:__ For details on how to use the tracker, see TBD

## Status

### Wave Editor Demo
![Wave Editor Demo](/misc/wave_editor_demo.gif)

The library is (almost) finished and the UI is being worked on now.

Here is a general list of what needs to be completed:
 * Sound effect support in the library (SfxRuntime, Sfx, SfxTable classes)
 * More unit tests for the library
 * Instrument editor
 * Wave editor
 * Pattern editor / Tracker grid
 * Visualizers: Oscilloscopes and volume meters (master and one for each channel)
 * Export to .asm (pattern compilation)
 * Export to .gbs
 * Audio filters for equalization or treble/bass filtering
 * MIDI support
 * Import from famitracker (tool to convert .ftm -> .tbm)

## Versioning

This project uses Semantic Versioning v2.0.0

## Authors

 * stoneface ([@stoneface86](https://github.com/stoneface86)) - Owner

# License

This project is licensed under the MIT License - See [LICENSE](LICENSE) for more details

[build-lib-badge]: https://github.com/stoneface86/trackerboy/workflows/build-lib/badge.svg
[build-lib-link]: https://github.com/stoneface86/trackerboy/actions?query=workflow%3Abuild-lib
[tests-badge]: https://github.com/stoneface86/trackerboy/workflows/tests/badge.svg
[tests-link]: https://github.com/stoneface86/trackerboy/actions?query=workflow%3Atests
