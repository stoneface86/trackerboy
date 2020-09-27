
#pragma once

#include <QAbstractListModel>
#include <QAction>

#include "model/ModuleDocument.hpp"

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
    //void setName(QString name);

    void setActions(QAction *actNew, QAction *actRemove, QAction *actDuplicate, QAction *actEdit);

public slots:

    // adds a new item, if there was no items prior then this one is selected
    void add();
    // removes the current selected item, the next available item is selected
    void remove();

    void duplicate();

    void rename(const QString &name);

    void select(int index);
    void select(const QModelIndex &index);

signals:
    void currentIndexChanged(int index);

protected:
    BaseModel(ModuleDocument &document);

    virtual void dataRename(const QString &name) = 0;

    virtual int nextIndex() = 0;
    virtual void dataAdd() = 0;
    virtual void dataRemove(int row) = 0;
    virtual void dataDuplicate(int row) = 0;

    virtual bool canAdd() = 0;
    virtual bool canRemove() = 0;

    ModuleDocument &mDocument;

    int mCurrentIndex;
    //bool mEnabled;

    QAction *mActNew;
    QAction *mActRemove;
    QAction *mActDuplicate;
    QAction *mActEdit;


};
