
#include "core/model/WaveListModel.hpp"

WaveListModel::WaveListModel(Module &mod, QObject *parent) :
    BaseTableModel(mod, mod.data().waveformTable(), tr("New waveform"), parent)
{
}

QIcon WaveListModel::iconData(uint8_t id) const {
    (void)id;
    return QIcon();
}

std::shared_ptr<trackerboy::Waveform> WaveListModel::getShared(int index) {

    if (index == -1) {
        // index invalid, return nullptr
        return nullptr;
    } else {
        return table().getShared(id(index));
    }
}

trackerboy::WaveformTable& WaveListModel::table() noexcept {
    return static_cast<trackerboy::WaveformTable&>(mBaseTable);
}
