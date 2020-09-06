
#pragma once

#include <fstream>

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/PatternMaster.hpp"
#include "trackerboy/data/Order.hpp"
#include "trackerboy/fileformat.hpp"
#include "trackerboy/Speed.hpp"
#include "trackerboy/gbs.hpp"


namespace trackerboy {


class Song : public DataItem {

public:

    enum class Mode : uint8_t {
        speed,              // use the speed setting
        speedFromTempo      // determine speed from tempo setting
    };

    static constexpr uint8_t DEFAULT_RPB = 4;
    static constexpr uint16_t DEFAULT_TEMPO = 150;
    static constexpr Mode DEFAULT_MODE = Mode::speedFromTempo;
    // Tempo = 150, RPB = 4  => 6.0 frames per row
    static constexpr Speed DEFAULT_SPEED = 0x30;

    Song();
    ~Song();

    uint8_t rowsPerBeat();

    Mode mode();

    uint16_t tempo();

    Speed speed();

    std::vector<Order>& orders();

    PatternMaster& patterns();

    Pattern getPattern(uint8_t orderNo);

    void setRowsPerBeat(uint8_t rowsPerBeat);

    void setTempo(uint16_t tempo);

    void setMode(Mode mode);

    void setSpeed(Speed speed);

    // apply the current mode to tempo/speed
    void apply(float framerate = Gbs::FRAMERATE_GB);

protected:

    virtual bool deserializeData(std::istream &stream) noexcept override;
    
    virtual bool serializeData(std::ostream &stream) noexcept override;


private:

    //void calcSpeed();

    PatternMaster mMaster;
    std::vector<Order> mOrder;

    uint8_t mRowsPerBeat;
    uint16_t mTempo;
    Mode mMode;

    // Speed - fixed point Q5.3
    // frame timing for each row
    // 4.0:   4, 4, 4, 4, ...
    // 4.125: 4, 4, 4, 4, 4, 4, 4, 5, ...
    // 4.25:  4, 4, 4, 5, 4, 4, 4, 5, ...
    // 4.375: 4, 4, 5, 4, 4, 5, 4, 4, 5, ...
    // 4.5:   4, 5, 4, 5, ...
    // 4.675: 4, 5, 5, 4, 5, 5, 4, 5, 5, 4, ...
    // 4.75:  4, 5, 5, 5, 4, 5, 5, 5, 4, ...
    // 4.875: 4, 5, 5, 5, 5, 5, 5, 4, ...

    Speed mSpeed; // frames per row

};


}
