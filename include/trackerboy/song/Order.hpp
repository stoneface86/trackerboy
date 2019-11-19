
#pragma once

#include <cstdint>
#include <vector>


namespace trackerboy {


class Order {

public:

    class Sequencer {

    public:

        Sequencer() = default;
        Sequencer(Order &order);
        Sequencer(Order &order, size_t loopFor);

        bool hasNext();

        uint8_t next();
        
    private:
        static constexpr size_t INFINITE_LOOP = static_cast<size_t>(-1);

        void init(Order &order);

        uint8_t *mPtr;
        size_t mLoopCounter;
        uint8_t *mEnd;
        uint8_t *mLoop;
    };

    Order();

    std::vector<uint8_t>& indexVec();

    void setLoop(uint8_t index);

    void removeLoop();

private:

    std::vector<uint8_t> mIndexVec;
    bool mLoops;
    uint8_t mLoopIndex;
};

}