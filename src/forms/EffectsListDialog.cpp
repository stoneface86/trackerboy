
#include "forms/EffectsListDialog.hpp"

#include <QAbstractItemModel>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <array>

#define TU EffectsListDialogTU
namespace TU {

static const char* ParamNone = "-";
static const char* ChannelsAny = QT_TR_NOOP("Any");
static const char* ChannelsEnvelope = "CH1, CH2, CH4";
static const char* ChannelsPulse = "CH1, CH2";
static const char* ChannelsWave = "CH3";

struct EffectListItem {

    enum Column {
        ColumnName,
        ColumnDescription,
        ColumnChannels,
        ColumnParam1,
        ColumnParam2,

        ColumnCount
    };

    std::array<const char*, ColumnCount> columns;

    explicit constexpr EffectListItem(
        const char* name,
        const char* description,
        const char* channels = ChannelsAny,
        const char* param1 = ParamNone,
        const char* param2 = ParamNone
    ) :
        columns{name, description, channels, param1, param2}
    {}

    inline constexpr const char* operator[](int index) const {
        return columns[index];
    }
};

static const char* ParamDescSpeed = QT_TR_NOOP("Speed, pitch units per frame");
static const char* ParamDescDelay = QT_TR_NOOP("Delay, in frames");
static const char* ParamDescSemitones = QT_TR_NOOP("Semitone offset from the current note");

static std::array const EffectListArray{
    EffectListItem("0xy", "Arpeggio", ChannelsAny, "1st Note", "2nd Note"),
    EffectListItem("1xx", "Pitch slide up", ChannelsAny, ParamDescSpeed),
    EffectListItem("2xx", QT_TR_NOOP("Pitch slide down"), ChannelsAny, ParamDescSpeed),
    EffectListItem("3xx", QT_TR_NOOP("Automatic portamento"), ChannelsAny,ParamDescSpeed),
    EffectListItem("4xy", QT_TR_NOOP("Square vibrato"), ChannelsAny, QT_TR_NOOP("Speed, pitch units per oscillation"), QT_TR_NOOP("Extent, in pitch units")),
    EffectListItem("5xx", QT_TR_NOOP("Set vibrato delay"), ChannelsAny, ParamDescDelay),
    EffectListItem("Bxx", QT_TR_NOOP("Pattern jump"), ChannelsAny, QT_TR_NOOP("Index of the pattern")),
    EffectListItem("C00", QT_TR_NOOP("Pattern halt"), ChannelsAny),
    EffectListItem("Dxx", QT_TR_NOOP("Pattern skip"), ChannelsAny, QT_TR_NOOP("Row number to start on")),
    EffectListItem("Fxy", QT_TR_NOOP("Set speed"), ChannelsAny, QT_TR_NOOP("Speed, integral part (1-F)"), QT_TR_NOOP("Speed, fractional part (0-F)")),
    EffectListItem("Exy", QT_TR_NOOP("Set volume envelope"), ChannelsEnvelope, QT_TR_NOOP("Initial volume"), QT_TR_NOOP("Mode and speed")),
    EffectListItem("Exx", QT_TR_NOOP("Set waveform"), ChannelsWave, QT_TR_NOOP("Waveform ID")),
    EffectListItem("Gxx", QT_TR_NOOP("Note delay"), ChannelsAny, ParamDescDelay),
    EffectListItem("Hxx", QT_TR_NOOP("Set sweep register"), ChannelsAny, QT_TR_NOOP("NR10 register value")),
    EffectListItem("I0x", QT_TR_NOOP("Set panning"), ChannelsAny, QT_TR_NOOP("panning (0-3)")),
    EffectListItem("Jxy", QT_TR_NOOP("Set global volume scale"), ChannelsAny, QT_TR_NOOP("Left volume (0-7)"), QT_TR_NOOP("Right volume (0-7)")),
    EffectListItem("L00", QT_TR_NOOP("Lock channel"), ChannelsAny),
    EffectListItem("Pxx", QT_TR_NOOP("Fine tuning"), ChannelsAny, QT_TR_NOOP("tune offset, 80 is in tune, 81 is +1, etc")),
    EffectListItem("Qxy", QT_TR_NOOP("Note slide up"), ChannelsAny, ParamDescSpeed, ParamDescSemitones),
    EffectListItem("Rxy", QT_TR_NOOP("Note slide down"), ChannelsAny, ParamDescSpeed, ParamDescSemitones),
    EffectListItem("Sxx", QT_TR_NOOP("Delayed note cut"), ChannelsAny, ParamDescDelay),
    EffectListItem("Txx", QT_TR_NOOP("Play sound effect"), ChannelsAny, QT_TR_NOOP("Sound effect ID")),
    EffectListItem("V0x", QT_TR_NOOP("Set duty"), ChannelsPulse, QT_TR_NOOP("Duty (0-3)")),
    EffectListItem("V0x", QT_TR_NOOP("Set wave volume"), ChannelsWave, QT_TR_NOOP("Volume (0-3)")),
    EffectListItem("V0x", QT_TR_NOOP("Set noise mode"), "CH4", QT_TR_NOOP("Noise mode (0-1)"))
};

}

class EffectsListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit EffectsListModel(QObject *parent = nullptr) :
        QAbstractItemModel(parent)
    {}

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override {
        if (parent.isValid()) {
            return {};
        } else {
            return createIndex(row, column, nullptr);
        }
    }

    virtual Qt::ItemFlags flags(QModelIndex const& index) const override {
        if (index.isValid()) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
        } else {
            return Qt::NoItemFlags;
        }
    }

    virtual QModelIndex parent(QModelIndex const& child) const override {
        Q_UNUSED(child)
        return {};
    }

    virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        } else {
            return (int)TU::EffectListArray.size();
        }
    }

    virtual int columnCount(QModelIndex const& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return TU::EffectListItem::ColumnCount;
    }

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override {
        if (index.isValid() && role == Qt::DisplayRole) {
            auto const row = index.row();
            if (row >= 0 && row < (int)TU::EffectListArray.size()) {
                auto const col = index.column();
                if (col >= 0 && col < TU::EffectListItem::ColumnCount) {
                    return tr(TU::EffectListArray[row][col]);
                }
            }
        }

        return {};
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
                case TU::EffectListItem::ColumnName:
                    return tr("Name");
                case TU::EffectListItem::ColumnDescription:
                    return tr("Description");
                case TU::EffectListItem::ColumnChannels:
                    return tr("Channels");
                case TU::EffectListItem::ColumnParam1:
                    return tr("Param 1 (x)");
                case TU::EffectListItem::ColumnParam2:
                    return tr("Param 2 (y)");
                default:
                    break;
            }
        }

        return {};
    }

private:
    Q_DISABLE_COPY(EffectsListModel)

};


EffectsListDialog::EffectsListDialog(QWidget *parent) :
    PersistantDialog(parent)
{
    setWindowTitle(tr("Effects list"));

    // create models, use a proxy model for sorting
    auto model = new EffectsListModel(this);
    auto proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    // setup the view, enable sorting
    auto view = new QTreeView();
    view->setModel(proxyModel);
    view->setSortingEnabled(true);

    // setup the layout, just contains the view
    auto layout = new QVBoxLayout();
    layout->addWidget(view, 1);
    setLayout(layout);

    // default sort order by effect name in ascending order
    proxyModel->sort(TU::EffectListItem::ColumnName, Qt::AscendingOrder);

    for (auto i = 0; i < TU::EffectListItem::ColumnCount; ++i) {
        view->resizeColumnToContents(i);
    }
    resize(640, 480);
}

#include "EffectsListDialog.moc"

#undef TU
