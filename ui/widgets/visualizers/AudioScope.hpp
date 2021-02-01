
#pragma once


#include <QFrame>
#include <QPixmap>

#include <array>
#include <cstdint>
#include <cstddef>


class AudioScope : public QFrame {

    Q_OBJECT


public:

    explicit AudioScope(QWidget *parent = nullptr);

    //
    // Render the curve from the given sample data. Call update() afterwards for
    // changes to be shown.
    //
    void render(int16_t samples[], size_t nsamples, size_t skip = 2);

public slots:

    //
    // Clear the scope.
    //
    void clear();

    //
    // Sets the curve duration, by number of samples. Ie, setting this to 44100 will
    // show one second of audio for 44100 Hz streams.
    //
    void setDuration(size_t samples);

protected:

    void paintEvent(QPaintEvent *evt) override;

private:

    static constexpr int WAVE_WIDTH = 160;
    static constexpr int WAVE_HEIGHT = 64;
    static constexpr int WAVE_AXIS = WAVE_HEIGHT / 2 - 1;


    // Size of the curve, in samples.
    size_t mDuration;

    // the size of a "bin" or the number of audio samples that make a single
    // pixel on the graph, units are in samples per pixel
    size_t mBinSize;

    // previous bin amplitudes from the last call to addSamples
    float mPreviousBin;

    //
    // Cached drawing of the waveform (so we don't have to do it every paint event)
    //
    QPixmap mWaveform;


};
