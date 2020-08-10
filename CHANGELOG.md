# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [0.0.6] - 2020-08-09
### Added
 - song_demo, demonstrates playback of a Song
 - Engine class, handles playback for Songs and sound effects
 - FrequencyControl, frequency effect logic for channels 1, 2 and 3
 - Note lookup table for channel 4, NOTE_NOISE_TABLE
 - MusicRuntime, handles song playback (replaces SongRuntime + PatternRutime + TrackRuntime)
 - NoteControl, handles note triggers and cuts with an optional delay.
 - PatternCursor, utility class for random access/iteration of a Pattern
 - RuntimeContext struct, utility struct containing Synth and Table references
 - Timer class, used for tempo/speed
### Changed
 - Fixed warnings for ubuntu build
 - Mixer no longer uses sigma approximation when calculating the STEP_TABLE, this was done as
   an experiment and also worked as a low pass filter. Filtering will be done elsewhere in the future.
 - added utility method Synth::setFrequency
 - added several new effects. Effects are now categorized by pattern, track and frequency.
### Removed
 - InstrumentRuntime, TrackRuntime, PatternRuntime and SongRuntime classes. These are being replaced
   with the newly added classes in the engine folder.
 - Q53.hpp, replaced by Speed.hpp

## [0.0.5] - 2020-04-24
### Added
 - PatternMaster class, container for all tracks in a Song
 - Synth::reset() and for all APU components also
### Changed
 - Instruments are now a sequence of commands, 1 command per frame
 - All runtime classes have been rewritten to use the new pattern format
 - Song serialization has been rewritten to use new pattern format
 - File format signature changed so that it starts with a non-printable character
 - TrackRuntime and InstrumentRuntime are now template classes
 - Patterns function like FamiTracker's, a pattern is a combination of resuable track data
 - Pattern is now a tuple of Track references
 - Order is now a POD struct of track ids, one for each channel
 - Track no longer behaves like an iterator and now contains data
 - pattern demo now plays a recreation of national park from pokemon G/S/C
 - implemented writeAll method in PlaybackQueue
 - PlaybackQueue uses standard library for sleeping instead of using Pa_Sleep
 - Synth methods now have noexcept and const qualifiers where applicable
 - Fix issue in Wav writer, where the sample count in the fact chunk was
   incorrect for multichannel data.
 - The Mixer and Synth sampling rates can now be changed
 - SongRuntime is now single-use, or, you can no longer set a new song
### Removed
 - Instruction struct, no longer needed
 - PatternRuntime::reset()

## [0.0.4] - 2020-04-01
### Added
 - Wav class, for writing wav files (only supports 32-bit float format)
 - Generator class, base class for PulseGen, WaveGen and NoiseGen
 - PulseGen class, similar to the previously removed PulseChannel
 - WaveGen class, similar to the previously removed WaveChannel
 - Mixer class, adds bandlimited steps to left/right channels
 - Sequencer class, similar to the previously removed one
### Changed
 - Minimum required C++ standard is now C++17
 - PlaybackQueue uses float samples instead of int16_t
 - Synth was rewritten to use emulation based synthesis, generates samples on
   a per frame basis.
 - Synth can now read and write APU registers
 - reset() renamed to restart() in synth APU components
 - setRegister renamed to writeRegister
 - Sweep has its own shadow register and now longer uses the channel's on trigger
   (this behavior now matches the hardware)
 - Sweep and Envelope register contents get applied on restart
### Removed
 - include/trackerboy/synth/utils.hpp, no longer being used
 - libtrackerboy/synth/test_Osc.cpp, was unused
 - Osc, PulseOsc and WaveOsc, since we are now using emulation based synthesis

## [0.0.3] - 2020-03-09
### Added
 - More test cases for File
 - tests for `Table<T>`
 - misc/genstep.py, similar to gensinc.py, generates bandlimited step sets for Osc
 - implementation for WaveOsc
 - NoiseGen, buffered version of NoiseChannel
 - Envelope, moved code from EnvChannel here (will replace EnvChannel)
 - Sweep class, replaces SweepPulseChannel
 - HardwareFile struct (replaces ChannelFile)
### Changed
 - Fix exception being thrown when attempting to insert into table of size one less than maximum
 - Rewrote Osc class. Subclasses must edit the delta buffer manually instead of
   using `deltaSet`.
 - Osc no longer uses sinc sets. It uses bandlimited steps instead, similiar to
   blargg's http://www.slack.net/~ant/bl-synth/11.implementation.html. `Osc::generate`
   uses floats for samples instead of int16_t. Samples range from -1.0-1.0, exceeding both
   limits due to overshoot (so volume will need to be adjusted or clipping will occur).
 - Mixer no longer emulates terminal volume/panning (tracker doesn't use it)
 - Mixer works with sample buffers as opposed to individual samples from each channel
 - Length counters are no longer emulated by the Synth
 - Mixer and Sequencer were moved into Synth, as only the synth used these internally.
### Removed
  - `Osc::deltaSet` as only WaveOsc needed this method
  - `Osc::setMute` and `Osc::muted`, replaced by `Osc::disable` and `Osc::disabled`
  - misc/gensinc.py no longer needed, replaced by genstep.py
  - Channel, EnvChannel, PulseChannel, SweepPulseChannel, WaveChannel, NoiseChannel,
    ChannelFile, Mixer and Sequencer classes

## [0.0.2] - 2020-02-05
### Added
 - `File::loadTable<T>` method
 - uint16_t overload for correctEndian
 - Destructor and copy constructor to `Table<T>`
 - `Table<T>::insert` methods
 - `Table<T>::clear`
 - Version struct + operator overloads
### Changed
 - Rewrote implementation for `Table<T>`, no longer uses `std::unordered_map`
   Uses a vector for the item data, and a 256 uint8_t array that maps an id
   to an index in the vector. Allowing for faster lookups but a slight
   performance loss when removing items.
 - Refactored pattern_demo.cpp and File.cpp to use new Table implementation
 - table size is now a 2 byte field when saving/loading
 - add setSpeed() overload to calculate speed from tempo/rpb settings. Song
   no longer calculates speed when tempo or rpb is set.
### Removed
 - `Table<T>::add`, `Table<T>::set`, as these methods are no longer needed as the
   insert methods should be used instead.

## [0.0.1] - 2020-02-01
### Added
 - This CHANGELOG.md file to serve as a changelog for any new features, removals
   deprecations and so on for the project as a whole.
### Changed
 - README.md, added a roadmap section to list planned features and the order
   in which they are worked on.
 - ORGANIZATION.md, added a guideline to use unix style newlines in all files
 - CMakeLists.txt, updated project version to v0.0.1
