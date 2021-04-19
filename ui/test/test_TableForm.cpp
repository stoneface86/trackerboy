
#include <QApplication>
#include "widgets/docks/TableForm.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/model/InstrumentListModel.hpp"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    Spinlock lock;
    ModuleDocument doc(lock);

    Q_INIT_RESOURCE(icons);

    InstrumentListModel model(doc);
    TableForm form(model, "wave");

    form.show();


    return app.exec();

}
