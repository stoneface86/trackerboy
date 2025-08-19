
#pragma once

#include "core/Document.hxx"
#include "forms/PersistantDialog.hxx"
#include "model/SongListEditorModel.hxx"

#include <QDialog>
#include <QTreeView>

class SongListEditor : public PersistantDialog {
    Q_OBJECT
public:
    explicit SongListEditor(SongListModel *model, QWidget *parent = nullptr);

    void applyChanges(Document &document);
    void revertChanges();

protected:
    virtual void closeEvent(QCloseEvent *evt) override;

private:
    Q_DISABLE_COPY(SongListEditor)

    void duplicate();
    void moveUp();
    void moveDown();

    SongListEditorModel *_model;
    QTreeView *_view;
};
