
#include "widgets/config/AppearanceConfigTab.hpp"
#include "core/model/PaletteModel.hpp"
#include "utils/connectutils.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHideEvent>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <algorithm>

#define TU AppearanceConfigTabTU
namespace TU {

static std::array const FONT_NAMES = {
    QT_TR_NOOP("Pattern editor"),
    QT_TR_NOOP("Order editor"),
    QT_TR_NOOP("Header")
};

// file filter for color settings (just ini files)
static const char *COLOR_SETTINGS_FILTER = QT_TR_NOOP("Color settings (*.ini)");

// clamp the size of a selected font to this size
constexpr int MAX_POINT_SIZE = 22;

}

AppearanceConfigTab::AppearanceConfigTab(
        AppearanceConfig const& appearance,
        Palette const& pal,
        QWidget *parent
    ) :
    ConfigTab(parent),
    mColorDialog(nullptr),
    mSaveDir(QDir::home())
{

    auto layout = new QVBoxLayout;

    auto fontGroup = new QGroupBox(tr("Fonts"));
    auto fontLayout = new QGridLayout;

    for (size_t i = 0; i < FONT_COUNT; ++i) {
        auto &chooseBtn = mFontChooseButtons[i];

        fontLayout->addWidget(new QLabel(tr(TU::FONT_NAMES[i])), i, 0);
        chooseBtn = new QPushButton;
        fontLayout->addWidget(chooseBtn, i, 1);

        connect(chooseBtn, &QPushButton::clicked, this, &AppearanceConfigTab::chooseFont);
    }

    fontLayout->setColumnStretch(1, 1);
    fontGroup->setLayout(fontLayout);

    auto generalGroup = new QGroupBox(tr("General"));
    auto generalLayout = new QVBoxLayout;
    mShowFlatsCheck = new QCheckBox(tr("Show flats instead of sharps"));
    generalLayout->addWidget(mShowFlatsCheck);
    mShowPreviewsCheck = new QCheckBox(tr("Show pattern previews"));
    generalLayout->addWidget(mShowPreviewsCheck);
    generalGroup->setLayout(generalLayout);

    auto colorGroup = new QGroupBox(tr("Colors"));
    auto colorLayout = new QVBoxLayout;
    auto colorTree = new QTreeView;
    colorLayout->addWidget(colorTree, 1);
    auto colorButtonLayout = new QHBoxLayout;
    auto loadButton = new QPushButton(tr("Load"));
    colorButtonLayout->addWidget(loadButton);
    auto saveButton = new QPushButton(tr("Save"));
    colorButtonLayout->addWidget(saveButton);
    mDefaultButton = new QPushButton(tr("Default"));
    colorButtonLayout->addWidget(mDefaultButton);
    colorButtonLayout->addStretch();
    auto pickerButton = new QPushButton(tr("Color picker..."));
    colorButtonLayout->addWidget(pickerButton);

    colorLayout->addLayout(colorButtonLayout);
    colorGroup->setLayout(colorLayout);

    layout->addWidget(fontGroup);
    layout->addWidget(generalGroup);
    layout->addWidget(colorGroup);
    layout->addStretch();
    setLayout(layout);

    mModel = new PaletteModel(this);
    colorTree->setModel(mModel);
    colorTree->expandAll();
    colorTree->resizeColumnToContents(0);
    colorTree->collapseAll();

    setFont(0, appearance.patternGridFont());
    setFont(1, appearance.orderGridFont());
    setFont(2, appearance.patternGridHeaderFont(), false);

    mShowFlatsCheck->setChecked(appearance.showFlats());
    mShowPreviewsCheck->setChecked(appearance.showPreviews());

    mModel->setPalette(pal);
    mDefaultButton->setEnabled(!pal.isDefault());


    lazyconnect(mShowFlatsCheck, toggled, this, setDirty<Config::CategoryAppearance>);
    lazyconnect(mShowPreviewsCheck, toggled, this, setDirty<Config::CategoryAppearance>);

    connect(mModel, &PaletteModel::dataChanged, this,
        [this]() {
            mDefaultButton->setEnabled(!mModel->palette().isDefault());
            setDirty<Config::CategoryAppearance>();
            updateColorDialog();
        });


    connect(loadButton, &QPushButton::clicked, this, [this]() {
        auto filename = QFileDialog::getOpenFileName(
                    this,
                    tr("Open color settings"),
                    mSaveDir.path(),
                    tr(TU::COLOR_SETTINGS_FILTER)
                    );
        if (!filename.isEmpty()) {
            QSettings settings(filename, QSettings::IniFormat);
            Palette pal;
            pal.readSettings(settings);
            mModel->setPalette(pal);
            mSaveDir.setPath(filename);
        }
    });

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        auto filename = QFileDialog::getSaveFileName(
                    this,
                    tr("Save color settings"),
                    mSaveDir.filePath(tr("colors.ini")),
                    tr(TU::COLOR_SETTINGS_FILTER)
                    );
        if (!filename.isEmpty()) {
            QSettings settings(filename, QSettings::IniFormat);
            mModel->palette().writeSettings(settings, true);
            mSaveDir.setPath(filename);
        }
    });

    connect(mDefaultButton, &QPushButton::clicked, this, [this]() {
        mModel->setPalette(Palette());
        setDirty<Config::CategoryAppearance>();
        mDefaultButton->setEnabled(false);
    });

    connect(pickerButton, &QPushButton::clicked, this, [this]() {
        if (mColorDialog == nullptr) {
            mColorDialog = new QColorDialog(this);
            mColorDialog->setOptions(QColorDialog::NoButtons | QColorDialog::DontUseNativeDialog);
            updateColorDialog();
            lazyconnect(mColorDialog, currentColorChanged, this, chooseColor);
        }
        mColorDialog->show();
    });

    auto selectionModel = colorTree->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
        [this](QItemSelection const& selected) {
            if (selected.isEmpty()) {
                selectColor(QModelIndex());
            } else {
                selectColor(selected.indexes().first());
            }
        });




}

void AppearanceConfigTab::apply(AppearanceConfig &appearanceConfig, Palette &pal) {

    appearanceConfig.setPatternGridFont(mFonts[0]);
    appearanceConfig.setOrderGridFont(mFonts[1]);
    appearanceConfig.setPatternGridHeaderFont(mFonts[2]);

    appearanceConfig.setShowFlats(mShowFlatsCheck->isChecked());
    appearanceConfig.setShowPreviews(mShowPreviewsCheck->isChecked());

    pal = mModel->palette();

    clean();
}

void AppearanceConfigTab::chooseFont() {
    auto button = qobject_cast<QPushButton*>(sender());
    if (button) {
        auto iter = std::find(mFontChooseButtons.begin(), mFontChooseButtons.end(), button);
        if (iter != mFontChooseButtons.end()) {
            auto index = iter - mFontChooseButtons.begin();

            bool ok;
            auto font = QFontDialog::getFont(
                &ok,
                mFonts[index],
                this,
                tr("Select a font"),
                QFontDialog::DontUseNativeDialog | QFontDialog::MonospacedFonts
            );

            if (ok) {
                if (font.pointSize() > TU::MAX_POINT_SIZE) {
                    font.setPointSize(TU::MAX_POINT_SIZE);
                }
                setFont(index, font, index < 2);
                setDirty<Config::CategoryAppearance>();
            }

        }
    }
}

void AppearanceConfigTab::chooseColor(QColor const& color) {
    mModel->setColor(mSelectedColor, color);
}

void AppearanceConfigTab::setFont(size_t index, QFont const& font, bool showSize) {
    mFonts[index] = font;
    auto button = mFontChooseButtons[index];

    if (showSize) {
        button->setText(tr("%1, %2 pt").arg(
                            font.family(),
                            QString::number(font.pointSize())));
    } else {
        button->setText(font.family());
    }
    auto buttonFont = font;
    // use the same font size as the button
    buttonFont.setPointSize(button->font().pointSize());
    button->setFont(buttonFont);

}

void AppearanceConfigTab::selectColor(const QModelIndex &index) {
    mSelectedColor = index;
    updateColorDialog();
}

void AppearanceConfigTab::updateColorDialog() {
    if (mColorDialog) {
        auto color = mModel->colorAt(mSelectedColor);
        if (color) {
            mColorDialog->setCurrentColor(*color);
        }
        mColorDialog->setEnabled(color.has_value());
    }
}

#undef TU

