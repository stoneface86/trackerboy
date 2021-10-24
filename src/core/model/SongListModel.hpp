
#pragma once

#include "core/Module.hpp"

#include <QAbstractListModel>

#include <memory>
#include <string>
#include <vector>

//
// List model for the song list.
//
class SongListModel : public QAbstractListModel {

    Q_OBJECT

public:
    explicit SongListModel(Module &mod, QObject *parent = nullptr);

    void commit();

    virtual Qt::ItemFlags flags(QModelIndex const& index) const override;

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

    //
    // Meta-info struct for a single song. Contains a QString version of the name with modified flag
    // and a QUndoStack.
    //
    struct SongMeta {
        QString name;
        bool shouldCommit;

        // use existing QString, shouldCommit is set to true
        SongMeta(QString const& str);

        // convert in name from song, shouldCommit is set to false
        SongMeta(std::string const& name);
    };


    Module &mModule;
    std::vector<SongMeta> mSongData;

};
