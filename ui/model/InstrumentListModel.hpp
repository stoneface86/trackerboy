
#pragma once

#include "model/BaseTableModel.hpp"


class InstrumentListModel : public BaseTableModel {

    Q_OBJECT

public:
    InstrumentListModel(trackerboy::InstrumentTable &table, QObject *parent = nullptr);

    // get the waveform associated with the model index
    trackerboy::Instrument* instrument(int modelIndex);

protected:
    QVariant iconData(const QModelIndex &index) const override;


};
