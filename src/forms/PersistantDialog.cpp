
#include "forms/PersistantDialog.hpp"

#include "utils/connectutils.hpp"

#include <QShowEvent>

PersistantDialog::PersistantDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    mShown(false)
{
}

void PersistantDialog::showEvent(QShowEvent *evt) {
    if (mShown) {
        // accepting the event prevents the dialog from being centered within the
        // parent widget (MainWindow).

        // this way the dialog "remembers" its geometry
        evt->accept();
    } else {
        // dialog hasn't been shown yet, center it within the parent widget
        mShown = true;
        QDialog::showEvent(evt);
    }

}
