
#include <QString>


//
// This namespace contains keys for use in QSettings. Interning is done here
// so we can avoid the runtime cost of converting a c-string to QString, as
// well as reducing code duplication from repeated string literals (which the
// compiler may optimize but is never guaranteed).
//
// Note that the variable names are the same for their corresponding key name
//
// So instead of:
//  foo = settings.value("foo").toInt();
// Do this:
//  foo = settings.value(Keys::foo).toInt();
//
namespace Keys {

// groups
extern QString const Appearance;
extern QString const General;
extern QString const Midi;
extern QString const Palette;
extern QString const PianoInput;
extern QString const Sound;

extern QString const api;
extern QString const bindings;
extern QString const deviceName;
extern QString const enabled;
extern QString const showFlats;
extern QString const showPreviews;
extern QString const font;
extern QString const fontSize;
extern QString const historyLimit;
extern QString const keyboardLayout;
extern QString const key;
extern QString const samplerate;
extern QString const period;
extern QString const latency;
extern QString const deviceId;

}

