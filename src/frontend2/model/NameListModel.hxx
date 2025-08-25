#pragma once

#include "core/Document.hxx"
#include "core/name.hxx"

#include <QAbstractListModel>

class NameListModel : public QAbstractListModel {

    Q_OBJECT

public:
    explicit NameListModel(Document *doc, B::ItemCategory cat,
                           QObject *parent = nullptr);

    virtual Qt::ItemFlags flags(QModelIndex const &index) const override;

    virtual int
    rowCount(QModelIndex const &index = QModelIndex()) const override;

    virtual QVariant data(QModelIndex const &index,
                          int role = Qt::DisplayRole) const override;

    Document *document() const;

    QString const &name(int index) const;

    void setName(int index, QString const &name);

    NameList const &list() const;
    void setList(NameList const &list);

    void load(bool newModule = false);

    void commit();

private:
    Document *_document;
    NameList _list;
    B::ItemCategory const _cat;
};