
#include "core/Document.hxx"
#include "model/SongListModel.hxx"
#include "model/SongListEditorModel.hxx"

#include <QApplication>
#include <QWidget>
#include <QListView>
#include <QVBoxLayout>
#include <QTreeView>
#include <QPushButton>


int main(int argc, char *argv[]) {
    B::NimMain();
    B::init();

    QApplication app(argc, argv);

    auto window = new QWidget;

    auto doc = new Document(window);

    auto listModel = new SongListModel(doc, window);
    auto editorModel = new SongListEditorModel(listModel, window);


    auto layout = new QVBoxLayout;
    auto listbox = new QListView;
    listbox->setModel(listModel);
    auto treeview = new QTreeView;
    treeview->setModel(editorModel);
    treeview->setSelectionMode(QAbstractItemView::SingleSelection);
    treeview->setDragEnabled(true);
    treeview->viewport()->setAcceptDrops(true);
    treeview->setDragDropMode(QAbstractItemView::InternalMove);

    auto treeviewLayout = new QHBoxLayout;
    auto buttonLayout = new QVBoxLayout;
    treeviewLayout->addWidget(treeview);
    treeviewLayout->addLayout(buttonLayout);
    auto buttonAdd = new QPushButton("New");
    auto buttonDuplicate = new QPushButton("Duplicate");
    auto buttonMoveUp = new QPushButton("Move Up");
    auto buttonMoveDown = new QPushButton("Move Down");
    auto buttonDiscard = new QPushButton("Discard Changes");
    auto buttonApply = new QPushButton("Apply");
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonDuplicate);
    buttonLayout->addWidget(buttonMoveUp);
    buttonLayout->addWidget(buttonMoveDown);
    buttonLayout->addWidget(buttonDiscard);
    buttonLayout->addWidget(buttonApply);
    buttonLayout->addStretch(1);


    layout->addWidget(listbox);
    layout->addLayout(treeviewLayout);

    window->setLayout(layout);

    QObject::connect(buttonAdd, &QPushButton::clicked, editorModel, &SongListEditorModel::add);
    QObject::connect(buttonApply, &QPushButton::clicked, [doc, editorModel]() {
        editorModel->apply(*doc);
    });
    QObject::connect(buttonDiscard, &QPushButton::clicked, editorModel, &SongListEditorModel::reset);
    QObject::connect(buttonDuplicate, &QPushButton::clicked, [treeview, editorModel]() {
        auto const curr = treeview->currentIndex();
        if (curr.isValid()) {
            editorModel->duplicate(curr.row());
        }
    });

    QObject::connect(buttonMoveUp, &QPushButton::clicked, [treeview, editorModel]() {
        auto const curr = treeview->currentIndex();
        if (curr.isValid()) {
            editorModel->moveUp(curr.row());
        }
    });

    QObject::connect(buttonMoveDown, &QPushButton::clicked, [treeview, editorModel]() {
        auto const curr = treeview->currentIndex();
        if (curr.isValid()) {
            editorModel->moveDown(curr.row());
        }
    });

    window->show();


    auto const exitcode = app.exec();

    delete window;
    B::deinit();
    return exitcode;
}


