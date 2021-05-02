
#include "widgets/PianoWidget.hpp"


#include <QApplication>
#include <QGridLayout>
#include <QMainWindow>
#include <QDockWidget>
#include <QScrollArea>


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    Q_INIT_RESOURCE(images);

    //QMainWindow win;

    //QDockWidget dock;

    QWidget container;
    QGridLayout layout;

    PianoInput input;
    PianoWidget piano(input);

    layout.addWidget(&piano, 0, 0);
    layout.setMargin(0);
    container.setLayout(&layout);
    container.show();
    

    //dock.setWidget(&piano);
    //win.addDockWidget(Qt::LeftDockWidgetArea, &dock);
    //dock.setFloating(true);

    //win.show();


    return app.exec();

}
