
#include "core/Document.hxx"
#include "utils/backendutils.hxx"

Document::EditContext::EditContext(Document &doc, bool setModified) :
    backend(doc.mSource.ref),
    document(doc),
    setModified(setModified)
{
    backend->lock();
}

Document::EditContext::~EditContext() {
    backend->unlock();
    if (setModified) {
        document.setModified();
    }
}

Document::Document(QObject *parent) :
    QObject(parent),
    mSource(makeNimRef(B::newDocument(toNimString(tr("New Song")).s)))
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

B::Document const* Document::source() const {
    return mSource.ref;
}

bool Document::open(QString const& path) {
    
}

bool Document::save() {
    if (mFilepath.isEmpty()) {
        return false;
    } else {
        return doSave(mFilepath);
    }
}

bool Document::save(QString const& filename) {
    auto const result = doSave(filename);
    if (result) {
        updateFilename(filename);
    }
    return result;
}

bool Document::doSave(QString const& filename) {
    return false;
}

void Document::updateFilename(QString const& filename) {
    
}