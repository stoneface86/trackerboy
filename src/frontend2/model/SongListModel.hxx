#pragma once

#include "core/Document.hxx"
#include <QAbstractListModel>
#include <QStringList>

class SongListModel : public QAbstractListModel {
    friend class SongListEditorModel;
    friend class SongModel;

    Q_OBJECT

public:
    explicit SongListModel(Document *doc, QObject *parent = nullptr);

    virtual Qt::ItemFlags flags(QModelIndex const &index) const override;

    virtual int
    rowCount(QModelIndex const &index = QModelIndex()) const override;

    virtual QVariant data(QModelIndex const &index,
                          int role = Qt::DisplayRole) const override;

    QString name(int song) const;
    void setName(int song, QString const &name);

private:
    Q_DISABLE_COPY(SongListModel)

    void reload(bool newModule);

    void setNames(QStringList const &list);

    void commit();

    Document *_document;
    QStringList _nameCache;
    bool _cacheDirty;
};