
#pragma once

#include "core/model/graph/SequenceModel.hpp"
#include "widgets/editors/BaseEditor.hpp"
#include "widgets/GraphEdit.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QTabBar>
#include <QVBoxLayout>

class InstrumentEditor : public BaseEditor {

    Q_OBJECT

public:
    explicit InstrumentEditor(PianoInput const& input, QWidget *parent = nullptr);


protected:

    virtual void setCurrentItem(int index) override;

    virtual BaseTableModel* getModel(ModuleDocument &doc) override;

private:
    void setSequenceInModel(int index);

    SequenceModel mSequenceModel;

    QVBoxLayout mLayout;
        QVBoxLayout mTabLayout;
            QTabBar mSequenceTabs;
            QWidget mTabFrame;
                QVBoxLayout mSequenceLayout;
                    GraphEdit mGraph;
                    QHBoxLayout mSizeLayout;
                        QLabel mSizeLabel;
                        QSpinBox mSizeSpin;

    trackerboy::Instrument *mInstrument;

};
