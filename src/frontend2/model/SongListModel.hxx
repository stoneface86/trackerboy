#pragma once

#include "core/Document.hxx"
#include <QAbstractListModel>

class SongListModel : public QAbstractListModel {
    friend class SongListEditorModel;

    Q_OBJECT

public:
    explicit SongListModel(Document const *doc, QObject *parent = nullptr);

    virtual Qt::ItemFlags flags(QModelIndex const &index) const override;

    virtual int
    rowCount(QModelIndex const &index = QModelIndex()) const override;

    virtual QVariant data(QModelIndex const &index,
                          int role = Qt::DisplayRole) const override;

private:
    Q_DISABLE_COPY(SongListModel)

    void reload();

    Document const *mDocument;
};