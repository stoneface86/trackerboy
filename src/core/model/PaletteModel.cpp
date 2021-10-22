
#include "core/model/PaletteModel.hpp"

#define TU PaletteModelTU
namespace TU {

enum class Category : uint8_t {
    patternEditor,
    header
};

constexpr size_t CATEGORIES = 2;

struct Node {

    // parent node
    constexpr Node(Category category) :
        parent(true),
        category(category),
        paletteIndex(0)
    {
    }

    // child node
    constexpr Node(Category category, uint8_t paletteIndex) :
        parent(false),
        category(category),
        paletteIndex(paletteIndex)
    {
    }

    bool parent;
    Category category;
    uint8_t paletteIndex;

};

static constexpr std::array NODE_TABLE = {
    Node(Category::patternEditor),
    Node(Category::patternEditor, Palette::ColorBackground),
    Node(Category::patternEditor, Palette::ColorBackgroundHighlight1),
    Node(Category::patternEditor, Palette::ColorBackgroundHighlight2),
    Node(Category::patternEditor, Palette::ColorForeground),
    Node(Category::patternEditor, Palette::ColorForegroundHighlight1),
    Node(Category::patternEditor, Palette::ColorForegroundHighlight2),
    Node(Category::patternEditor, Palette::ColorRow),
    Node(Category::patternEditor, Palette::ColorRowEdit),
    Node(Category::patternEditor, Palette::ColorRowPlayer),
    Node(Category::patternEditor, Palette::ColorEffectType),
    Node(Category::patternEditor, Palette::ColorInstrument),
    Node(Category::patternEditor, Palette::ColorSelection),
    Node(Category::patternEditor, Palette::ColorCursor),
    Node(Category::patternEditor, Palette::ColorLine),
    Node(Category::header),
    Node(Category::header, Palette::ColorHeaderBackground1),
    Node(Category::header, Palette::ColorHeaderBackground2),
    Node(Category::header, Palette::ColorHeaderForeground1),
    Node(Category::header, Palette::ColorHeaderForeground2),
    Node(Category::header, Palette::ColorHeaderEnabled),
    Node(Category::header, Palette::ColorHeaderDisabled),

};
static_assert(NODE_TABLE.size() == Palette::ColorCount + CATEGORIES, "NODE_TABLE size mismatch");

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
    QT_TR_NOOP("Channel disabled")

};
static_assert(COLOR_NAMES.size() == Palette::ColorCount, "color name table size mismatch");

constexpr quintptr findParent(int order) {
    for (quintptr i = 0; i < NODE_TABLE.size(); ++i) {
        if (NODE_TABLE[i].parent) {
            if (order) {
                --order;
            } else {
                return i;
            }
        }
    }
    return (quintptr)NODE_TABLE.size();
}

constexpr auto PARENT_PATTERN_EDITOR = findParent(0);
constexpr auto PARENT_HEADER = findParent(1);

constexpr int PARENTS = 2;


Node getNode(QModelIndex const& index) {
    return NODE_TABLE[index.internalId()];
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
        emit dataChanged(index(0, 0), index(TU::PARENTS - 1, 1), { Qt::DisplayRole, Qt::DecorationRole });
    }
}

int PaletteModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 2;
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {

        auto const node = TU::getNode(index);
        if (node.parent) {
            if (index.column() == 0 && role == Qt::DisplayRole) {
                switch (node.category) {
                    case TU::Category::patternEditor:
                        return tr("Pattern editor");
                    case TU::Category::header:
                        return tr("Pattern editor header");
                    default:
                        break;
                }
            }
        } else {
            switch (index.column()) {
                case 0:
                    if (role == Qt::DisplayRole) {
                        return tr(TU::COLOR_NAMES[node.paletteIndex]);
                    }
                    break;
                case 1:
                    if (role == Qt::DecorationRole) {
                        return mPalette[(Palette::Color)node.paletteIndex];
                    } else if (role == Qt::DisplayRole || role == Qt::EditRole) {
                        return mPalette[(Palette::Color)node.paletteIndex].name(QColor::HexRgb);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return {};
}

Qt::ItemFlags PaletteModel::flags(QModelIndex const& index) const {
    if (index.isValid()) {
        auto const node = TU::getNode(index);
        if (node.parent) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else {
            auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
            if (index.column() == 1) {
                flags |= Qt::ItemIsEditable;
            }
            return flags;
        }
    }

    return Qt::NoItemFlags;
}

bool PaletteModel::hasChildren(const QModelIndex &index) const {
    if (index.isValid()) {
        auto const node = TU::getNode(index);
        return node.parent;
    } else {
        return true;
    }
}

QVariant PaletteModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Description");
            case 1:
                return tr("Color");
            default:
                break;
        }
    }

    return {};
}

QModelIndex PaletteModel::index(int row, int column, const QModelIndex &parent) const {

    // aid is an index to the TU::NODE_TABLE
    quintptr aid = 0;

    if (parent.isValid()) {
        aid = parent.internalId() + row + 1;
    } else {
        switch (row) {
            case 0:
                aid = TU::PARENT_PATTERN_EDITOR;
                break;
            case 1:
                aid = TU::PARENT_HEADER;
                break;
            default:
                return {};
        }
    }

    return createIndex(row, column, aid);
}

QModelIndex PaletteModel::parent(const QModelIndex &index) const {
    if (index.isValid()) {
        auto const node = TU::getNode(index);
        if (!node.parent) {
            switch (node.category) {
                case TU::Category::patternEditor:
                    return createIndex(0, 0, TU::PARENT_PATTERN_EDITOR);
                case TU::Category::header:
                    return createIndex(1, 0, TU::PARENT_HEADER);
                default:
                    break;
            }
        }
    }

    return {};
}

int PaletteModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        auto const node = TU::NODE_TABLE[parent.internalId()];
        if (node.parent) {
            switch (node.category) {
                case TU::Category::patternEditor:
                    return TU::PARENT_HEADER - TU::PARENT_PATTERN_EDITOR - 1;
                case TU::Category::header:
                    return TU::NODE_TABLE.size() - TU::PARENT_HEADER - 1;
            }
        }
        return 0;
    } else {
        return TU::PARENTS;
    }
}

bool PaletteModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && index.column() == 1) {
        auto const node = TU::getNode(index);

        if (!node.parent) {
            QColor color;
            color.setNamedColor(value.toString());
            if (color.isValid()) {
                color.setAlpha(255); // sanitize, do not let the user modify alpha channel
                updateColor(index, (Palette::Color)node.paletteIndex, color);
                return true;
            }



        }


    }

    return false;
}

std::optional<QColor> PaletteModel::colorAt(const QModelIndex &index) const {
    if (index.isValid()) {
        auto const node = TU::getNode(index);
        if (!node.parent) {
            return mPalette[(Palette::Color)node.paletteIndex];
        }
    }

    return std::nullopt;
}

void PaletteModel::setColor(const QModelIndex &index, const QColor &color) {
    if (index.isValid()) {
        auto const node = TU::getNode(index);
        if (!node.parent) {
            updateColor(index, (Palette::Color)node.paletteIndex, color);
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
