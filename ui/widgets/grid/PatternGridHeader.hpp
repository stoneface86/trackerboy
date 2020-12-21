
#pragma once

#include "widgets/grid/PatternMetrics.hpp"
#include "ColorTable.hpp"

#include <QBitmap>
#include <QPixmap>
#include <QWidget>

class PatternGridHeader : public QWidget {

    Q_OBJECT

public:

    PatternGridHeader(QWidget *parent = nullptr);

    void setColors(ColorTable const& colorTable);

    void setMetrics(PatternMetrics const& metrics);

protected:

    void paintEvent(QPaintEvent *evt) override;

    void leaveEvent(QEvent *evt) override;

    void mouseDoubleClickEvent(QMouseEvent *evt) override;

    void mouseMoveEvent(QMouseEvent *evt) override;

    void mousePressEvent(QMouseEvent *evt) override;


private:

    static constexpr int HEIGHT = 32;
    static constexpr int FONT_WIDTH = 7;
    static constexpr int FONT_HEIGHT = 11;

    // QPalette is used for color storage, the roles were chosen arbitrarily (except for BACKGROUND)
    static constexpr auto COLOR_FOREGROUND = QPalette::WindowText;
    static constexpr auto COLOR_BACKGROUND = QPalette::Window;
    static constexpr auto COLOR_HOVER = QPalette::Button;
    static constexpr auto COLOR_DISABLED = QPalette::Light;
    static constexpr auto COLOR_LINE = QPalette::Midlight;

    static constexpr int HOVER_NONE = -1;

    void setTrackHover(int hover);


    PatternMetrics mMetrics;

    // 1bpp font 7x11, used for the Header
    QBitmap mHeaderFont;

    // header stuff
    int mTrackHover;
    int mTrackFlags;

};
