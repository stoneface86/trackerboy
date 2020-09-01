
#pragma once

#include <QAbstractListModel>

#include "trackerboy/data/Table.hpp"

// used by QComboBox for selecting a waveform in the Instrument editor or Wave editor
class WaveListModel : public QAbstractListModel {

    Q_OBJECT

public:
    WaveListModel(trackerboy::WaveTable &table, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

private:

    trackerboy::WaveTable &mTable;

};
