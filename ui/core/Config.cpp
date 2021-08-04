
#include "core/Config.hpp"

#include <QSettings>

Config::Config() :
    mAppearance(),
    mGeneral(),
    //mKeyboard(),
    mMidi(),
    mSound(),
    mPalette(),
    mPianoInput()
{
}

AppearanceConfig& Config::appearance()              { return mAppearance; }
AppearanceConfig const& Config::appearance() const  { return mAppearance; }

GeneralConfig& Config::general()                    { return mGeneral; }
GeneralConfig const& Config::general() const        { return mGeneral; }

// KeyboardConfig& Config::keyboard()                  { return mKeyboard; }
// KeyboardConfig const& Config::keyboard() const      { return mKeyboard; }

MidiConfig& Config::midi()                          { return mMidi; }
MidiConfig const& Config::midi() const              { return mMidi; }

SoundConfig& Config::sound()                        { return mSound; }
SoundConfig const& Config::sound() const            { return mSound; }

Palette& Config::palette()                          { return mPalette; }
Palette const& Config::palette() const              { return mPalette; }

PianoInput& Config::pianoInput()                    { return mPianoInput; }
PianoInput const& Config::pianoInput() const        { return mPianoInput; }

void Config::readSettings() {
    QSettings settings;

    mAppearance.readSettings(settings);
    mGeneral.readSettings(settings);
    mMidi.readSettings(settings);
    mSound.readSettings(settings);
    mPalette.readSettings(settings);
    mPianoInput.readSettings(settings);
}


void Config::writeSettings() {
    QSettings settings;

    mAppearance.writeSettings(settings);
    mGeneral.writeSettings(settings);
    mMidi.writeSettings(settings);
    mSound.writeSettings(settings);
    mPalette.writeSettings(settings);
    mPianoInput.writeSettings(settings);
}

void Config::disableMidi() {
    mMidi.setEnabled(false);
}
