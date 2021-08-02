
#pragma once

#include "core/Palette.hpp"

#include <QFont>
#include <QSettings>

#include <optional>

class AppearanceConfig {

public:

    AppearanceConfig();

    bool showFlats() const;

    bool showPreviews() const;

    QFont font() const;
    QFont& font();

    Palette const& palette() const;
    Palette& palette();

    void setShowFlats(bool showFlats);

    void setShowPreviews(bool showPreviews);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    std::optional<Palette> mPalette; // optional is used for lazy loading
    QFont mFont;
    bool mShowFlats;         // if true flats will be shown for accidental notes
    bool mShowPreviews;      // if true, pattern previews will be rendered

};
