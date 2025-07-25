
#include "widgets/PianoWidget.hxx"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QWidget>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QWidget win;
    QVBoxLayout layout;
    PianoWidget piano;
    QLabel noteLabel("Note: ");

    piano.connect(&piano, &PianoWidget::keyChange, [&noteLabel](int note) {
        QString text("Note: %1");
        noteLabel.setText(text.arg(note));
    });

    layout.addStretch();
    layout.addWidget(&noteLabel);
    layout.addWidget(&piano, 1);
    win.setLayout(&layout);
    win.show();

    return app.exec();
}