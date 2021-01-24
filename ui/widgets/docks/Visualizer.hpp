
#pragma once

#include <QPixmap>
#include <QWidget>

#include <cstdint>

//
// Audio visualizer widget. Visualizes audio data in the form of an oscilloscope, one
// for each channel.
//
class Visualizer : public QWidget {

    Q_OBJECT

public:
    explicit Visualizer(QWidget *parent = nullptr);

    //
    // Adds samples to the curve. Changes will be visible on the next paint event.
    // data is a buffer of interleaved stereo PCM samples. The size of data must be
    // >= 2 * nsamples
    //
    void addSamples(int16_t sampleData[], size_t nsamples);

public slots:

    //
    // Clear the scope.
    //
    void clear();

    //
    // Setting for antialiasing when drawing the curve. Changes only apply to
    // new samples being added.
    //
    void setAntialiasing(bool antialias);

    //
    // Sets the curve duration, by number of samples. Ie, setting this to 44100 will
    // show one second of audio for 44100 Hz streams.
    //
    void setDuration(int samples);

protected:

    void paintEvent(QPaintEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;

private:
    // height, in pixels of a waveform
    static constexpr int WAVE_HEIGHT = 64;

    //
    // initialize the QPainter reference for drawing the waveform
    //
    void initPainter(QPainter &painter);


    // sample a bin, or reduce mBinSize samples from the buffer into a single sample
    // The average sample within the bin is returned.
    void reduce(int16_t *&samples, int8_t &left, int8_t &right, int binSize);

    void clearWaveform(QPixmap &waveform);

    void scrollWaveform(QPixmap &waveform, int amount, int timeIndex);

    // setting for whether or not to antialias when drawing lines
    bool mAntialias;

    // Size of the curve, in samples.
    int mDuration;

    // the size of a "bin" or the number of audio samples that make a single
    // pixel on the graph
    int mBinSize;

    // previous bin amplitudes from the last call to addSamples
    int8_t mPreviousBinLeft;
    int8_t mPreviousBinRight;


    //
    // Cached drawing of the waveform (so we don't have to do it every paint event)
    //
    QPixmap mWaveformLeft;
    QPixmap mWaveformRight;

    //
    // Y offset for centering when painting the widget
    //
    int mOffsetY;

};
