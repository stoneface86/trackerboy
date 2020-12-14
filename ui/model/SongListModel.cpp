
#include "model/SongListModel.hpp"

SongListModel::SongListModel(ModuleDocument &document) :
    BaseModel(document),
    mOrderModel(document),
    mSongVector(document.songs())
{
    select(0); // we always have 1 song so the select the first one
}

int SongListModel::rowCount(const QModelIndex &parent) const {
    return mSongVector.size();
}

QVariant SongListModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        QString fmt("%1: %2");
        return QVariant(fmt.arg(
            QString::number(index.row() + 1),
            QString::fromStdString(mSongVector[index.row()].name())
        ));
    }
    return QVariant();
}

QString SongListModel::nameAt(int index) {
    return QString::fromStdString(mSongVector[index].name());
}

OrderModel& SongListModel::orderModel() {
    return mOrderModel;
}

void SongListModel::setRowsPerBeat(int rowsPerBeat) {
    auto &curr = mSongVector[mCurrentIndex];
    uint8_t rpb = static_cast<uint8_t>(rowsPerBeat);
    if (rpb != curr.rowsPerBeat()) {
        auto ctx = mDocument.beginEdit();
        curr.setRowsPerBeat(rpb);
    }
}

void SongListModel::setRowsPerMeasure(int rowsPerMeasure) {
    auto &curr = mSongVector[mCurrentIndex];
    uint8_t rpm = static_cast<uint8_t>(rowsPerMeasure);
    if (rpm != curr.rowsPerMeasure()) {
        auto ctx = mDocument.beginEdit();
        curr.setRowsPerMeasure(rpm);
    }
}

void SongListModel::setSpeed(int speed) {
    auto &curr = mSongVector[mCurrentIndex];
    auto _speed = static_cast<trackerboy::Speed>(speed);
    if (_speed != curr.speed()) {
        auto editCtx = mDocument.beginEdit();
        curr.setSpeed(_speed);
    }
}

void SongListModel::setPatterns(int patterns) {
    auto &curr = mSongVector[mCurrentIndex];
    int currPatterns = mOrderModel.rowCount();
    if (patterns > currPatterns) {
        // grow
        mOrderModel.insertRows(currPatterns, patterns - currPatterns);
    } else if (patterns < currPatterns) {
        // shrink
        int amount = currPatterns - patterns;
        mOrderModel.removeRows(currPatterns - amount, amount);
    }
}

void SongListModel::setRowsPerPattern(int rows) {
    // DO NOT CALL THIS IF THE RENDERER IS PLAYING MUSIC
    auto &pm = mSongVector[mCurrentIndex].patterns();
    if (pm.rowSize() != rows) {
        {
            auto ctx = mDocument.beginEdit();
            pm.setRowSize(rows);
        }
        emit patternSizeChanged(rows);
    }
}

trackerboy::Song* SongListModel::currentSong() {
    return &mSongVector[mCurrentIndex];
}

bool SongListModel::canAdd() {
    return mSongVector.size() < 256;
}

bool SongListModel::canRemove() {
    // must have at least one song
    return mSongVector.size() > 1;
}

void SongListModel::dataAdd() {
    mSongVector.emplace_back();
}

void SongListModel::dataRemove(int row) {
    mSongVector.erase(mSongVector.begin() + row);
}

void SongListModel::dataDuplicate(int row) {
    // emplace using Song's copy constructor
    mSongVector.emplace_back(mSongVector[row]);
}

void SongListModel::dataRename(int index, const QString &name) {
    mSongVector[index].setName(name.toStdString());
}

int SongListModel::nextIndex() {
    // songs always get added/duplicated to the end of the list
    return mSongVector.size();
}

void SongListModel::dataSelected(int index) {
    // index is only -1 when the model is being reset
    if (index != -1) {
        mOrderModel.setOrder(&mSongVector[index].orders());
    }
}
