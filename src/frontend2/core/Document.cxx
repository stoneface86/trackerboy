
#include "core/Document.hxx"
#include "utils/backendutils.hxx"

#include <QUndoStack>
#include <QtDebug>

Document::EditContext::EditContext(Document &doc, bool setModified)
    : backend(doc._source.ref)
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
    : backend(doc._source.ref)
    , document(doc) {}

Document::ViewContext::~ViewContext() {}

Document::Document(QObject *parent)
    : QObject(parent)
    , _source(makeNimRef(B::newDocument()))
    , _modified(false)
    , _undoGroup(new QUndoGroup(this))
    , _songHistories()
    , _currentSong(0) {
    _songHistories = initHistoryFromSource();
    _undoGroup->setActiveStack(_songHistories[0].stack);
}

QString Document::defaultSongName() const {
    return tr("New Song");
}

void Document::setModified() {
    if (!_modified) {
        _modified = true;
        emit modifiedChanged(true);
    }
}

void Document::selectSong(int songNo) {
    if (_currentSong != songNo) {
        selectSongImpl(songNo);
    }
}

int Document::song() const {
    return _currentSong;
}

Document::EditContext Document::edit(bool setModified) {
    EditContext result(*this, setModified);
    return result;
}

void Document::edit(QUndoCommand *cmd) {
    _undoGroup->activeStack()->push(cmd);
}

Document::ViewContext Document::view() const {
    ViewContext result(*this);
    return result;
}

QUndoGroup const *Document::undoGroup() const {
    return _undoGroup;
}

QList<Document::SongHistory> Document::initHistoryFromSource() {
    QList<Document::SongHistory> result;
    auto const songCount = _source->mod.songCount();
    for (B::NI i = 0; i < songCount; ++i) {
        result.append({_source->mod.songId(i), nullptr});
    }
    return result;
}

void Document::changeSongList(B::SongListChanges const &changes) {
    edit(true)->setSongList(changes);

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
    for (auto const &h : _songHistories) {
        auto index = findHistoryById(nextHistory, h.id);
        if (index == -1) {
            // not found, delete the stack and remove it from the group
            qDebug() << "Deleting history for song id " << h.id;
            _undoGroup->removeStack(h.stack);
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
            _undoGroup->addStack(h.stack);
        }
    }
    auto const song =
        findHistoryById(nextHistory, _songHistories[_currentSong].id);

    _songHistories = std::move(nextHistory);
    // re-select the current song. If the current song was deleted, select the
    // first one
    selectSongImpl(song == -1 ? 0 : song);
}

void Document::selectSongImpl(int songNo) {
    _source->selectSong(songNo);
    _currentSong = songNo;
    _undoGroup->setActiveStack(_songHistories[songNo].stack);
    emit songChanged(songNo);
}
