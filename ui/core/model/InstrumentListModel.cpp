
#include "core/model/InstrumentListModel.hpp"
#include "misc/IconManager.hpp"
#include "core/model/ModuleDocument.hpp"


InstrumentListModel::InstrumentListModel(ModuleDocument &document) :
    BaseTableModel(document, document.mod().instrumentTable())
{
}


QIcon InstrumentListModel::iconData(trackerboy::DataItem const& item) const {
    auto ch = static_cast<trackerboy::Instrument const&>(item).channel();
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


