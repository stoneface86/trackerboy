#pragma once

#include "core/Document.hxx"
#include "core/name.hxx"

#include <QAbstractListModel>

class NameListModel final : public QAbstractListModel {

    Q_OBJECT

public:
    explicit NameListModel(Document *doc, B::ItemCategory cat,
                           QString defaultName, QObject *parent = nullptr);

    [[nodiscard]] Qt::ItemFlags flags(QModelIndex const &index) const override;

    [[nodiscard]] int
    rowCount(QModelIndex const &index = QModelIndex()) const override;

    [[nodiscard]] QVariant data(QModelIndex const &index,
                                int role = Qt::DisplayRole) const override;

    [[nodiscard]] Document *document() const;

    [[nodiscard]] B::ItemCategory category() const;

    [[nodiscard]] QString const &defaultName() const;

    [[nodiscard]] QString const &name(int index) const;

    void setName(int index, QString const &name);

    [[nodiscard]] NameList const &list() const;
    void setList(NameList const &list);

    void load(bool newModule = false);

    void commit();

    [[nodiscard]] QString prefixId(u8 id) const;

    //
    // Insert a new Name with the given id and string at the given index
    // position.
    //
    void insert(int at, u8 id, QString const &name);

    //
    // Removes the given name at the given index
    //
    u8 remove(int at);

private:
    Document *_document;
    NameList _list;
    B::ItemCategory const _cat;
    B::Itemizer _itemizer;
    QString _defaultName;
};