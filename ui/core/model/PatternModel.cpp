
#include "core/model/PatternModel.hpp"

#include "core/model/ModuleDocument.hpp"

PatternModel::PatternModel(ModuleDocument &doc, QObject *parent) :
    QObject(parent),
    mDocument(doc),
    mCursorRow(0),
    mCursorColumn(0),
    mRecording(false),
    mFollowing(true),
    mPlaying(false),
    mShowPreviews(true),
    mTrackerRow(0),
    mTrackerPattern(0),
    mPatternPrev(),
    mPatternCurr(doc.mod().song().getPattern(0)),
    mPatternNext()
{
    // OrderModel must be initialized first in order for this to work!
    auto &orderModel = doc.orderModel();
    connect(&orderModel, &OrderModel::currentPatternChanged, this, &PatternModel::setCursorPattern);
    connect(&orderModel, &OrderModel::currentTrackChanged, this, &PatternModel::setCursorTrack);
    connect(&orderModel, &OrderModel::patternsChanged, this, [this]() {
        setCursorPattern(mDocument.orderModel().currentPattern());
    });
}

void PatternModel::reload() {
    // patternCurr was invalidated
    mPatternCurr = mDocument.mod().song().getPattern(0);
    setCursorPattern(0);
}

trackerboy::Pattern* PatternModel::previousPattern() {
    return mPatternPrev ? &*mPatternPrev : nullptr;
}

trackerboy::Pattern& PatternModel::currentPattern() {
    return mPatternCurr;
}

trackerboy::Pattern* PatternModel::nextPattern() {
    return mPatternNext ? &*mPatternNext : nullptr;
}

int PatternModel::cursorRow() const {
    return mCursorRow;
}

int PatternModel::cursorColumn() const {
    return mCursorColumn;
}

int PatternModel::trackerCursorRow() const {
    return mTrackerRow;
}

int PatternModel::trackerCursorPattern() const {
    return mTrackerPattern;
}

bool PatternModel::isRecording() const {
    return mRecording;
}

bool PatternModel::isFollowing() const {
    return mFollowing;
}

bool PatternModel::isPlaying() const {
    return mPlaying;
}

// slots -------------------------------

void PatternModel::moveCursorRow(int amount) {
    setCursorRow(mCursorRow + amount);
}

void PatternModel::moveCursorColumn(int amount) {
    setCursorColumn(mCursorColumn + amount);
}

void PatternModel::setCursorRow(int row) {
    if (mCursorRow == row) {
        return;
    }

    //auto &song = mDocument->mod().song();
    auto &orderModel = mDocument.orderModel();
    auto const pattern = orderModel.currentPattern();

    if (row < 0) {
        // go to the previous pattern or wrap around to the last one
        orderModel.selectPattern(pattern == 0 ? orderModel.rowCount() - 1 : pattern - 1);
        mCursorRow = std::max(0, (int)mPatternCurr.totalRows() + row);

    } else if (row >= (int)mPatternCurr.totalRows()) {
        // go to the next pattern or wrap around to the first one
        row -= mPatternCurr.totalRows();
        auto nextPattern = pattern + 1;
        if (nextPattern == orderModel.rowCount()) {
            nextPattern = 0;
        }
        orderModel.selectPattern(nextPattern);
        mCursorRow = std::min((int)mPatternCurr.totalRows() - 1, row);
    } else {
        mCursorRow = row;
    }

    emit cursorRowChanged(mCursorRow);
}

void PatternModel::setCursorColumn(int column) {
    if (mCursorColumn == column) {
        return;
    }

    if (column < 0) {
        column = COLUMNS - (-column % COLUMNS);
    } else if (column >= COLUMNS) {
        column = column % COLUMNS;
    }

    int track = mCursorColumn / COLUMNS_PER_TRACK;
    int newtrack = column / COLUMNS_PER_TRACK;
    if (track != newtrack) {
        mDocument.orderModel().selectTrack(newtrack);
    }

    mCursorColumn = column;
    emit cursorColumnChanged(mCursorColumn);
}

void PatternModel::setCursorTrack(int track) {
    Q_ASSERT(track >= 0 && track < 4);

    auto col = track * COLUMNS_PER_TRACK;
    if (mCursorColumn != col) {
        mCursorColumn = col;
        emit cursorColumnChanged(col);
    }
}

void PatternModel::setCursorPattern(int pattern) {
    auto &song = mDocument.mod().song();

    
    if (mShowPreviews) {
        setPreviewPatterns(pattern);
    }

    // update the current pattern
    auto oldsize = (int)mPatternCurr.totalRows();
    mPatternCurr = song.getPattern((uint8_t)pattern);
    auto newsize = (int)mPatternCurr.totalRows();

    if (oldsize != newsize) {
        emit patternSizeChanged(newsize);
    }

    emit patternsChanged();

    if (mCursorRow >= newsize) {
        mCursorRow = newsize - 1;
        emit cursorRowChanged(mCursorRow);
    }
}

void PatternModel::setTrackerCursor(int row, int pattern) {
    bool changed = false;
    if (mTrackerPattern != pattern) {
        mTrackerPattern = pattern;
        if (mFollowing) {
            setCursorPattern(pattern);
        }
        changed = true;
    }
    if (mTrackerRow != row) {
        mTrackerRow = row;
        if (mFollowing) {
            setCursorRow(row);
        }
        changed = true;
    }

    if (changed) {
        emit trackerCursorChanged(row, pattern);
    }
}

void PatternModel::setFollowing(bool following) {
    mFollowing = following;
}

void PatternModel::setPlaying(bool playing) {
    if (mPlaying != playing) {
        mPlaying = playing;
        emit playingChanged(playing);
    }
}

void PatternModel::setRecord(bool record) {
    if (mRecording != record) {
        mRecording = record;
        emit recordingChanged(record);
    }
}

void PatternModel::setPreviewEnable(bool enable) {
    if (mShowPreviews != enable) {
        mShowPreviews = enable;
        if (enable) {
            setPreviewPatterns(mDocument.orderModel().currentPattern());
        } else {
            mPatternPrev.reset();
            mPatternNext.reset();
        }
        emit patternsChanged();
    }
}

void PatternModel::setPreviewPatterns(int pattern) {
    auto &song = mDocument.mod().song();
    // get the previous pattern for preview
    if (pattern > 0) {
        mPatternPrev.emplace(song.getPattern((uint8_t)pattern - 1));
    } else {
        mPatternPrev.reset();
    }
    // get the next pattern for preview
    auto nextPattern = pattern + 1;
    if (nextPattern < song.order().size()) {
        mPatternNext.emplace(song.getPattern((uint8_t)nextPattern));
    } else {
        mPatternNext.reset();
    }
}
