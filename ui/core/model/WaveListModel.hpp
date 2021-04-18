
#pragma once

#include "core/model/BaseTableModel.hpp"

#include "trackerboy/data/Table.hpp"

#include <QPoint>

// Model for QListView (Waveforms dock widget) or QComboBox
// used by QComboBox for selecting a waveform in the Instrument editor or Wave editor
class WaveListModel : public BaseTableModel {

    Q_OBJECT

public:
    WaveListModel(ModuleDocument &document);
    
protected:
    virtual QIcon iconData(trackerboy::DataItem const& item) const override;

private:
    Q_DISABLE_COPY(WaveListModel)

};
