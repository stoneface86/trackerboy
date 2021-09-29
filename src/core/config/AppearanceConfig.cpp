
#include "core/config/AppearanceConfig.hpp"

#include "core/config/keys.hpp"

#include <QMetaEnum>

AppearanceConfig::AppearanceConfig() :
    mPalette(),
    mFont(),
    mShowFlats(false),
    mShowPreviews(true)
{
}

bool AppearanceConfig::showFlats() const {
    return mShowFlats;
}

bool AppearanceConfig::showPreviews() const {
    return mShowPreviews;
}

QFont AppearanceConfig::font() const {
    return mFont;
}

QFont& AppearanceConfig::font() {
    return mFont;
}


void AppearanceConfig::readSettings(QSettings &settings) {

    settings.beginGroup(Keys::Appearance);

    auto fontName = settings.value(Keys::font, QStringLiteral("Cascadia Mono")).toString();
    mFont.setFamily(fontName);
    mFont.setPointSize(settings.value(Keys::fontSize, 12).toInt());

    mShowFlats = settings.value(Keys::showFlats, mShowFlats).toBool();
    mShowPreviews = settings.value(Keys::showPreviews, mShowPreviews).toBool();

    settings.endGroup();

}

void AppearanceConfig::writeSettings(QSettings &settings) const {

    settings.beginGroup(Keys::Appearance);

    settings.setValue(Keys::font, mFont.family());
    settings.setValue(Keys::fontSize, mFont.pointSize());
    settings.setValue(Keys::showFlats, mShowFlats);
    settings.setValue(Keys::showPreviews, mShowPreviews);

    settings.endGroup();
}
