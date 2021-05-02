
#include <QApplication>
#include "widgets/editors/BaseEditor.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/model/InstrumentListModel.hpp"

class TestEditor : public BaseEditor {
    Q_OBJECT

public:
    explicit TestEditor(PianoInput const& input, QWidget *parent = nullptr) :
        BaseEditor(input, "instrument", parent),
        mLayout(),
        mLabel()
    {
        mLayout.addWidget(&mLabel, 0, 0);
        auto &widget = editorWidget();
        widget.setLayout(&mLayout);
    }

protected:
    virtual void setCurrentItem(int index) override {
        if (index == -1) {
            mLabel.setText(tr("No item!"));
        } else {
            mLabel.setText(tr("Current item: %1").arg(index));
        }
    }

    virtual BaseTableModel* getModel(ModuleDocument &doc) override {
        return &doc.instrumentModel();
    }

private:
    QGridLayout mLayout;
    QLabel mLabel;

};


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(images);

    ModuleDocument doc;

    QWidget win;
    QVBoxLayout layout;
    
    PianoInput input;

    TestEditor editor(input);
    editor.setDocument(&doc);
    editor.show();


    return app.exec();

}

#include "test_BaseEditor.moc"
