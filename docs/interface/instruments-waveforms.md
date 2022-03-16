
# The instruments/waveforms panel

![panel](img/instruments-waveforms.png "Instruments/Waveforms panel")

This panel shows all of your instruments and waveforms contained in the current
module. It is located to the right of the [pattern editor](pattern-editor.md).

You can have up to 64 instruments and 64 waveforms in each module. Each item can
be referenced by its 6-bit ID. For instruments this ID is used in the instrument
column in the pattern editor. For waveforms this ID is used as a parameter for
the [Exx](../tracker/effect-list.md#exx-set-envelope) effect.

When an item is selected the instrument/waveform editor will switch to that item.
You can also double-click an item to open the editor.

When an instrument is selected, the pattern editor will automatically insert its
ID into the instrument column when entering notes. To prevent this from happening,
deselect the instrument by clicking on it while holding the `CTRL` key.

## Toolbar

The toolbar contains actions for adding/removing instruments or waveforms. Note
that all actions in this toolbar permanently modify the module and CANNOT be
undone.

### Add

Adds an instrument/waveform to the module. The item uses the lowest available
ID.

### Remove

Removes the selected item from the module.

### Duplicate

Makes a copy of the selected item. Like [Add](#add), the copied item uses the
lowest available ID.

### Import

This feature is currently not available.

### Export

This feature is currently not available.

### Edit

Opens the editor for the selected item.
