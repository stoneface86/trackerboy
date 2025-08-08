
#include "core/Document.hxx"
#include "utils/backendutils.hxx"

#include <QUndoStack>
#include <QtDebug>

Document::EditContext::EditContext(Document &doc, bool setModified)
    : backend(doc.mSource.ref)
    , document(doc)
    , setModified(setModified) {
    backend->lock();
}

Document::EditContext::~EditContext() {
    backend->unlock();
    if (setModified) {
        document.setModified();
    }
}

Document::ViewContext::ViewContext(Document const &doc)
    : backend(doc.mSource.ref)
    , document(doc) {}

Document::ViewContext::~ViewContext() {}

Document::Document(QObject *parent)
    : QObject(parent)
    , mSource(makeNimRef(B::newDocument(toNimString(tr("New Song")).s)))
    , mModified(false)
    , mUndoGroup(new QUndoGroup(this))
    , mSongHistories() {
    songListChanged();
}

void Document::setModified() {
    if (!mModified) {
        mModified = true;
        emit modifiedChanged(true);
    }
}

void Document::selectSong(int songNo) {
    if (mCurrentSong != songNo) {
        mSource->selectSong(songNo);
        mCurrentSong = songNo;
        mUndoGroup->setActiveStack(mSongHistories[songNo].stack);
        emit songChanged(songNo);
    }
}

int Document::song() const {
    return mCurrentSong;
}

Document::EditContext Document::edit(bool setModified) {
    EditContext result(*this, setModified);
    return result;
}

void Document::edit(QUndoCommand *cmd) {
    mUndoGroup->activeStack()->push(cmd);
}

Document::ViewContext Document::view() const {
    ViewContext result(*this);
    return result;
}

QUndoGroup const *Document::undoGroup() const {
    return mUndoGroup;
}

QList<Document::SongHistory> Document::initHistoryFromSource() {
    QList<Document::SongHistory> result;
    auto const songCount = mSource->songCount();
    for (B::NI i = 0; i < songCount; ++i) {
        result.append({mSource->songId(i), nullptr});
    }
    return result;
}

void Document::songListChanged() {
    auto findHistoryById = [](QList<SongHistory> const &list,
                              qintptr id) -> int {
        int i = 0;
        for (auto const &h : list) {
            if (h.id == id) {
                return i;
            }
            ++i;
        }
        return -1;
    };

    auto nextHistory = initHistoryFromSource();
    // scan current for removed songs
    for (auto const &h : mSongHistories) {
        auto index = findHistoryById(nextHistory, h.id);
        if (index == -1) {
            // not found, delete the stack and remove it from the group
            qDebug() << "Deleting history for song id " << h.id;
            mUndoGroup->removeStack(h.stack);
            delete h.stack;
        } else {
            // move stack to the new history list
            nextHistory[index].stack = h.stack;
        }
    }

    // now if there's a nullptr stack in nextHistory, this means a new song
    // was added (or duplicated) and will need a new stack
    // There is no way to deep-copy a QUndoStack so duplicated songs will have
    // a brand new stack.
    for (auto &h : nextHistory) {
        if (h.stack == nullptr) {
            qDebug() << "Creating new history for song id " << h.id;
            h.stack = new QUndoStack(this);
            mUndoGroup->addStack(h.stack);
        }
    }
    mSongHistories = std::move(nextHistory);
}
