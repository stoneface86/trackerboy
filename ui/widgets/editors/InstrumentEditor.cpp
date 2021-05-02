
#include "widgets/editors/InstrumentEditor.hpp"


InstrumentEditor::InstrumentEditor(PianoInput const& input, QWidget *parent) :
    BaseEditor(input, tr("instrument"), parent)
{

}

void InstrumentEditor::setCurrentItem(int index) {

}

BaseTableModel* InstrumentEditor::getModel(ModuleDocument &doc) {
    return &doc.instrumentModel();
}
