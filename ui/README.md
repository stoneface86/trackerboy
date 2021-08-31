
# Ui project

This subdirectory contains the ui project. You can build this project via the
root CMakeLists.txt, with `ENABLE_UI` option set to ON. (note: ENABLE_UI is ON
by default).

## Organization

* `src/`
    * `ui/`
    * `core/` - Application core internals, non-ui classes
        * `audio/` - audio-related code: rendering, data buffers, device management, etc
        * `clipboard/` - clipboard management and clipboard data classes
        * `config/` - Config class components (see core/Config.hpp)
        * `graphics/` - Utility code for use in paint events / QPainter
        * `midi/` - MIDI handling and device management
        * `misc/` - miscellaneous/utility code
        * `model/` - data model classes using Qt Model/View framework
    * `forms/` - Form/dialog classes
    * `resources/` - images, Qt resource files (.qrc), etc
        * `icons/` - image files for QIcons (see core/misc/IconManager.hpp)
        * `images/` - misc image files for QWidget subclasses
    * `widgets/` - QWidget subclasses and custom widgets/controls
        * `config/` - ConfigTab subclasses (each tab in ConfigDialog gets its own
                      widget subclass here).
        * `docks/` - QWidget subclasses that are contained in a QDockWidget
        * `grid/` - PatternEditor internals
        * `sidebar/` - QWidget subclasses for the Sidebar widget

