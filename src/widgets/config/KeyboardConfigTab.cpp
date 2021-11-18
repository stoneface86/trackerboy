
#include "widgets/config/KeyboardConfigTab.hpp"

#include "core/input/PianoInput.hpp"
#include "core/misc/NoteStrings.hpp"
#include "utils/connectutils.hpp"

#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <QtDebug>

#include <algorithm>


#define TU KeyboardConfigTabTU
namespace TU {


}

class BindingEdit : public QFrame {

    Q_OBJECT

    static constexpr int GRID_COLUMNS = 13;
    static constexpr int GRID_ROWS = 6;
    static constexpr int GRID_PADDING = 2;

    // specials (just note cut) is located at row 4, column 11
    // if more are added we will probably arrange them in a 2 column grid
    static constexpr int SPECIAL_ROW_START = 4;


public:

    explicit BindingEdit(QWidget *parent = nullptr) :
        QFrame(parent),
        mCursorRow(0),
        mCursorColumn(0)
    {
        setFocusPolicy(Qt::StrongFocus);
        setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
        setLineWidth(1);

        auto pal = palette();
        auto const bgrole = backgroundRole();
        auto const fgrole = foregroundRole();
        pal.setColor(QPalette::Active, fgrole, Qt::black);
        pal.setColor(QPalette::Disabled, fgrole, Qt::gray);
        pal.setColor(QPalette::Active, bgrole, Qt::white);
        pal.setColor(QPalette::Disabled, bgrole, Qt::white);
        setPalette(pal);


        determineMinHeight();
    }

    PianoInput::Bindings const& bindings() const {
        return mBindings;
    }

    void setBindings(PianoInput::Bindings const& bindings) {
        mBindings = bindings;
        update();
    }

signals:
    void bindingsChanged();

protected:

    virtual void changeEvent(QEvent *evt) override {
        if (evt->type() == QEvent::FontChange) {
            determineMinHeight();
        }
    }

    virtual void keyPressEvent(QKeyEvent *evt) override {
        auto key = evt->key();
        switch (key) {
            case Qt::Key_Left:
                moveCursorColumn(-1);
                return;
            case Qt::Key_Right:
                moveCursorColumn(1);
                return;
            case Qt::Key_Up:
                moveCursorRow(-1);
                return;
            case Qt::Key_Down:
                moveCursorRow(1);
                return;
            case Qt::Key_Delete:
                if (evt->modifiers() & Qt::ShiftModifier) {
                    key = 0;
                    break;
                }
                [[fallthrough]];
            case Qt::Key_Shift:
            case Qt::Key_Control:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
            case Qt::Key_Alt:
            case Qt::Key_AltGr:
            case Qt::Key_Meta:
                evt->ignore();
                return;
        }

        if (evt->isAutoRepeat()) {
            return;
        }

        setKey(key);

    }

    virtual void mousePressEvent(QMouseEvent *evt) override {
        if (evt->button() == Qt::LeftButton) {
            int row = evt->y() / mRowHeight;
            int col = evt->x() / mColumnWidth;

            if ((row >= 1 && row <= 4 && col >= 1 && col <= 12) || (row == 5 && col == 12)) {
                mCursorRow = row - 1;
                mCursorColumn = col - 1;
                update();
            }
        }
    }

    virtual void paintEvent(QPaintEvent *evt) override {

        QPainter painter(this);

        QRect cell(GRID_PADDING, GRID_PADDING, mColumnWidth - GRID_PADDING * 2, mRowHeight - GRID_PADDING * 2);

        auto mergeCells = [this](int row, int col, int rows, int cols) {
            return QRect(
                col * mColumnWidth + GRID_PADDING,
                row * mRowHeight + GRID_PADDING,
                cols * mColumnWidth - GRID_PADDING * 2,
                rows * mRowHeight - GRID_PADDING * 2
            );
        };

        auto invertRect = [&painter](QRect const& rect) {
            auto mode = painter.compositionMode();
            painter.setCompositionMode(QPainter::RasterOp_NotDestination);
            painter.fillRect(rect, QColor(0, 0, 0, 0));
            painter.setCompositionMode(mode);
        };

        auto drawKeybind = [&painter, &cell](Qt::Key key) {
            painter.drawText(cell, Qt::AlignCenter, QKeySequence(key).toString());
        };

        // lines
        painter.drawLine(0, mRowHeight, width(), mRowHeight);

        // note header
        for (int i = 0; i < 12; ++i) {
            cell.translate(mColumnWidth, 0);

            auto notestr = NoteStrings::Sharps[i];
            painter.drawText(cell, Qt::AlignCenter, notestr);

            if (isEnabled() && notestr[1] == '#') {
                // invert the cell so it has a black background
                invertRect(cell);
            }
        }

        // octaves
        cell.moveTo(GRID_PADDING, mRowHeight + GRID_PADDING);
        for (auto octave : { 0, 1, 1, 2}) {
            painter.drawText(cell, Qt::AlignCenter, QStringLiteral("+%1").arg(octave));
            cell.translate(0, mRowHeight);
        }

        // note binds        
        cell.moveTo(mColumnWidth + GRID_PADDING, mRowHeight + GRID_PADDING);
        auto bindingsIter = mBindings.cbegin();
        for (auto r = 0; r < 4; ++r) {
            for (auto c = 0; c < 12; ++c) {
                drawKeybind(*bindingsIter++);
                cell.translate(mColumnWidth, 0);
            }
            cell.moveLeft(mColumnWidth + GRID_PADDING);
            cell.translate(0, mRowHeight);
        }


        // special binds
        cell.moveTo(mColumnWidth * 12 + GRID_PADDING, mRowHeight * 5 + GRID_PADDING);
        painter.drawText(mergeCells(5, 1, 1, 11), Qt::AlignVCenter | Qt::AlignRight, tr("Note cut: "));
        drawKeybind(mBindings[PianoInput::BindingNoteCut]);

        // cursor
        cell.moveTo(
            mColumnWidth * (mCursorColumn + 1) + GRID_PADDING,
            mRowHeight * (mCursorRow + 1) + GRID_PADDING
        );

        if (hasFocus()) {
            invertRect(cell);
        } else {
            painter.drawRect(cell);
        }



        QFrame::paintEvent(evt);
    }

    virtual void resizeEvent(QResizeEvent *evt) override {
        auto size = evt->size();

        mColumnWidth = size.width() / GRID_COLUMNS;
        mRowHeight = size.height() / GRID_ROWS;
    }

private:

    Qt::Key& cursorKey() {
        switch (mCursorRow) {
            case 0:
            case 1:
            case 2:
            case 3:
                return mBindings[mCursorRow * 12 + mCursorColumn];
            default:
                // specials (just note cut)
                return mBindings[PianoInput::BindingNoteCut];
        }
    }

    void determineMinHeight() {
        auto metrics = fontMetrics();
        setMinimumHeight((metrics.height() + GRID_PADDING * 2) * GRID_ROWS);
    }

    void moveCursorColumn(int amount) {
        if (mCursorRow < 4) {
            mCursorColumn = std::clamp(mCursorColumn + amount, 0, 11);
            update();
        }
    }

    void moveCursorRow(int amount) {
        mCursorRow = std::clamp(mCursorRow + amount, 0, 4);
        if (mCursorRow == 4) {
            mCursorColumn = 11;
        }

        update();
    }

    void setKey(int key) {
        auto &dest = cursorKey();
        if (dest != key) {

            if (key) {
                // check if this key is already bound
                auto existing = std::find(mBindings.begin(), mBindings.end(), (Qt::Key)key);
                if (existing != mBindings.end()) {
                    // key is bound, overwrite with the cursor's old value
                    *existing = dest;
                }
            }
            dest = (Qt::Key)key;

            emit bindingsChanged();
            update();
        }
    }

    PianoInput::Bindings mBindings;

    int mColumnWidth;
    int mRowHeight;

    int mCursorRow;
    int mCursorColumn;

};


KeyboardConfigTab::KeyboardConfigTab(PianoInput const& input, QWidget *parent) :
    ConfigTab(parent)
{

    auto layout = new QVBoxLayout;

    auto keyboardLayoutLayout = new QHBoxLayout;
    keyboardLayoutLayout->addWidget(new QLabel(tr("Layout:")));

    mLayoutCombo = new QComboBox;
    keyboardLayoutLayout->addWidget(mLayoutCombo, 1);


    mCustomGroup = new QGroupBox(tr("Custom layout"));
    auto customLayout = new QVBoxLayout;
    customLayout->addWidget(new QLabel(tr("Assign keys to each note. Press Shift+Del to unbind.")));
    mBindingEdit = new BindingEdit;
    customLayout->addWidget(mBindingEdit);
    mCustomGroup->setLayout(customLayout);


    layout->addLayout(keyboardLayoutLayout);
    layout->addWidget(mCustomGroup);
    layout->addStretch(1);
    setLayout(layout);



    // ignore LayoutSystem for now
    for (int kblayout = PianoInput::LayoutQwerty; kblayout < PianoInput::LayoutCustom; ++kblayout) {
        mLayoutCombo->addItem(PianoInput::layoutName((PianoInput::KeyboardLayout)kblayout));
    }

    mCustomGroup->setCheckable(true);

    switch (auto layout = input.layout(); layout) {
        case PianoInput::LayoutQwerty:
        case PianoInput::LayoutQwertz:
        case PianoInput::LayoutAzerty:
            mLayoutCombo->setCurrentIndex(layout);
            break;
        default:
            break;

    }

    mCustomGroup->setChecked(input.layout() == PianoInput::LayoutCustom);
    mBindingEdit->setBindings(input.bindings());

    lazyconnect(mCustomGroup, toggled, mLayoutCombo, setDisabled);
    lazyconnect(mCustomGroup, toggled, this, setDirty<Config::CategoryKeyboard>);
    lazyconnect(mBindingEdit, bindingsChanged, this, setDirty<Config::CategoryKeyboard>);

    connect(mLayoutCombo, qOverload<int>(&QComboBox::activated), this, &KeyboardConfigTab::setDirty<Config::CategoryKeyboard>);    

}

void KeyboardConfigTab::apply(PianoInput &input) {
    if (mCustomGroup->isChecked()) {
        input.setBindings(mBindingEdit->bindings());
    } else {
        switch (mLayoutCombo->currentIndex()) {
            case 0:
                input.setLayout(PianoInput::LayoutQwerty);
                break;
            case 1:
                input.setLayout(PianoInput::LayoutQwertz);
                break;
            case 2:
                input.setLayout(PianoInput::LayoutAzerty);
                break;
        }
    }

    clean();
}

#include "KeyboardConfigTab.moc"

#undef TU
