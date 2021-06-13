
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QAbstractItemModel>
#include <QUndoGroup>
#include <QVector>

#include <tuple>

//
// Model implementation to be used in a QTreeView. Shows all open modules
//
class ModuleModel : public QAbstractItemModel {

    Q_OBJECT

public:

    //
    // enum for each item type in the model.
    //
    enum class ItemType {
        invalid,
        document,
        instruments,
        orders,
        waveforms,
        instrument,
        order,
        waveform,
        settings
    };

    explicit ModuleModel(QObject *parent = nullptr);

    // model implementation

    virtual Qt::ItemFlags flags(QModelIndex const& index) const override;

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

    virtual bool hasChildren(QModelIndex const& index) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const override;

    virtual QModelIndex parent(QModelIndex const& index) const override;

    virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override;

    virtual int columnCount(QModelIndex const& parent = QModelIndex()) const override;

    // document management

    //
    // Adds the document to the model. The Model does not take ownership of
    // document. The document's undo stack is also added to the undo group.
    //
    QModelIndex addDocument(ModuleDocument *doc);

    //
    // Removes the document from the model. The document's undo stack is
    // also removed from the group.
    //
    void removeDocument(ModuleDocument *doc);
    
    //
    // Gets the vector of documents added to this model.
    //
    QVector<ModuleDocument*> const& documents() const noexcept;

    //
    // Set the current document. -1 for no document
    //
    void setCurrentDocument(int index);

    //
    // Gets the index of the document that belongs to the given model index.
    // All items in the model have an associated document. -1 is only returned
    // when the index is invalid.
    //
    int documentIndex(QModelIndex const& index);

    //
    // Gets the ItemType for the given model index.
    //
    ItemType itemAt(QModelIndex const& index);

    //
    // Gets the current document set by setCurrentDocument
    //
    ModuleDocument* currentDocument() const noexcept;

    //
    // Gets the QUndoGroup for the model. This group contains all QUndoStacks
    // from every document in the model.
    //
    QUndoGroup& undoGroup() noexcept;

    //
    // Moves the given document located at the from index and puts it the to
    // index.
    //
    void moveDocument(int from, int to);

signals:

    //
    // Signal emitted when the current document changes.
    //
    void currentDocumentChanged(ModuleDocument *doc);


private:

    // context data for a child node: the document it belongs to and
    // the row (0 = instrument, 1 = song order, 2 = waveforms)
    using ChildModelContext = std::tuple<ModuleDocument*, int>;

    // insertChildRows and removeChildRows are slots but we cannot mark
    // them as such since they are templated

    template <class tModel>
    void insertChildRows(QModelIndex const& index, int first, int last);
    void _insertChildRows(ChildModelContext ctx, int first, int last);

    template <class tModel>
    void removeChildRows(QModelIndex const& index, int first, int last);
    void _removeChildRows(ChildModelContext ctx, int first, int last);

    template <class tModel>
    void childDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void _childDataChanged(ChildModelContext ctx, const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    template <class T>
    ChildModelContext getChildNode(QObject *sender);

    template <class tModel>
    void connectChildModel(tModel &model);

    QVector<ModuleDocument*> mDocuments;
    ModuleDocument *mCurrent;

    QUndoGroup mUndoGroup;
};
