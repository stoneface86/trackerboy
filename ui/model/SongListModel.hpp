
#pragma once

#include "model/BaseModel.hpp"
#include "model/OrderModel.hpp"

#include "trackerboy/data/Song.hpp"

class SongListModel : public BaseModel {

    Q_OBJECT

public:
    SongListModel(ModuleDocument &document);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    virtual QString name() override;

    OrderModel* orderModel() const;

protected:
    virtual bool canAdd() override;

    virtual bool canRemove() override;
    
    virtual void dataAdd() override;

    virtual void dataRemove(int row) override;

    virtual void dataDuplicate(int row) override;

    virtual void dataRename(const QString &name) override;

    virtual int nextIndex() override;

    virtual void dataSelected(int index) override;

private:
    OrderModel *mOrderModel;

    std::vector<trackerboy::Song> &mSongVector;


};
