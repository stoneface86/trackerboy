
#pragma once


#include "audio/VisualizerBuffer.hpp"
#include "config/data/Palette.hpp"
#include "utils/Guarded.hpp"

#include <QFrame>


class AudioScope : public QFrame {

    Q_OBJECT


public:

    explicit AudioScope(QWidget *parent = nullptr);


    void setBuffer(Guarded<VisualizerBuffer>* buffer);

    void setColors(Palette const& pal);

protected:

    void paintEvent(QPaintEvent *evt) override;

private:
    Q_DISABLE_COPY(AudioScope)

    void drawSilence();

    void sample(Locked<VisualizerBuffer> &handle, float index, float ratio, float &outLeft, float &outRight);

    static constexpr int WAVE_WIDTH = 160;
    static constexpr int WAVE_HEIGHT = 64;
    static constexpr int WAVE_AXIS = WAVE_HEIGHT / 2 - 1;

    static constexpr int WAVE_LEFT_AXIS = (WAVE_HEIGHT / 2) + 1;
    static constexpr int WAVE_RIGHT_AXIS = (WAVE_HEIGHT / 2) + WAVE_HEIGHT + 1;

    Guarded<VisualizerBuffer> *mBuffer;

    QColor mLineColor;


};
