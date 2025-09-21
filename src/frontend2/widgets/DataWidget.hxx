
#pragma once

#include "model/TableModel.hxx"

#include <QGroupBox>
#include <QListView>
#include <QToolBar>

//
// Composite widget showing the items in module's instrument or waveform table.
//
class DataWidget final : public QGroupBox {

    Q_OBJECT

public:
    struct Actions {
        QAction *add;
        QAction *remove;
        QAction *duplicate;
        QAction *imp;
        QAction *exp;
        QAction *edit;
        QAction *showEmpty;
    };

    explicit DataWidget(NameListModel *model, QString const &title,
                        QWidget *parent = nullptr);

    [[nodiscard]] Actions const &dataActions() const;

    [[nodiscard]] bool showEmpty() const;

    void setDataActions(Actions const &actions);

    [[nodiscard]] int currentId() const;

    void setCurrentId(int id);

    void add();
    void remove();
    void duplicate();
    void edit();

signals:
    void currentIdChanged(int id);
    void editRequested(int id);

private:
    Q_DISABLE_COPY(DataWidget)

    void selectCurrentId();

    void viewSelectionChanged(QItemSelection const &selected,
                              QItemSelection const &deselected);

    void updateActions();

    TableModel *_model;
    QToolBar *_toolbar;
    QListView *_view;
    Actions _actions;
    int _currentId;
};