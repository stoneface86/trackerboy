
#include "core/model/WaveListModel.hpp"

WaveListModel::WaveListModel(Module &mod, QObject *parent) :
    BaseTableModel(mod, mod.data().waveformTable(), tr("New waveform"), parent)
{
}

QIcon WaveListModel::iconData(uint8_t id) const {
    (void)id;
    return QIcon();
}

trackerboy::Waveform* WaveListModel::currentWaveform() {
    if (mCurrentIndex == -1) {
        return nullptr;
    } else {
        return static_cast<trackerboy::WaveformTable&>(mBaseTable)[id(mCurrentIndex)];
    }
}