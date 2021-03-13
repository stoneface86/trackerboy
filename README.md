# Trackerboy

[![build-lib][build-lib-badge]][build-lib-link]
[![tests][tests-badge]][tests-link]
[![codecov](https://codecov.io/gh/stoneface86/trackerboy/branch/develop/graph/badge.svg)](https://codecov.io/gh/stoneface86/trackerboy)
[![Discord](https://img.shields.io/discord/770034905231917066?svg=true)](https://discord.gg/m6wcAK3)


Trackerboy is a tracker program for producing music for the gameboy / gameboy color
consoles.

WIP! This project is currently in the development phase (major version 0).

If you have any questions or would like to contribute, feel free to contact me on discord.
My tag is stoneface#7646 or you can join the server, https://discord.gg/m6wcAK3

## Getting Started

 * __Build:__ To compile Trackerboy from source, see the [build instructions](BUILD.md)
 * __Contribute:__ For contributing to this repo, see the [contributing guidelines](CONTRIBUTING.md) and [source code organization](ORGANIZATION.md)
 * __Use:__ For details on how to use the tracker, see TBD

## Status

v0.2.0 is in progress and will be the first "functional" version of trackerboy. This version
will feature a usable pattern editor.

### Pattern Editor Demo
![Pattern Editor Demo](/.github/pattern_editor_demo.gif)

The library is (almost) finished and the UI is being worked on now.

Here is a general list of what needs to be completed:
 * ~~Instrument editor~~
 * ~~Wave editor~~
 * Pattern editor / Tracker grid (In progress)
 * Visualizers: Oscilloscopes and volume meters (master and one for each channel)
 * Instruments rewrite. Similar to FamiTracker's

 Post-release features:
 * Sound effects
 * MIDI support
 * Export to .asm (pattern compilation)
 * Export to .gbs
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
