
#include "core/model/SongModel.hpp"


SongModel::SongModel(Module &mod, QObject *parent) :
    QObject(parent),
    mModule(mod),
    mTempo(0.0f)
{
    connect(&mod, &Module::reloaded, this, &SongModel::reload);
}

void SongModel::reload() {
    auto song = mModule.song();
    emit rowsPerBeatChanged(song->rowsPerBeat());
    emit rowsPerMeasureChanged(song->rowsPerMeasure());
    emit speedChanged((int)song->speed());
    emit patternSizeChanged(song->patterns().rowSize());
    calcTempo(true);
}

int SongModel::rowsPerBeat() {
    return mModule.song()->rowsPerBeat();
}

int SongModel::rowsPerMeasure() {
    return mModule.song()->rowsPerMeasure();
}

int SongModel::speed() {
    return mModule.song()->speed();
}

int SongModel::patternSize() {
    return mModule.song()->patterns().rowSize();
}

trackerboy::Speed SongModel::estimateSpeed(int tempo) {
    return mModule.song()->estimateSpeed(tempo, mModule.data().framerate());
}

void SongModel::setRowsPerBeat(int rpb) {
    auto song = mModule.song();

    if (song->rowsPerBeat() != (uint8_t)rpb) {
        {
            auto ctx = mModule.permanentEdit();
            song->setRowsPerBeat((uint8_t)rpb);
        }
        calcTempo();
        emit rowsPerBeatChanged(rpb);
    }

}

void SongModel::setRowsPerMeasure(int rpm) {
    auto song = mModule.song();

    if (song->rowsPerMeasure() != (uint8_t)rpm) {
        {
            auto ctx = mModule.permanentEdit();
            song->setRowsPerMeasure((uint8_t)rpm);
        }
        emit rowsPerMeasureChanged(rpm);
    }
}

void SongModel::setSpeed(int speed) {
    auto song = mModule.song();

    if (song->speed() != (trackerboy::Speed)speed) {
        {
            auto ctx = mModule.permanentEdit();
            song->setSpeed((trackerboy::Speed)speed);
        }
        calcTempo();
        emit speedChanged(speed);
    }
}

void SongModel::setSpeedFromTempo(int tempo) {
    setSpeed(mModule.song()->estimateSpeed(tempo, mModule.data().framerate()));
}

void SongModel::setPatternSize(int rows) {
    auto &pm = mModule.song()->patterns();

    if (pm.rowSize() != (uint16_t)rows) {
        {
            auto ctx = mModule.permanentEdit();
            pm.setRowSize((uint16_t)rows);
        }
        emit patternSizeChanged(rows);
    }
}

void SongModel::calcTempo(bool notify) {
    auto tempo = mModule.song()->tempo(mModule.data().framerate());
    if (notify || !qFuzzyCompare(tempo, mTempo)) {
        mTempo = tempo;
        emit tempoChanged(tempo);
    }
}
