
#include "core/Document.hxx"

Document::EditContext::EditContext(Document &doc, bool setModified) :
    backend(doc.mSource.ref),
    document(doc),
    setModified(setModified)
{
    B::lock(backend);
}

Document::EditContext::~EditContext() {
    B::unlock(backend);
    if (setModified) {
        document.setModified();
    }
}

Document::Document(QObject *parent) :
    QObject(parent),
    mSource(makeNimRef(B::newDocument()))
{
}

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

B::Document* Document::source() {
    return mSource.ref;
}