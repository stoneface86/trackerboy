
#include "widgets/editors/WaveEditor.hpp"

WaveEditor::WaveEditor(PianoInput const& input, QWidget *parent) :
    BaseEditor(input, tr("waveform"), parent)
{

}

void WaveEditor::setCurrentItem(int index) {

}

BaseTableModel* WaveEditor::getModel(ModuleDocument &doc) {
    return &doc.waveModel();
}
