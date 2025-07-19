
#include <QBoxLayout>
#include <QToolBar>

//
// QBoxLayout subclass that automatically changes direction when an associated
// QToolBar changes its orientation.
//
class ToolbarLayout : public QBoxLayout {

public:
    using QBoxLayout::QBoxLayout;

    void setToolBar(QToolBar *bar);

private:
    void changeDirection(Qt::Orientation o);
};