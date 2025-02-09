
# src/

This folder contains all the source code for the trackerboy application.

## Organization

* `src/`
    * `audio/` - audio-related code: rendering, data buffers, device management, etc
    * `clipboard/` - clipboard management and clipboard data classes
    * `config/` - configuration releated code: ConfigDialog and Config classes
        * `data/` - internal data classes for the Config class
        * `tabs/` - QWidget subclasses for each tab in the ConfigDialog
    * `core/` - core classes used throughout the source
    * `forms/` - Form/dialog classes
    * `graphics/` - Utility code for use in paint events / QPainter
    * `midi/` - MIDI handling and device management
    * `model/` - data model classes using Qt Model/View framework
        * `commands/` - QUndoCommand subclasses used by model classes
    * `resources/` - images, Qt resource files (.qrc), etc
        * `icons/` - image files for QIcons (see core/misc/IconManager.hpp)
        * `images/` - misc image files for QWidget subclasses
    * `utils/` - miscellaneous/utilities
    * `widgets/` - QWidget subclasses and custom widgets/controls
        * `docks/` - QWidget subclasses that are contained in a QDockWidget
        * `grid/` - PatternEditor internals
        * `sidebar/` - QWidget subclasses for the Sidebar widget

