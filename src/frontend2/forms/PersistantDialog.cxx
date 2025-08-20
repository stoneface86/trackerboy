
#include "forms/PersistantDialog.hxx"

#include <QShowEvent>

PersistantDialog::PersistantDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , _geometry() {}

void PersistantDialog::hideEvent(QHideEvent *evt) {
    _geometry = geometry();
    QDialog::hideEvent(evt);
}

void PersistantDialog::showEvent(QShowEvent *evt) {
    QDialog::showEvent(evt);
    if (_geometry.isValid()) {
        setGeometry(_geometry);
    }
}
