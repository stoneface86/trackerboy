#pragma once

#include <QWidget>
#include <QDialog>
#include <QMenu>
#include <QLineEdit>
#include <QListView>


#include "model/BaseTableModel.hpp"

//
// Widget containing a list view, toolbar and line edit for the wave and
// instrument tables. Qt Designer was not used as it does not support
// QToolBar outside of a QMainWindow.
//
class TableForm : public QWidget {

public:
    TableForm(BaseTableModel &model, QWidget *editor, QString editorShortcut, QString typeName, QWidget *parent = nullptr);

    QMenu* menu() const;

private slots:
    void viewCurrentChanged(const QModelIndex &current, const QModelIndex &prev);

    void viewContextMenu(const QPoint &pos);

    void modelCurrentChanged(int index);

private:
    BaseTableModel &mModel;
    QMenu *mMenu;
    QWidget *mEditor;

    // widgets
    QListView *mListView;
    QLineEdit *mNameEdit;

    // actions
    QAction *mActAdd = nullptr;
    QAction *mActRemove = nullptr;
    QAction *mActDuplicate = nullptr;
    QAction *mActImport = nullptr;
    QAction *mActExport = nullptr;
    QAction *mActEdit = nullptr;

    


};
