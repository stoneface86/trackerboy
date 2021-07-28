
#pragma once

#include "core/model/BaseTableModel.hpp"
#include "misc/TableActions.hpp"

#include <QKeySequence>
#include <QListView>
#include <QString>
#include <QWidget>

class TableDock : public QWidget {

    Q_OBJECT

public:
    TableDock(
        BaseTableModel &model,
        QKeySequence editorShortcut,
        QString const& typeName,
        QWidget *parent = nullptr
    );

    TableActions const& tableActions() const;

    void add();

    void remove();

    void duplicate();


signals:
    void selectedItemChanged(int index);

    void edit(int index);

private:

    void updateActions();
    
    void setSelected(int index);


    BaseTableModel &mModel;
    TableActions mActions;

    QListView *mView;

    int mSelectedItem;
    


};
