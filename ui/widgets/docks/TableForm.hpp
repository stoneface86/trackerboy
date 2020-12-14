
#pragma once

#include "model/BaseTableModel.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QLineEdit>
#include <QListView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

//
// Widget containing a list view, toolbar and line edit for the wave and
// instrument tables. Qt Designer was not used as it does not support
// QToolBar outside of a QMainWindow.
//
class TableForm : public QWidget {

    Q_OBJECT

public:
    TableForm(BaseTableModel &model, QKeySequence editorShortcut, QString typeName, QWidget *parent = nullptr);
    ~TableForm() = default;

    void setupMenu(QMenu &menu);

signals:
    void showEditor();

private slots:
    void viewCurrentChanged(const QModelIndex &current, const QModelIndex &prev);

    void viewContextMenu(const QPoint &pos);

    void modelCurrentChanged(int index);

private:
    BaseTableModel &mModel;
    

    QVBoxLayout mLayout;
        QHBoxLayout mToolbarLayout;
            QToolBar mToolbar;
            QLineEdit mNameEdit;
        QListView mListView;

    QMenu mContextMenu;    

    // actions
    QAction mActionAdd;
    QAction mActionRemove;
    QAction mActionDuplicate;
    QAction mActionImport;
    QAction mActionExport;
    QAction mActionEdit;

    


};
