
#include "core/model/PaletteModel.hpp"

#define TU PaletteModelTU
namespace TU {

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

    static_assert(Palette::ColorCount <= 256, "too many colors defined");
    static_assert(CategoryCount <= 256, "too many categories defined");
    static_assert(sizeof(quintptr) >= 3, "cannot pack ModelId into quintptr");

    Category category;
    std::optional<Palette::Color> color;

    quintptr toInternalId() {
        // pack this struct into a quintptr, so that it can be placed in a QModelIndex's internalId
        // bits 0-7:  Palette::Color index
        // bits 8-15: Category
        // bit  16:   color enable

        quintptr id = category << 8;
        if (color) {
            id |= 0x10000 | *color;
        }
        return id;
    }
};

//
// Get the ModelId from the given QModelIndex. The index's internalId is converted
// to a ModelId.
//
ModelId getModelId(QModelIndex const& index) {
    auto id = index.internalId();
    ModelId result {
        (Category)((id >> 8) & 0xFF),
        (id & 0x10000) ? std::make_optional((Palette::Color)(id & 0xFF)) : std::nullopt
    };

    // check that this ModelId is valid
    Q_ASSERT(result.category < CategoryCount);
    Q_ASSERT(!result.color || *result.color < Palette::ColorCount);

    return result;
}


}


PaletteModel::PaletteModel(QObject *parent) :
    QAbstractItemModel(parent),
    mPalette()
{
}

Palette const& PaletteModel::palette() const {
    return mPalette;
}

void PaletteModel::setPalette(const Palette &pal) {
    if (mPalette != pal) {
        mPalette = pal;
        // the entire tree changed
        emit dataChanged(QModelIndex(), QModelIndex(), { Qt::DisplayRole, Qt::DecorationRole });
    }
}

int PaletteModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return TU::ColumnCount;
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const {
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

Qt::ItemFlags PaletteModel::flags(QModelIndex const& index) const {
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

bool PaletteModel::hasChildren(const QModelIndex &index) const {
    if (index.isValid()) {
        return !TU::getModelId(index).color;
    } else {
        return true;
    }
}

QVariant PaletteModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QModelIndex PaletteModel::index(int row, int column, const QModelIndex &parent) const {

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

QModelIndex PaletteModel::parent(const QModelIndex &index) const {
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

int PaletteModel::rowCount(const QModelIndex &parent) const {
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

bool PaletteModel::setData(const QModelIndex &index, const QVariant &value, int role) {
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

std::optional<QColor> PaletteModel::colorAt(const QModelIndex &index) const {
    if (index.isValid()) {
        auto const color = TU::getModelId(index).color;
        if (color) {
            return mPalette[*color];
        }
    }

    return std::nullopt;
}

void PaletteModel::setColor(const QModelIndex &index, const QColor &color) {
    if (index.isValid()) {
        auto const id = TU::getModelId(index);
        if (id.color) {
            updateColor(index, *id.color, color);
        }

    }
}

void PaletteModel::updateColor(QModelIndex const& index, Palette::Color paletteIndex, const QColor &color) {
    if (color != mPalette[paletteIndex]) {
        mPalette.setColor(paletteIndex, color);
        auto changedIndex = createIndex(index.row(), 1, index.internalId());
        emit dataChanged(changedIndex, changedIndex, { Qt::DisplayRole, Qt::DecorationRole });
    }
}

#undef TU
