
#pragma once

#include <QFrame>
#include <QPainterPath>

//
// Simple widget for showing how quality settings affect the synthesizer.
// A low quality setting results in a linear interpolated transition whereas
// a high quality setting results in a sinc interpolated transition.
//
class SoundQualityPreview : public QFrame {

    Q_OBJECT

public:

    SoundQualityPreview(QWidget *parent = nullptr);

    void setHighQuality(bool quality);

protected:

    void paintEvent(QPaintEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;

private:

    void updatePath();

    Q_DISABLE_COPY(SoundQualityPreview)

    bool mQuality;
    QPainterPath mPath;

};
