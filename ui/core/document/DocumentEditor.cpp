
#include "core/document/DocumentEditor.hpp"
#include "core/document/Document.hpp"

DocumentEditor::EditContext::EditContext(DocumentEditor &editor, QMutex &mutex) :
    CommandEditContext(editor.mDocument.mModule, mutex),
    mEditor(editor)
{
}

DocumentEditor::EditContext::~EditContext() {
    unlock();
    // set perma dirty
    mEditor.mDocument.makeDirty();
}


DocumentEditor::DocumentEditor(Document &document) :
    mDocument(document)
{
}

DocumentEditor::CommandEditContext DocumentEditor::commandEdit() {
    return { mDocument.mModule, mDocument.mMutex };
}

DocumentEditor::EditContext DocumentEditor::edit() {
    return { *this, mDocument.mMutex };
}

Document& DocumentEditor::document() {
    return mDocument;
}
