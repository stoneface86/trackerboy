
#pragma once

#include "widgets/editors/BaseEditor.hpp"


class InstrumentEditor : public BaseEditor {

    Q_OBJECT

public:
    explicit InstrumentEditor(PianoInput const& input, QWidget *parent = nullptr);


protected:

    virtual void setCurrentItem(int index) override;

    virtual BaseTableModel* getModel(ModuleDocument &doc) override;

};
