
# The pattern editor

The pattern editor is where you write your song, and is the heart of the
application.

![pattern-editor](../img/interface/pattern-editor.png "The pattern editor")

The editor shows the current pattern, which can be selected from the
[order editor](sidebar.md#order-editor). The first column on the left are row
numbers. Track data for each channel then follows.

Record mode must be enabled to modify pattern data via the keyboard. To toggle
record mode, hit the `Spacebar` or click the [Record](menus/tracker.md#record)
action in the [Tracker](menus/tracker.md) menu. When record mode is enabled,
the cursor line will be red instead of blue.

## Track layout

![pattern-editor-layout](../img/interface/pattern-editor-layout.png "Pattern editor layout")

For any unset column, dashes are displayed. All numberical data in the editor
uses hexadecimal notation.

 - **Note** - Displays the note if one was set. The format of the note shown has
   the letter in the first cell, octave in the last, and separated by a dash
   for natural notes or a `#` or `b` for accidentals. By default sharp
   accidentals, `#`, are displayed (`C#4` instead of `Db4`).  
   Notes can be entered using your keyboard, or a MIDI input device (requires
   configuration in the [Sound / Midi](configuration/sound.md) config). See the
   [Keyboard](configuration/keyboard.md) configuration for more details on the
   default key bindings.
 - **Instrument** - Displays the instrument id if one was set. You do not need
   a note set when setting this column! This way you can change instruments
   mid-note. You can set this column using your keyboard or by entering a note
   with an instrument selected.
 - **Effects** - Displays each effect and parameter. You can have 1-3 effect
   columns displayed for each track. Effects are composed of three characters.
   The first designates the type of effect and the last two are hex nibbles
   that make up the effect's parameter. See the [effect list](../tracker/effect-list.md)
   for all available effects to use.  
   Any unknown effect will be displayed with `?` as the effect type (module was
   made with a newer version of Trackerboy or is corrupted).

You can unset any column by putting the cursor over it and hitting `Del`.
When erasing the note column the instrument column will be erased as well. You
can avoid this by just selecting the note you want to erase instead.

## Controls

 - `Ctrl+A` - Selects all of the current track (first time) or selects the
   entire pattern (second time).
 - `Ctrl+X`, `Ctrl+C`, `Ctrl+V` - cut, copy, paste
 - `Up`, `Left`, `Down`, `Right` - move the cursor, hold `Shift` to select
 - `Tab` - move cursor to next track, `Shift+Tab` moves it to the previous track.
 - `PgUp`, `PgDn` - move cursor up/down by the [page step](configuration/general.md#page-step)
 - `Backspace` - deletes the previous row, shifting all rows ahead up 1.
 - `Del` - erases selection / cursor column

### Mouse

You can also use the mouse to move the cursor by clicking on the position you
want. You can drag the mouse to select an area. Drag this area again to move it
to a new location.

### Selecting rows

Clicking in the row number column selects whole rows. Hold down the mouse to
select multiple rows.

See [shortcuts](shortcuts.md) for more available shortcuts.

## Header

The header allows you to hide/show effect columns for each channel and 
mute/unmute channels.

![pattern-editor-header](../img/interface/pattern-editor-header.png "The header")

### Adding/hiding effects

To add more effect columns, click the plus in the header for the track you want.
Each track can have up to 3 effect columns.

To hide the columns, click the minus instead. You can hide up to 2 columns for
every track. Keep in mind that **effects not shown still get performed by the
tracker**.

### Muting

To mute a channel click its section in the header. To unmute it, click it again.
The activity bar in the header will be green for unmuted and red for muted.

### Solo

To solo a channel double-click its section in the header. Double-click it again
to un-solo.

### Context menu

The header also has a context menu for muting/unmuting and soloing channels. To
use right-click when the mouse is under the channel's section in the header.
