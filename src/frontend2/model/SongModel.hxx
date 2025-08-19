
#pragma once

#include "core/Document.hxx"
#include "model/SongListModel.hxx"

#include <QObject>

class SongModel : public QObject {

    Q_OBJECT

public:
    explicit SongModel(Document *doc, SongListModel *listModel,
                       QObject *parent = nullptr);

    Document *document() const;
    SongListModel *listModel() const;

    QString name() const;

    int rowsPerBeat() const;
    int rowsPerMeasure() const;
    int speed() const;
    float speedFloat() const;
    float tempo() const;
    int patternSize() const;

    bool hasTickrate() const;
    B::Tickrate tickrate() const;

    void setName(QString const &name);
    void setRowsPerBeat(int rpb);
    void setRowsPerMeasure(int rpm);
    void setSpeed(int speed);
    void setSpeedFromTempo(int tempo);
    void setPatternSize(int rows);
    void setTickrate(B::Tickrate tickrate);
    void clearTickrate();

signals:
    void rowsPerBeatChanged(int rpb);
    void rowsPerMeasureChanged(int rpm);
    void patternSizeChanged(int rows);
    void speedChanged(int speed);
    void tickrateChanged();

private:
    Q_DISABLE_COPY(SongModel)

    Document *_document;
    SongListModel *_listModel;
};