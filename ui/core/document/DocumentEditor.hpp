
#pragma once

#include "core/Locked.hpp"

#include "trackerboy/data/Module.hpp"

#include <QMutex>
#include <QUndoStack>

class Document;

//
// Editor interface for the Document class. All edits to the document must
// be made using this class.
//
class DocumentEditor {

public:

    //
    // CommandEditContext is just a locked trackerboy module. This
    // context is used for edits that can be undone, by using a QUndoCommand
    // subclass.
    //
    using CommandEditContext = Locked<trackerboy::Module>;

    //
    // Extends the CommandEditContext, by setting the permanent dirty
    // flag. This context should be used for edits that cannot be undone
    //
    class EditContext : public CommandEditContext {
    public:
        ~EditContext();

    private:
        friend class DocumentEditor;

        EditContext(DocumentEditor &editor, QMutex &mutex);

        DocumentEditor &mEditor;

    };

    DocumentEditor(Document &document);

    CommandEditContext commandEdit();

    EditContext edit();

    Document& document();

private:
    friend class EditContext;

    Document &mDocument;

};

