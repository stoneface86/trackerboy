
#include "config/Config.hpp"

#include <QSettings>

Config::Config() :
    mFonts(),
    mGeneral(),
    mMidi(),
    mSound(),
    mPalette(),
    mPianoInput(),
    mShortcuts()
{
}

FontTable& Config::fonts()                          { return mFonts; }
FontTable const& Config::fonts() const              { return mFonts; }

GeneralConfig& Config::general()                    { return mGeneral; }
GeneralConfig const& Config::general() const        { return mGeneral; }

ShortcutTable& Config::shortcuts()                  { return mShortcuts; }
ShortcutTable const& Config::shortcuts() const      { return mShortcuts; }

MidiConfig& Config::midi()                          { return mMidi; }
MidiConfig const& Config::midi() const              { return mMidi; }

SoundConfig& Config::sound()                        { return mSound; }
SoundConfig const& Config::sound() const            { return mSound; }

Palette& Config::palette()                          { return mPalette; }
Palette const& Config::palette() const              { return mPalette; }

PianoInput& Config::pianoInput()                    { return mPianoInput; }
PianoInput const& Config::pianoInput() const        { return mPianoInput; }

void Config::readSettings(AudioEnumerator &audio, MidiEnumerator &midi) {
    QSettings settings;

    mFonts.readSettings(settings);
    mGeneral.readSettings(settings);
    mMidi.readSettings(settings, midi);
    mSound.readSettings(settings, audio);
    mPalette.readSettings(settings);
    mPianoInput.readSettings(settings);
    mShortcuts.readSettings(settings);
}


void Config::writeSettings(AudioEnumerator const& audio, MidiEnumerator const& midi) {
    QSettings settings;

    mFonts.writeSettings(settings);
    mGeneral.writeSettings(settings);
    mMidi.writeSettings(settings, midi);
    mSound.writeSettings(settings, audio);
    mPalette.writeSettings(settings);
    mPianoInput.writeSettings(settings);
    mShortcuts.writeSettings(settings);
}
