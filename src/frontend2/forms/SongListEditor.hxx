
#pragma once

#include "core/Document.hxx"
#include "model/SongListEditorModel.hxx"

#include <QDialog>
#include <QTreeView>

class SongListEditor : public QDialog {
    Q_OBJECT
public:
    explicit SongListEditor(SongListModel *model, QWidget *parent = nullptr);

    void applyChanges(Document &document);
    void revertChanges();

private:
    Q_DISABLE_COPY(SongListEditor)

    void duplicate();
    void moveUp();
    void moveDown();

    SongListEditorModel *mModel;
    QTreeView *mView;
};
