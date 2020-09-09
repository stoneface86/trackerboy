
#pragma once

#include <QAbstractListModel>

//
// Base class for all models used in the UI. Each model is a list model and has a
// current item "selected". The selected item is the one being edited.
//
class BaseModel : public QAbstractListModel {

    Q_OBJECT

public:
    int currentIndex() const;

    // disable or enable the model. Used when loading module data from a file, so
    // that the model is reset with the new data. Disable before updating, update
    // and then re-enable.
    void setEnabled(bool enabled);

    // get the name of the current index
    virtual QString name() = 0;

    // set the name of the current index
    void setName(QString name);

public slots:

    void select(int index);
    void select(const QModelIndex &index);

signals:
    void currentIndexChanged(int index);

protected:
    BaseModel(QObject *parent = nullptr);

    virtual void setNameInData(QString name) = 0;

    int mCurrentIndex;
    //bool mEnabled;


};
