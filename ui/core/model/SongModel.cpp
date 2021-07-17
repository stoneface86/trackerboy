
#include "core/model/SongModel.hpp"


SongModel::SongModel(Module &mod) :
    QObject(),
    mModule(mod)
{
}

void SongModel::reload() {
    auto &song = mModule.data().song();
    emit rowsPerBeatChanged(song.rowsPerBeat());
    emit rowsPerMeasureChanged(song.rowsPerMeasure());
    emit speedChanged((int)song.speed());
    emit patternSizeChanged(song.patterns().rowSize());
}

int SongModel::rowsPerBeat() {
    return mModule.data().song().rowsPerBeat();
}

int SongModel::rowsPerMeasure() {
    return mModule.data().song().rowsPerMeasure();
}

int SongModel::speed() {
    return mModule.data().song().speed();
}

int SongModel::patternSize() {
    return mModule.data().song().patterns().rowSize();
}

void SongModel::setRowsPerBeat(int rpb) {
    auto &song = mModule.data().song();

    if (song.rowsPerBeat() != (uint8_t)rpb) {
        {
            auto ctx = mModule.permanentEdit();
            song.setRowsPerBeat((uint8_t)rpb);
        }
        emit rowsPerBeatChanged(rpb);
    }

}

void SongModel::setRowsPerMeasure(int rpm) {
    auto &song = mModule.data().song();

    if (song.rowsPerMeasure() != (uint8_t)rpm) {
        {
            auto ctx = mModule.permanentEdit();
            song.setRowsPerMeasure((uint8_t)rpm);
        }
        emit rowsPerMeasureChanged(rpm);
    }
}

void SongModel::setSpeed(int speed) {
    auto &song = mModule.data().song();

    if (song.speed() != (trackerboy::Speed)speed) {
        {
            auto ctx = mModule.permanentEdit();
            song.setSpeed((trackerboy::Speed)speed);
        }
        emit speedChanged(speed);
    }
}

void SongModel::setPatternSize(int rows) {
    auto &pm = mModule.data().song().patterns();

    if (pm.rowSize() != (uint16_t)rows) {
        {
            auto ctx = mModule.permanentEdit();
            pm.setRowSize((uint16_t)rows);
        }
        emit patternSizeChanged(rows);
    }
}