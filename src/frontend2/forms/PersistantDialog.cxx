
#include "forms/PersistantDialog.hxx"

#include <QShowEvent>

PersistantDialog::PersistantDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , _shown(false) {}

void PersistantDialog::showEvent(QShowEvent *evt) {
    if (_shown) {
        // accepting the event prevents the dialog from being centered within
        // the parent widget (MainWindow).

        // this way the dialog "remembers" its geometry
        evt->accept();
    } else {
        // dialog hasn't been shown yet, center it within the parent widget
        _shown = true;
        QDialog::showEvent(evt);
    }
}
