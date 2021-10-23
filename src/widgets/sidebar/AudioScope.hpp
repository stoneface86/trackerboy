
#pragma once


#include "core/audio/VisualizerBuffer.hpp"
#include "core/Guarded.hpp"
#include "core/Palette.hpp"

#include <QFrame>
#include <QMutex>
#include <QPixmap>

#include <array>
#include <cstdint>
#include <cstddef>


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
