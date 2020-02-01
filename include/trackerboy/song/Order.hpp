
#pragma once

#include <cstdint>
#include <fstream>
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

    //
    // Returns the index the order will return to on loop. If no loop was set,
    // zero is returned.
    //
    uint8_t loopIndex();

    //
    // Returns true if the order has a loop index set, false otherwise
    //
    bool loops();

    void setLoop(uint8_t index);

    void removeLoop();

private:

    std::vector<uint8_t> mIndexVec;
    bool mLoops;
    uint8_t mLoopIndex;
};

}
