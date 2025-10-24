
#include "model/SongModel.hxx"
#include "utils/backendutils.hxx"

SongModel::SongModel(Document *doc, NameListModel *listModel, QObject *parent)
    : QObject(parent)
    , _document(doc)
    , _listModel(listModel) {}

Document *SongModel::document() const {
    return _document;
}

NameListModel *SongModel::listModel() const {
    return _listModel;
}

QString SongModel::name() const {
    return _listModel->name(_document->song());
}

int SongModel::rowsPerBeat() const {
    return _document->view()->song.rowsPerBeat();
}

int SongModel::rowsPerMeasure() const {
    return _document->view()->song.rowsPerMeasure();
}

int SongModel::speed() const {
    return _document->view()->song.speed();
}

float SongModel::speedFloat() const {
    return _document->view()->song.speedFloat();
}

float SongModel::tempo() const {
    return _document->view()->song.tempo();
}

int SongModel::patternSize() const {
    return _document->view()->song.trackLen();
}

bool SongModel::hasTickrate() const {
    return _document->view()->song.hasTickrate();
}

BTickrate SongModel::tickrate() const {
    return _document->view()->song.tickrate();
}

void SongModel::setName(QString const &name) {
    _listModel->setName(_document->song(), name);
}

void SongModel::setRowsPerBeat(int rpb) {
    if (rpb != rowsPerBeat()) {
        _document->edit(true)->song.setRowsPerBeat(rpb);
        emit rowsPerBeatChanged(rpb);
    }
}

void SongModel::setRowsPerMeasure(int rpm) {
    if (rpm != rowsPerMeasure()) {
        _document->edit(true)->song.setRowsPerMeasure(rpm);
        emit rowsPerMeasureChanged(rpm);
    }
}

void SongModel::setSpeed(int speed_) {
    if (speed_ != speed()) {
        _document->edit(true)->song.setSpeed(speed_);
        emit speedChanged(speed_);
    }
}

void SongModel::setSpeedFromTempo(int tempo) {}

void SongModel::setPatternSize(int rows) {
    if (rows != patternSize()) {
        _document->edit(true)->song.setTrackLen(rows);
        emit patternSizeChanged(rows);
    }
}

void SongModel::clearTickrate() {
    if (hasTickrate()) {
        _document->edit(true)->song.clearTickrate();
        emit tickrateChanged();
    }
}

void SongModel::setTickrate(BTickrate tickrate) {
    if (!_document->view()->song.tickrateEqual(tickrate)) {
        _document->edit(true)->song.setTickrate(tickrate);
        emit tickrateChanged();
    }
}
