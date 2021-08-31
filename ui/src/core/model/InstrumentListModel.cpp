
#include "core/model/InstrumentListModel.hpp"
#include "core/IconManager.hpp"


InstrumentListModel::InstrumentListModel(Module &mod, QObject *parent) :
    BaseTableModel(mod, mod.data().instrumentTable(), tr("New instrument"), parent)
{
}


QIcon InstrumentListModel::iconData(uint8_t id) const {
    auto ch = table().get(id)->channel();
    Icons icons;
    switch (ch) {
        case trackerboy::ChType::ch1:
            icons = Icons::ch1;
            break;
        case trackerboy::ChType::ch2:
            icons = Icons::ch2;
            break;
        case trackerboy::ChType::ch3:
            icons = Icons::ch3;
            break;
        default:
            icons = Icons::ch4;
            break;

    }

    return IconManager::getIcon(icons);
}

std::shared_ptr<trackerboy::Instrument> InstrumentListModel::getShared(int index) {
    if (index == -1) {
        return nullptr;
    } else {
        return table().getShared(id(index));
    }
}

void InstrumentListModel::updateChannelIcon(int index) {
    auto modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, { Qt::DecorationRole });
}

trackerboy::InstrumentTable& InstrumentListModel::table() noexcept {
    return static_cast<trackerboy::InstrumentTable&>(mBaseTable);
}

trackerboy::InstrumentTable const& InstrumentListModel::table() const noexcept {
    return static_cast<trackerboy::InstrumentTable const&>(mBaseTable);
}
