
#pragma once

#include <QBitmap>
#include <QPixmap>
#include <QWidget>

namespace grid {

class PatternGridHeader : public QWidget {

    Q_OBJECT

public:

    explicit PatternGridHeader(QWidget *parent = nullptr);
    ~PatternGridHeader() = default;

protected:

    void paintEvent(QPaintEvent *evt) override;

private:

    QBitmap mFontBitmap;
    QPixmap mDisplay;

    QColor mColorBg;
    QColor mColorFg;
    QColor mColorDisabled;
    QColor mColorHighlight;

    int mMouseOverTrack;

};

}
