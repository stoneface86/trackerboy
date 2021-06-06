
#include "core/Config.hpp"

#include "gbapu.hpp"

#include <QSettings>


namespace {

constexpr int DEFAULT_SAMPLERATE_INDEX = 4; // 44100 Hz
constexpr int DEFAULT_PERIOD = 5;
constexpr int DEFAULT_LATENCY = 40;
constexpr int DEFAULT_QUALITY = static_cast<int>(gbapu::Apu::Quality::medium);
constexpr unsigned DEFAULT_HISTORY_LIMIT = 64;

Qt::Key const DEFAULT_PIANO_BINDINGS[] = {

    // base octave
    Qt::Key_Q,              // C
    Qt::Key_W,
    Qt::Key_E,
    Qt::Key_R,
    Qt::Key_T,
    Qt::Key_Y,
    Qt::Key_U,
    Qt::Key_I,
    Qt::Key_O,
    Qt::Key_P,
    Qt::Key_BracketLeft,
    Qt::Key_BracketRight,   // B

    // base octave + 1

    Qt::Key_A,              // C
    Qt::Key_S,
    Qt::Key_D,
    Qt::Key_F,
    Qt::Key_G,
    Qt::Key_H,
    Qt::Key_J,
    Qt::Key_K,
    Qt::Key_L,
    Qt::Key_Semicolon,
    Qt::Key_Apostrophe,
    Qt::Key_Backslash,      // B

    // base octave + 2

    Qt::Key_Z,              // C
    Qt::Key_X,
    Qt::Key_C,
    Qt::Key_V,
    Qt::Key_B,
    Qt::Key_N,
    Qt::Key_M,
    Qt::Key_Comma,
    Qt::Key_Period,
    Qt::Key_Slash,          // A
    Qt::Key_unknown,
    Qt::Key_unknown,
    Qt::Key_QuoteLeft      // note cut
};

}


Config::Config(Miniaudio &miniaudio) :
    mMiniaudio(miniaudio),
    mAppearance(),
    mKeyboard(),
    mSound()
{
}

Config::Appearance const& Config::appearance() {
    return mAppearance;
}

Config::General const& Config::general() {
    return mGeneral;
}

Config::Keyboard const& Config::keyboard() {
    return mKeyboard;
}

Config::Sound const& Config::sound() {
    return mSound;
}

void Config::readSettings() {
    QSettings settings;


    settings.beginGroup("config");

    settings.beginGroup(QStringLiteral("appearance"));

    mAppearance.font.setFamily(settings.value("fontFamily", "Cascadia Mono").toString());
    mAppearance.font.setPointSize(settings.value("fontSize", 12).toInt());

    settings.beginReadArray("colors");
    readColor(settings, Color::background,              QColor(0x18, 0x18, 0x18));
    readColor(settings, Color::backgroundHighlight1,    QColor(0x20, 0x20, 0x20));
    readColor(settings, Color::backgroundHighlight2,    QColor(0x30, 0x30, 0x30));
    readColor(settings, Color::foreground,              QColor(0xC0, 0xC0, 0xC0));
    readColor(settings, Color::foregroundHighlight1,    QColor(0xF0, 0xF0, 0xF0));
    readColor(settings, Color::foregroundHighlight2,    QColor(0xFF, 0xFF, 0xFF));
    readColor(settings, Color::row,                     QColor(0x20, 0x30, 0x80));
    readColor(settings, Color::rowEdit,                 QColor(0x80, 0x20, 0x20));
    readColor(settings, Color::rowPlayer,               QColor(0x80, 0x20, 0x80));
    readColor(settings, Color::effectType,              QColor(0x80, 0x80, 0xFF));
    readColor(settings, Color::instrument,              QColor(0x80, 0xFF, 0x80));
    readColor(settings, Color::selection,               QColor(0x45, 0x45, 0x50));
    readColor(settings, Color::cursor,                  QColor(192, 192, 192));
    readColor(settings, Color::line,                    QColor(64, 64, 64));
    readColor(settings, Color::headerBackground,        QColor(224, 248, 208));
    readColor(settings, Color::headerForeground,        QColor(8, 24, 32));
    readColor(settings, Color::headerHover,             QColor(136, 192, 112));
    readColor(settings, Color::headerDisabled,          QColor(52, 104, 86));
    settings.endArray();
   
    mAppearance.showFlats = settings.value("showFlats", false).toBool();
    mAppearance.showPreviews = settings.value("showPreviews", true).toBool();

    settings.endGroup(); // appearance

    settings.beginGroup(QStringLiteral("general"));
    mGeneral.historyLimit = settings.value(QStringLiteral("historyLimit"), DEFAULT_HISTORY_LIMIT).toUInt();
    settings.endGroup(); // general

    settings.beginGroup(QStringLiteral("keyboard"));

    // keybindings for the piano widget + note column in pattern editor
    settings.beginReadArray("piano");
    for (int i = 0; i != sizeof(DEFAULT_PIANO_BINDINGS) / sizeof(Qt::Key); ++i) {
        settings.setArrayIndex(i);
        mKeyboard.pianoInput.bind(
            static_cast<Qt::Key>(settings.value("keycode", DEFAULT_PIANO_BINDINGS[i]).toInt()),
            i
        );
    }
    settings.endArray();

    settings.endGroup(); // keyboard

    settings.beginGroup(QStringLiteral("sound"));
    
    QByteArray id = settings.value("deviceId").toByteArray();

    // default device
    mSound.deviceIndex = 0;

    if (id.size() == sizeof(ma_device_id)) {

        int index = mMiniaudio.lookupDevice(reinterpret_cast<ma_device_id*>(id.data()));
        if (index != -1) {
            mSound.deviceIndex = index;
        }
        // if we don't find the device we will go back to the default device
        // TODO: let the user know via messagebox if this occurs
    }

    mSound.samplerateIndex = settings.value("samplerateIndex", DEFAULT_SAMPLERATE_INDEX).toUInt();
    mSound.latency = settings.value("latency", DEFAULT_LATENCY).toInt();
    mSound.period = settings.value("period", DEFAULT_PERIOD).toInt();
    mSound.quality = settings.value("quality", DEFAULT_QUALITY).toInt();

    settings.endGroup(); // sound

    settings.endGroup(); // config
}

void Config::readColor(QSettings &settings, Color color, QColor def) {
    settings.setArrayIndex(+color);
    mAppearance.colors[+color] = settings.value("color", def).value<QColor>();
}


void Config::writeSettings() {
    QSettings settings;
    settings.beginGroup("config");

    settings.beginGroup("appearance");
    settings.setValue("fontFamily", mAppearance.font.family());
    settings.setValue("fontSize", mAppearance.font.pointSize());
    settings.beginWriteArray("colors", (int)mAppearance.colors.size());
    for (int i = 0; i != mAppearance.colors.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("color", mAppearance.colors[i]);
    }
    settings.endArray();
    settings.setValue("showFlats", mAppearance.showFlats);
    settings.setValue("showPreviews", mAppearance.showPreviews);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("general"));
    settings.setValue("historyLimit", mGeneral.historyLimit);
    settings.endGroup();

    settings.beginGroup("keyboard");
    settings.beginWriteArray("piano");
    auto &bindings = mKeyboard.pianoInput.bindings();
    for (size_t i = 0; i != bindings.size(); ++i) {
        settings.setArrayIndex((int)i);
        settings.setValue("keycode", bindings[i]);
    }
    settings.endArray();
    settings.endGroup();

    settings.beginGroup("sound");
    QByteArray barray;
    if (mSound.deviceIndex != 0) {
        ma_device_id *id = mMiniaudio.deviceId(mSound.deviceIndex);
        if (id != nullptr) {
            barray = QByteArray(reinterpret_cast<char*>(id), sizeof(ma_device_id));
        }
    }
    settings.setValue("deviceId", barray);

    settings.setValue("samplerateIndex", mSound.samplerateIndex);
    settings.setValue("latency", mSound.latency);
    settings.setValue("period", mSound.period);
    settings.setValue("quality", mSound.quality);
    settings.endGroup();

    settings.endGroup();
}
