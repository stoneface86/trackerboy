
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

