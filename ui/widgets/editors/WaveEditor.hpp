
#pragma once

#include "core/model/graph/WaveModel.hpp"
#include "widgets/editors/BaseEditor.hpp"
#include "widgets/GraphEdit.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


class WaveEditor : public BaseEditor {

    Q_OBJECT

public:
    explicit WaveEditor(PianoInput const& input, QWidget *parent = nullptr);


protected:

    virtual void setCurrentItem(int index) override;

    virtual BaseTableModel* getModel(ModuleDocument &doc) override;

private:

    void setWaveFromPreset(int preset);
    
    WaveModel mWaveModel;

    QVBoxLayout mLayout;
        GraphEdit mGraph;
        QHBoxLayout mPresetLayout;
            QPushButton mSquare50Button;
            QPushButton mSquare25Button;
            QPushButton mTriangleButton;
            QPushButton mSawtoothButton;
            QPushButton mSineButton;
        QHBoxLayout mWaveramLayout;
            QLineEdit mWaveramEdit;
            QPushButton mClearButton;


    bool mWaveEditedFromText;

};
