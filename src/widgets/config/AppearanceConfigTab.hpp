
#pragma once

#include "core/model/PaletteModel.hpp"
#include "widgets/config/ConfigTab.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QDir>
#include <QModelIndex>
#include <QPushButton>

#include <array>

class AppearanceConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit AppearanceConfigTab(QWidget *parent = nullptr);

    void apply(AppearanceConfig &appearanceConfig, Palette &pal);

    void resetControls(AppearanceConfig const& appearanceConfig, Palette const& pal);

protected:

    virtual void hideEvent(QHideEvent *evt) override;

private slots:

    void chooseFont();

    void chooseColor(QColor const& color);

private:
    Q_DISABLE_COPY(AppearanceConfigTab)

    void setFont(size_t index, QFont const& font);

    void selectColor(QModelIndex const& index);

    void updateColorDialog();

    static constexpr size_t FONT_COUNT = 3;
    std::array<QPushButton*, FONT_COUNT> mFontChooseButtons;
    std::array<QFont, FONT_COUNT> mFonts;

    QCheckBox *mShowFlatsCheck;
    QCheckBox *mShowPreviewsCheck;

    QPushButton *mDefaultButton;

    PaletteModel *mModel;

    QColorDialog *mColorDialog;

    QDir mSaveDir;

    QModelIndex mSelectedColor;

};
