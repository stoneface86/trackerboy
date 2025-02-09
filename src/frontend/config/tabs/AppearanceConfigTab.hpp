
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
        FontTable &fonts,
        Palette &pal,
        QWidget *parent = nullptr
    );

private:

    Q_DISABLE_COPY(AppearanceConfigTab)

    void chooseFont();

    void chooseColor(QColor const& color);

    void updateFontButton(int index);

    void selectColor(QModelIndex const& index);

    void updateColorDialog();

    FontTable &mFonts;
    std::array<QPushButton*, FontTable::Count> mFontChooseButtons;

    QPushButton *mDefaultButton;

    PaletteModel *mModel;

    QColorDialog *mColorDialog;

    QDir mSaveDir;

    QModelIndex mSelectedColor;

};
