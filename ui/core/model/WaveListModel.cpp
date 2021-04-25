
#include "core/model/WaveListModel.hpp"
#include "core/model/ModuleDocument.hpp"

WaveListModel::WaveListModel(ModuleDocument &document) :
    BaseTableModel(document, document.mod().waveformTable())
{
}


QIcon WaveListModel::iconData(trackerboy::DataItem const& item) const {
    (void)item;
    return QIcon();
}


