
#include "core/model/SongModel.hpp"
#include "core/model/ModuleDocument.hpp"


SongModel::SongModel(ModuleDocument &doc) :
    QObject(),
    mDocument(doc)
{
}

void SongModel::reload() {
    auto &song = mDocument.mod().song();
    emit rowsPerBeatChanged(song.rowsPerBeat());
    emit rowsPerMeasureChanged(song.rowsPerMeasure());
    emit speedChanged((int)song.speed());
    emit patternSizeChanged(song.patterns().rowSize());
}

int SongModel::rowsPerBeat() {
    return mDocument.mod().song().rowsPerBeat();
}

int SongModel::rowsPerMeasure() {
    return mDocument.mod().song().rowsPerMeasure();
}

int SongModel::speed() {
    return mDocument.mod().song().speed();
}

int SongModel::patternSize() {
    return mDocument.mod().song().patterns().rowSize();
}

void SongModel::setRowsPerBeat(int rpb) {
    auto &song = mDocument.mod().song();

    if (song.rowsPerBeat() != (uint8_t)rpb) {
        {
            auto ctx = mDocument.beginEdit();
            song.setRowsPerBeat((uint8_t)rpb);
        }
        emit rowsPerBeatChanged(rpb);
    }

}

void SongModel::setRowsPerMeasure(int rpm) {
    auto &song = mDocument.mod().song();

    if (song.rowsPerMeasure() != (uint8_t)rpm) {
        {
            auto ctx = mDocument.beginEdit();
            song.setRowsPerMeasure((uint8_t)rpm);
        }
        emit rowsPerMeasureChanged(rpm);
    }
}

void SongModel::setSpeed(int speed) {
    auto &song = mDocument.mod().song();

    if (song.speed() != (trackerboy::Speed)speed) {
        {
            auto ctx = mDocument.beginEdit();
            song.setSpeed((trackerboy::Speed)speed);
        }
        emit speedChanged(speed);
    }
}

void SongModel::setPatternSize(int rows) {
    auto &pm = mDocument.mod().song().patterns();

    if (pm.rowSize() != (uint16_t)rows) {
        {
            auto ctx = mDocument.beginEdit();
            pm.setRowSize((uint16_t)rows);
        }
        emit patternSizeChanged(rows);
    }
}