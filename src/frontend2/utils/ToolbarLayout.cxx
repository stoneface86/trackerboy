
#include "utils/ToolbarLayout.hxx"
#include "utils/connectutils.hxx"

void ToolbarLayout::setToolBar(QToolBar *bar) {
    lazyconnect(bar, orientationChanged, this, changeDirection);
    changeDirection(bar->orientation());
}

void ToolbarLayout::changeDirection(Qt::Orientation o) {

    auto const dir = direction();
    if (o == Qt::Horizontal) {
        switch (dir) {
        case QBoxLayout::TopToBottom:
            setDirection(QBoxLayout::LeftToRight);
            break;
        case QBoxLayout::BottomToTop:
            setDirection(QBoxLayout::RightToLeft);
            break;
        default:
            break;
        }
    } else { // Qt::Vertical
        switch (dir) {
        case QBoxLayout::LeftToRight:
            setDirection(QBoxLayout::TopToBottom);
            break;
        case QBoxLayout::RightToLeft:
            setDirection(QBoxLayout::BottomToTop);
            break;
        default:
            break;
        }
    }
}
