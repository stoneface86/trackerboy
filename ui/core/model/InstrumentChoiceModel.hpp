
#pragma once


class InstrumentListModel;

#include <QAbstractListModel>

//
// Wraps an InstrumentListModel, but provides a "None" item. To be used in
// a combobox so that the user can select an instrument or no instrument.
//
class InstrumentChoiceModel : public QAbstractListModel {

    Q_OBJECT

public:

    explicit InstrumentChoiceModel(QObject *parent = nullptr);

    virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    void setModel(InstrumentListModel *model);

private:
    void modelDestroyed();

    void forwardRowInsertion(QModelIndex const& parent, int start, int end);
    void forwardRowRemoval(QModelIndex const& parent, int first, int last);
    void forwardDataChanges(QModelIndex const& topLeft, QModelIndex const& bottomRight, QVector<int> const& roles);

    InstrumentListModel *mModel;

};
