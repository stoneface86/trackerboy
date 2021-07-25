
#include "core/model/InstrumentListModel.hpp"
#include "misc/IconManager.hpp"


InstrumentListModel::InstrumentListModel(Module &mod, QObject *parent) :
    BaseTableModel(mod, mod.data().instrumentTable(), tr("New instrument"), parent)
{
}


QIcon InstrumentListModel::iconData(uint8_t id) const {
    auto ch = static_cast<trackerboy::Instrument const*>(mBaseTable.get(id))->channel();
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

std::shared_ptr<trackerboy::Instrument> InstrumentListModel::currentInstrument() {
    //if (mCurrentIndex == -1) {
        return nullptr;
    //} else {
    //    return static_cast<trackerboy::InstrumentTable const&>(mBaseTable).getShared(id(mCurrentIndex));
    //}
}

void InstrumentListModel::updateChannelIcon() {
    //auto index = createIndex(mCurrentIndex, 0);
    //emit dataChanged(index, index, { Qt::DecorationRole });
}
