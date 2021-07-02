//
// This program tests the AudioStream class. Random tones are played out to the
// configured device. Changing the config while the stream is playing is also
// possible: a gap in playback will occur, but playback should resume without error
// 
// Latency settings can also be tested here
//

#include "core/audio/AudioProber.hpp"
#include "core/audio/AudioStream.hpp"
#include "core/samplerates.hpp"
#include "widgets/config/SoundConfigTab.hpp"

#include "trackerboy/Synth.hpp"

#include <QApplication>
#include <QWidget>


class TestWindow : public QWidget {

    Q_OBJECT


    static constexpr int FRAMES_PER_TONE = 10;

public:
    TestWindow() :
        mConfig(),
        mLayout(),
        mTabs(),
        mSoundTab(mConfig),
        mControlGroup("Control"),
        mControlLayout(),
        mSetConfigButton("Set config"),
        mStartButton("Start"),
        mStatusGroup("Status"),
        mStatusLayout(),
        mUnderrunLabel("Underruns:"),
        mUnderruns(),
        mBuffersizeLabel("Buffer use:"),
        mBuffersize(),
        mStream(),
        mSynth(44100),
        mFrameCounter(0),
        mTimer(-1)
    {
        mTabs.addTab(&mSoundTab, "Sound");

        mControlLayout.addStretch();
        mControlLayout.addWidget(&mSetConfigButton);
        mControlLayout.addWidget(&mStartButton);
        mControlGroup.setLayout(&mControlLayout);
        
        mStatusLayout.addWidget(&mUnderrunLabel, 0, 0);
        mStatusLayout.addWidget(&mUnderruns, 0, 1);
        mStatusLayout.addWidget(&mBuffersizeLabel, 1, 0);
        mStatusLayout.addWidget(&mBuffersize, 1, 1);
        mStatusGroup.setLayout(&mStatusLayout);

        mGroupLayout.addWidget(&mControlGroup);
        mGroupLayout.addWidget(&mStatusGroup);
        mGroupLayout.addStretch();
        

        mLayout.addWidget(&mTabs);
        mLayout.addLayout(&mGroupLayout);
        setLayout(&mLayout);

        mStartButton.setEnabled(false);


        connect(&mSetConfigButton, &QPushButton::clicked, this,
            [this]() {
                mSoundTab.apply(mSoundConfig);

                bool resume = false;
                if (mTimer != -1) {
                    killTimer(mTimer);
                    resume = true;
                }
                mStream.setConfig(mSoundConfig);
                
                mSynth.setSamplingRate(SAMPLERATE_TABLE[mSoundConfig.samplerateIndex]);
                mSynth.setupBuffers();


                mStartButton.setEnabled(mStream.isEnabled());

                if (resume) {
                    if (mStream.isRunning()) {
                        mTimer = startTimer(mSoundConfig.period, Qt::PreciseTimer);
                    } else {
                        mStartButton.setText("Start");
                    }
                }

            });

        connect(&mStartButton, &QPushButton::clicked, this,
            [this]() {
                if (mStream.isEnabled()) {
                    if (mStream.isRunning()) {
                        stop();
                    } else {
                        start();
                    }
                }
            });


    }

protected:
    virtual void timerEvent(QTimerEvent *evt) override {
        if (evt->timerId() == mTimer) {
            mUnderruns.setText(QString::number(mStream.underruns()));
            auto bufferSize = mStream.bufferSize();
            auto bufferUsage = mStream.bufferUsage();
            mBuffersize.setText(tr("%1% (%2 / %3 samples)")
                .arg(bufferUsage * 100 / (int)bufferSize)
                .arg(bufferUsage)
                .arg(bufferSize)
            );

            // replenish buffer
            auto &apu = mSynth.apu();
            auto writer = mStream.writer();
            auto samplesToRender = writer.availableWrite();

            while (samplesToRender) {
                if (apu.availableSamples() == 0) {

                    if (mFrameCounter == 0) {
                        int terms = rand() & 0x11;
                        if (terms == 0) {
                            terms = 0x11;
                        }
                        apu.writeRegister(gbapu::Apu::REG_NR52, 0x80);
                        apu.writeRegister(gbapu::Apu::REG_NR50, 0x77);
                        apu.writeRegister(gbapu::Apu::REG_NR51, terms);
                        apu.writeRegister(gbapu::Apu::REG_NR10, 0x70 + (rand() & 0xF));
                        apu.writeRegister(gbapu::Apu::REG_NR11, 0x80);
                        apu.writeRegister(gbapu::Apu::REG_NR12, 0xF1);
                        int16_t freq = (rand() & 0x4FF) + 0x300;
                        apu.writeRegister(gbapu::Apu::REG_NR13, freq & 0xFF);
                        apu.writeRegister(gbapu::Apu::REG_NR14, (freq >> 8) | 0x80);
                        mFrameCounter = FRAMES_PER_TONE;
                    } else {
                        --mFrameCounter;
                    }

                    mSynth.run();

                }

                size_t toWrite = std::min(samplesToRender, apu.availableSamples());
                auto writePtr = writer.acquireWrite(toWrite);
                apu.readSamples(writePtr, toWrite);
                writer.commitWrite(writePtr, toWrite);

                samplesToRender -= toWrite;
            }

        }
    }

private:

    void start() {
        mStream.start();
        mStartButton.setEnabled(mStream.isEnabled());
        if (mStream.isRunning()) {
            mTimer = startTimer(mSoundConfig.period, Qt::PreciseTimer);
            mStartButton.setText("Stop");
        }
    }

    void stop() {
        mStream.stop();
        killTimer(mTimer);
        mTimer = -1;
        mStartButton.setText("Start");
        mStartButton.setEnabled(mStream.isEnabled());
    }


    Config mConfig;
    Config::Sound mSoundConfig;

    QHBoxLayout mLayout;
        QTabWidget mTabs;
            SoundConfigTab mSoundTab;
        QVBoxLayout mGroupLayout;
            QGroupBox mControlGroup;
                QHBoxLayout mControlLayout;
                    QPushButton mSetConfigButton;
                    QPushButton mStartButton;
            QGroupBox mStatusGroup;
                QGridLayout mStatusLayout;
                    QLabel mUnderrunLabel;
                    QLabel mUnderruns;
                    QLabel mBuffersizeLabel;
                    QLabel mBuffersize;

    AudioStream mStream;

    trackerboy::Synth mSynth;
    int mFrameCounter;

    int mTimer;

};




int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    TestWindow win;
    win.show();


    return app.exec();


}

#include "test_AudioStream.moc"


