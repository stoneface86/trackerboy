
#pragma once

#include "core/model/BaseModel.hpp"
#include "core/model/OrderModel.hpp"

#include "trackerboy/data/Song.hpp"

//
// Model class for a module's song list.
//
class SongListModel : public BaseModel {

    Q_OBJECT

public:
    SongListModel(ModuleDocument &document, OrderModel &orderModel);

    trackerboy::Song* currentSong();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    virtual QString nameAt(int index) override;

    OrderModel& orderModel();


signals:
    void patternSizeChanged(int rows);
    void rowsPerBeatChanged(int rpb);
    void rowsPerMeasureChanged(int rpm);

public slots:
    void setRowsPerBeat(int rowsPerBeat);
    void setRowsPerMeasure(int rowsPerMeasure);
    void setSpeed(int speed);
    void setPatterns(int patterns);
    void setRowsPerPattern(int rows);


protected:
    virtual bool canAdd() override;

    virtual bool canRemove() override;
    
    virtual void dataAdd() override;

    virtual void dataRemove(int row) override;

    virtual void dataDuplicate(int row) override;

    virtual void dataRename(int index, const QString &name) override;

    virtual int nextIndex() override;

    virtual void dataSelected(int index) override;

private:
    Q_DISABLE_COPY(SongListModel)

    OrderModel &mOrderModel;

    std::vector<trackerboy::Song> &mSongVector;


};
