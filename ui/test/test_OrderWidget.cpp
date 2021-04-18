
#include "core/model/ModuleDocument.hpp"
#include "core/model/OrderModel.hpp"
#include "widgets/docks/OrderWidget.hpp"


#include <QApplication>
#include <QUndoView>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    Spinlock lock;
    ModuleDocument doc(lock);
    OrderModel model(doc);
    model.setRowColor(Qt::gray);

    QWidget win;
    QVBoxLayout layout;

    OrderWidget orderWidget(model);
    QUndoView undoView;
    undoView.setStack(&doc.undoStack());

    layout.addWidget(&orderWidget);
    layout.addWidget(&undoView);
    win.setLayout(&layout);
    win.show();


    return app.exec();

}
