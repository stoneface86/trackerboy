
#pragma once

#include "model/BaseTableModel.hpp"

#include <QIcon>

class InstrumentListModel : public BaseTableModel {

    Q_OBJECT

public:
    InstrumentListModel(trackerboy::InstrumentTable &table, QObject *parent = nullptr);

    // get the waveform associated with the model index
    trackerboy::Instrument* instrument(int modelIndex) const;

protected:
    QVariant iconData(const QModelIndex &index) const override;

private:
    QIcon mPulseIcon;
    QIcon mWaveIcon;
    QIcon mNoiseIcon;

};
