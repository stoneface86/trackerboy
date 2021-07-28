
# Ui project

This subdirectory contains the ui project. You can build this project via the
root CMakeLists.txt, with `ENABLE_UI` option set to ON. (note: ENABLE_UI is ON
by default).

## Organization

* `ui/`
  * `core/` - Application core internals, non-ui classes
    * `audio/` - audio-related code: rendering, data buffers, device management, etc
    * `clipboard/` - clipboard management and clipboard data classes
    * `midi/` - MIDI handling and device management
    * `model/` - data model classes using Qt Model/View framework
  * `forms/` - Form/dialog classes
  * `misc/` - miscellaneous/utility code
  * `resources/` - images, Qt resource files (.qrc), etc
    * `icons/` - image files for QIcons (see misc/IconManager.hpp)
    * `images/` - misc image files for QWidget subclasses
  * `widgets/` - QWidget subclasses and custom widgets/controls
    * `config/` - ConfigTab subclasses (each tab in ConfigDialog gets its own
                  widget subclass here).
    * `docks/` - QWidget subclasses that are contained in a QDockWidget
    * `grid/` - PatternEditor internals
    * `sidebar/` - QWidget subclasses for the Sidebar widget

