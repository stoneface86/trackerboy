
#include "config/tabs/AppearanceConfigTab.hpp"
#include "utils/connectutils.hpp"

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


//
// Columns of the model
//
enum Columns {
    ColumnItem,
    ColumnColor,

    ColumnCount
};

//
// Palette::Color enums are organized into the following categories:
//
enum Category {
    CategoryPatternEditor,
    CategoryHeader,
    CategoryGraph,
    CategoryScope,

    CategoryCount
};

//
// Name for each Palette::Color to be displayed to the user
//
static std::array const COLOR_NAMES = {
    QT_TR_NOOP("Background"),
    QT_TR_NOOP("Highlighted background 1"),
    QT_TR_NOOP("Highlighted background 2"),
    QT_TR_NOOP("Text"),
    QT_TR_NOOP("Highlighted text 1"),
    QT_TR_NOOP("Highlighted text 2"),
    QT_TR_NOOP("Current row"),
    QT_TR_NOOP("Current row (recording)"),
    QT_TR_NOOP("Player row"),
    QT_TR_NOOP("Effect type"),
    QT_TR_NOOP("Instrument"),
    QT_TR_NOOP("Selection"),
    QT_TR_NOOP("Cursor"),
    QT_TR_NOOP("Line"),
    QT_TR_NOOP("Background 1"),
    QT_TR_NOOP("Background 2"),
    QT_TR_NOOP("Foreground 1"),
    QT_TR_NOOP("Foreground 2"),
    QT_TR_NOOP("Channel enabled"),
    QT_TR_NOOP("Channel disabled"),
    QT_TR_NOOP("Background"),
    QT_TR_NOOP("Alternate"),
    QT_TR_NOOP("Lines"),
    QT_TR_NOOP("Samples"),
    QT_TR_NOOP("Background"),
    QT_TR_NOOP("Line")
};
static_assert(COLOR_NAMES.size() == Palette::ColorCount, "color name table size mismatch");

//
// Color groups for each Category, this array must be sorted in ascending order,
// thus the Palette::Color enum must be organized accordingly
//
static constexpr std::array<int, CategoryCount + 1> COLOR_GROUPS = {
    Palette::ColorBackground,           // CategoryPatternEditor here
    Palette::ColorHeaderBackground1,    // CategoryHeader starts here
    Palette::ColorGraphBackground,      // CategoryGraph starts here
    Palette::ColorScopeBackground,      // CategoryScope starts here
    Palette::ColorCount
};

template <typename T, size_t N>
constexpr bool arrayIsSorted(std::array<T, N> const& arr, size_t from) {
    return N - from == 0 || (arr[from - 1] <= arr[from] && arrayIsSorted(arr, from + 1));
}
static_assert(arrayIsSorted(COLOR_GROUPS, 1), "cannot group colors");
// in C++20 we can do this instead:
//static_assert(std::is_sorted(COLOR_GROUPS.cbegin(), COLOR_GROUPS.cend()), "cannot group colors");

//
// Get the number of colors for a category.
//
constexpr int colorsInCategory(Category category) {
    return COLOR_GROUPS[category + 1] - COLOR_GROUPS[category];
}


struct ModelId {

    Category category;
    std::optional<Palette::Color> color;

    quintptr toInternalId() {

        // for parent nodes, the internalId is CategoryCount
        // for child nodes, the internalId is the Category of the parent
        return color ? (quintptr)category : (quintptr)CategoryCount;
    }
};

//
// Get the ModelId from the given QModelIndex. The index's internalId is converted
// to a ModelId.
//
ModelId getModelId(QModelIndex const& index) {
    auto id = (Category)index.internalId();
    if (id == CategoryCount) {
        return {
            (Category)index.row(),
            std::nullopt
        };
    } else {
        auto const color = index.row() + COLOR_GROUPS[id];
        Q_ASSERT(color >= 0 && color < Palette::ColorCount);
        return {
            id,
            (Palette::Color)color
        };
    }

}

}

//
// Model class for editing a Palette. Used only by the AppearanceConfigTab
//
class PaletteModel : public QAbstractItemModel {

    Q_OBJECT

public:
    explicit PaletteModel(Palette &src, QObject *parent = nullptr) :
        QAbstractItemModel(parent),
        mPalette(src)
    {
    }

    Palette const& palette() const {
        return mPalette;
    }

    void setPalette(Palette const& pal) {
        if (mPalette != pal) {
            mPalette = pal;
            // the entire tree changed
            emit dataChanged(QModelIndex(), QModelIndex(), { Qt::DisplayRole, Qt::DecorationRole });
        }
    }

    virtual int columnCount(QModelIndex const& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return TU::ColumnCount;
    }

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override {
        if (index.isValid()) {
            auto const id = TU::getModelId(index);

            if (id.color) {
                switch (index.column()) {
                    case TU::ColumnItem:
                        if (role == Qt::DisplayRole) {
                            return tr(TU::COLOR_NAMES[*id.color]);
                        }
                        break;
                    case TU::ColumnColor:
                        if (role == Qt::DecorationRole) {
                            return mPalette[*id.color];
                        } else if (role == Qt::DisplayRole || role == Qt::EditRole) {
                            return mPalette[*id.color].name(QColor::HexRgb);
                        }
                        break;
                    default:
                        break;
                }
            } else {
                if (index.column() == TU::ColumnItem && role == Qt::DisplayRole) {
                    switch (id.category) {
                        case TU::CategoryPatternEditor:
                            return tr("Pattern editor");
                        case TU::CategoryHeader:
                            return tr("Pattern editor header");
                        case TU::CategoryGraph:
                            return tr("Graph editor");
                        case TU::CategoryScope:
                            return tr("Audio scope");
                        default:
                            break;
                    }
                }
            }
        }

        return {};
    }

    virtual Qt::ItemFlags flags(QModelIndex const& index) const override {
        if (index.isValid()) {
            auto const id = TU::getModelId(index);
            if (id.color) {
                auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
                if (index.column() == 1) {
                    flags |= Qt::ItemIsEditable;
                }
                return flags;
            } else {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        }

        return Qt::NoItemFlags;
    }

    virtual bool hasChildren(QModelIndex const& index) const override {
        if (index.isValid()) {
            return !TU::getModelId(index).color;
        } else {
            return true;
        }
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            switch (section) {
                case TU::ColumnItem:
                    return tr("Item");
                case TU::ColumnColor:
                    return tr("Color");
                default:
                    break;
            }
        }

        return {};
    }

    virtual QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const override {

        TU::ModelId id;

        if (parent.isValid()) {
            auto const parentId = TU::getModelId(parent);
            id.category = parentId.category;
            id.color = (Palette::Color)(TU::COLOR_GROUPS[id.category] + row);

        } else {
            if (row >= TU::CategoryCount) {
                return {};
            } else {
                id.category = (TU::Category)row;
                id.color.reset();
            }
        }

        return createIndex(row, column, id.toInternalId());
    }

    virtual QModelIndex parent(QModelIndex const& index) const override {
        if (index.isValid()) {
            auto const id = TU::getModelId(index);
            if (id.color) {
                TU::ModelId parentId;
                parentId.category = id.category;
                parentId.color = std::nullopt;
                return createIndex(id.category, 0, parentId.toInternalId());
            }
        }

        return {};
    }

    virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            auto const id = TU::getModelId(parent);
            if (!id.color && id.category < TU::CategoryCount) {
                return TU::colorsInCategory(id.category);
            }
            return 0;
        } else {
            return TU::CategoryCount;
        }
    }

    virtual bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole) override {
        if (index.isValid() && role == Qt::EditRole && index.column() == TU::ColumnColor) {
            auto const id = TU::getModelId(index);
            if (id.color) {
                QColor color;
                color.setNamedColor(value.toString());
                if (color.isValid()) {
                    color.setAlpha(255); // sanitize, do not let the user modify alpha channel
                    updateColor(index, *id.color, color);
                    return true;
                }
            }
        }

        return false;
    }

    std::optional<QColor> colorAt(QModelIndex const& index) const {
        if (index.isValid()) {
            auto const color = TU::getModelId(index).color;
            if (color) {
                return mPalette[*color];
            }
        }

        return std::nullopt;
    }

    void setColor(QModelIndex const& index, std::optional<QColor> color = std::nullopt) {
        if (index.isValid()) {
            auto const id = TU::getModelId(index);
            if (id.color) {
                if (color) {
                    updateColor(index, *id.color, *color);
                } else {
                    updateColor(index, *id.color, Palette::getDefault(*id.color));
                }
            }

        }
    }


private:

    Q_DISABLE_COPY(PaletteModel)

    void updateColor(QModelIndex const& index, Palette::Color paletteIndex, QColor const& color) {
        if (color != mPalette[paletteIndex]) {
            mPalette.setColor(paletteIndex, color);
            auto changedIndex = createIndex(index.row(), 1, index.internalId());
            emit dataChanged(changedIndex, changedIndex, { Qt::DisplayRole, Qt::DecorationRole });
        }
    }


    Palette &mPalette;

};






AppearanceConfigTab::AppearanceConfigTab(
        AppearanceConfig const& appearance,
        Palette &pal,
        QWidget *parent
    ) :
    ConfigTab(parent),
    mColorDialog(nullptr),
    mSaveDir(QDir::home())
{

    auto layout = new QVBoxLayout;

    auto fontGroup = new QGroupBox(tr("Fonts"));
    auto fontLayout = new QGridLayout;

    for (int i = 0; i < FONT_COUNT; ++i) {
        auto &chooseBtn = mFontChooseButtons[i];

        fontLayout->addWidget(new QLabel(tr(TU::FONT_NAMES[i])), i, 0);
        chooseBtn = new QPushButton;
        fontLayout->addWidget(chooseBtn, i, 1);

        connect(chooseBtn, &QPushButton::clicked, this, &AppearanceConfigTab::chooseFont);
    }

    fontLayout->setColumnStretch(1, 1);
    fontGroup->setLayout(fontLayout);

    auto colorGroup = new QGroupBox(tr("Colors"));
    auto colorLayout = new QVBoxLayout;
    auto colorTree = new QTreeView;
    colorLayout->addWidget(colorTree, 1);
    auto colorButtonLayout = new QHBoxLayout;
    auto loadButton = new QPushButton(tr("Load"));
    colorButtonLayout->addWidget(loadButton);
    auto saveButton = new QPushButton(tr("Save"));
    colorButtonLayout->addWidget(saveButton);
    colorButtonLayout->addStretch();
    mDefaultButton = new QPushButton(tr("Default"));
    colorButtonLayout->addWidget(mDefaultButton);
    auto pickerButton = new QPushButton(tr("Color picker..."));
    colorButtonLayout->addWidget(pickerButton);

    colorLayout->addLayout(colorButtonLayout);
    colorGroup->setLayout(colorLayout);

    layout->addWidget(fontGroup);
    layout->addWidget(colorGroup, 1);
    setLayout(layout);

    mModel = new PaletteModel(pal, this);
    colorTree->setModel(mModel);
    colorTree->expandAll();
    colorTree->resizeColumnToContents(0);
    colorTree->collapseAll();

    setFont(0, appearance.patternGridFont());
    setFont(1, appearance.orderGridFont());
    setFont(2, appearance.patternGridHeaderFont(), false);

    connect(mModel, &PaletteModel::dataChanged, this,
        [this]() {
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
            mModel->palette().writeSettings(settings);
            mSaveDir.setPath(filename);
        }
    });

    connect(mDefaultButton, &QPushButton::clicked, this, [this]() {
        mModel->setColor(mSelectedColor);
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

void AppearanceConfigTab::apply(AppearanceConfig &appearanceConfig) {

    appearanceConfig.setPatternGridFont(mFonts[0]);
    appearanceConfig.setOrderGridFont(mFonts[1]);
    appearanceConfig.setPatternGridHeaderFont(mFonts[2]);

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

#include "AppearanceConfigTab.moc"

#undef TU

