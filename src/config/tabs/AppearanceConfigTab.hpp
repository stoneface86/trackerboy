
#pragma once

#include "config/tabs/ConfigTab.hpp"
class PaletteModel; // implemented in AppearanceConfigTab.cpp

#include <QDir>
class QColorDialog;
#include <QModelIndex>
class QPushButton;

#include <array>

class AppearanceConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit AppearanceConfigTab(
        AppearanceConfig const& appearance,
        Palette const& pal,
        QWidget *parent = nullptr
    );

    void apply(AppearanceConfig &appearanceConfig, Palette &pal);

private slots:

    void chooseFont();

    void chooseColor(QColor const& color);

private:
    Q_DISABLE_COPY(AppearanceConfigTab)

    void setFont(size_t index, QFont const& font, bool showSize = true);

    void selectColor(QModelIndex const& index);

    void updateColorDialog();

    static constexpr size_t FONT_COUNT = 3;
    std::array<QPushButton*, FONT_COUNT> mFontChooseButtons;
    std::array<QFont, FONT_COUNT> mFonts;

    QPushButton *mDefaultButton;

    PaletteModel *mModel;

    QColorDialog *mColorDialog;

    QDir mSaveDir;

    QModelIndex mSelectedColor;

};
