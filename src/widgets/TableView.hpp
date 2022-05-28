
#pragma once

#include "model/BaseTableModel.hpp"
#include "utils/TableActions.hpp"
#include "verdigris/wobjectdefs.h"

#include <QKeySequence>
#include <QListView>
#include <QString>
#include <QWidget>

//
// Composite widget for viewing/editing a trackerboy::Table. Not
// to be confused with QTableView.
//
class TableView : public QWidget {

    W_OBJECT(TableView)

public:
    TableView(
        BaseTableModel &model,
        QString const& typeName,
        QWidget *parent = nullptr
    );

    TableActions const& tableActions() const;

    void add();

    void remove();

    void duplicate();

    int selectedItem() const;

    void setSelectedItem(int item);

    void setShortcut(QKeySequence const& seq);

//signals:
    void selectedItemChanged(int index) W_SIGNAL(selectedItemChanged, index)

    void edit(int index) W_SIGNAL(edit, index)

private:
    Q_DISABLE_COPY(TableView)

    void updateActions();
    
    void setSelected(int index);


    BaseTableModel &mModel;
    TableActions mActions;

    QListView *mView;

    int mSelectedItem;
    


};
