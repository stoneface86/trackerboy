
#include "core/model/WaveListModel.hpp"
#include "core/model/ModuleDocument.hpp"

WaveListModel::WaveListModel(ModuleDocument &document) :
    BaseTableModel(document, document.mod().waveformTable(), tr("New waveform"))
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