
#pragma once

#include "core/Module.hpp"

#include <QAbstractListModel>

#include <vector>

class SongListModel : public QAbstractListModel {

    Q_OBJECT

public:
    explicit SongListModel(Module &mod, QObject *parent = nullptr);
    virtual ~SongListModel() = default;

    void commit();

    virtual int rowCount(QModelIndex const& index = QModelIndex()) const override;

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

    virtual bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole) override;

    void rename(int index, QString const& name);

    void append();

    void remove(int index);

    void duplicate(int index);

    void moveUp(int index);

    void moveDown(int index);

private slots:
    void reload();

private:

    struct SongMeta {
        QString name;
        bool shouldCommit;

        SongMeta(std::string const& name);
    };


    Module &mModule;
    std::vector<SongMeta> mSongData;

};
