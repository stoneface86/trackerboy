
#pragma once

#include "core/ColorTable.hpp"
#include "core/model/ModuleDocument.hpp"

#include <QBitmap>
#include <QPixmap>
#include <QWidget>

class PatternGridHeader : public QWidget {

    Q_OBJECT

public:

    PatternGridHeader(QWidget *parent = nullptr);

    void setColors(ColorTable const& colorTable);

    void setDocument(ModuleDocument *doc);

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

    void setOutputFlags(ModuleDocument::OutputFlags flags);

    void setTrackHover(int hover);

    ModuleDocument *mDocument;

    int mOffset;
    int mRownoWidth;
    int mTrackWidth;

    // 1bpp font 7x11, used for the Header
    QBitmap mHeaderFont;

    // header stuff
    int mTrackHover;

    ModuleDocument::OutputFlags mTrackFlags;

    QColor mColorForeground;
    QColor mColorHover;
    QColor mColorDisabled;
    QColor mColorLine;

};
