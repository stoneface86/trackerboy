
#include "core/model/WaveListModel.hpp"


WaveListModel::WaveListModel(ModuleDocument &document) :
    BaseTableModel(document, document.waveformTable())
{
}


QIcon WaveListModel::iconData(trackerboy::DataItem const& item) const {
    (void)item;
    return QIcon();
}


