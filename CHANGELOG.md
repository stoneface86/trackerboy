# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
