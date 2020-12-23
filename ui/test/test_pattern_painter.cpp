#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>



int main(int argc, char *argv[]) {

    QApplication app(argc, argv);


    QWidget win;


    QVBoxLayout layout;
    QLabel label;

    layout.addWidget(&label);
    win.setLayout(&layout);


    win.show();


    return app.exec();

}
