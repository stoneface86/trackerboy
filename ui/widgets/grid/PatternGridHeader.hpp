
#pragma once

#include "core/ColorTable.hpp"
#include "core/Document.hpp"

#include <QBitmap>
#include <QPixmap>
#include <QWidget>

class PatternGridHeader : public QWidget {

    Q_OBJECT

public:

    PatternGridHeader(Document &document, QWidget *parent = nullptr);

    void setColors(ColorTable const& colorTable);

    void setWidths(int rownoWidth, int trackWidth);

    void setOffset(int offset);

protected:

    void paintEvent(QPaintEvent *evt) override;

    void leaveEvent(QEvent *evt) override;

    void mouseDoubleClickEvent(QMouseEvent *evt) override;

    void mouseMoveEvent(QMouseEvent *evt) override;

    void mousePressEvent(QMouseEvent *evt) override;


private:
    Q_DISABLE_COPY(PatternGridHeader)

    static constexpr int HEIGHT = 32;
    static constexpr int FONT_WIDTH = 7;
    static constexpr int FONT_HEIGHT = 11;

    static constexpr int HOVER_NONE = -1;

    void setOutputFlags(Document::OutputFlags flags);

    void setTrackHover(int hover);

    Document &mDocument;

    int mOffset;
    int mRownoWidth;
    int mTrackWidth;

    // 1bpp font 7x11, used for the Header
    QBitmap mHeaderFont;

    // header stuff
    int mTrackHover;

    Document::OutputFlags mTrackFlags;

    QColor mColorForeground;
    QColor mColorHover;
    QColor mColorDisabled;
    QColor mColorLine;

};
