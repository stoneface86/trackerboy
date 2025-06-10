
#include "core/Document.hxx"
#include "utils/backendutils.hxx"

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
    , mSource(makeNimRef(B::newDocument(toNimString(tr("New Song")).s))) {}

void Document::setModified() {
    if (!mModified) {
        mModified = true;
        emit modifiedChanged(true);
    }
}

Document::EditContext Document::edit(bool setModified) {
    EditContext result(*this, setModified);
    return result;
}

Document::ViewContext Document::view() const {
    ViewContext result(*this);
    return result;
}
