
#include "core/Document.hxx"
#include "forms/SongListEditor.hxx"
#include "model/SongListModel.hxx"
#include "utils/connectutils.hxx"

#include <QApplication>
#include <QListView>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
    B::NimMain();
    B::init();

    QApplication app(argc, argv);

    auto window = new QWidget;

    auto doc = new Document(window);

    auto listModel = new SongListModel(doc, window);

    auto windowLayout = new QVBoxLayout;
    auto showEditorBtn = new QPushButton("Song List Editor");
    auto editor = new SongListEditor(listModel, window);
    windowLayout->addWidget(showEditorBtn);
    window->setLayout(windowLayout);
    QObject::lazyconnect(showEditorBtn, clicked, editor, open);

    window->show();

    auto const exitcode = app.exec();

    delete window;
    B::deinit();
    return exitcode;
}
