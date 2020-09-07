
#pragma once

#include <QAbstractListModel>

#include "trackerboy/data/Table.hpp"

#include "model/BaseTableModel.hpp"

// Model for QListView (Waveforms dock widget) or QComboBox
// used by QComboBox for selecting a waveform in the Instrument editor or Wave editor
class WaveListModel : public BaseTableModel {

    Q_OBJECT

public:
    WaveListModel(trackerboy::WaveTable &table, QObject *parent = nullptr);

    // get the waveform associated with the model index
    trackerboy::Waveform* waveform(int modelIndex);
    
protected:
    QVariant iconData(const QModelIndex &index) const override;

private:

    trackerboy::WaveTable &mTable;

};
