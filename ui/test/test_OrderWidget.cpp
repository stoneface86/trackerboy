
#include "core/model/ModuleDocument.hpp"
#include "core/model/OrderModel.hpp"
#include "widgets/module/OrderEditor.hpp""


#include <QApplication>
#include <QUndoView>
#include <QPushButton>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    Q_INIT_RESOURCE(icons);

    ModuleDocument doc;
    doc.orderModel().setRowColor(Qt::gray);

    QWidget win;
    QVBoxLayout layout;

    OrderEditor orderWidget;
    QUndoView undoView;
    QPushButton button(QStringLiteral("set document"));
    QObject::connect(&button, &QPushButton::clicked, [&orderWidget, &doc, &button]() {
        orderWidget.setDocument(&doc);
        button.setEnabled(false);
    });
    undoView.setStack(&doc.undoStack());

    layout.addWidget(&orderWidget, 1);
    layout.addWidget(&undoView, 1);
    layout.addWidget(&button);
    win.setLayout(&layout);
    win.show();


    return app.exec();

}
