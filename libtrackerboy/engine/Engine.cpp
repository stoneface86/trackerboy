
#include "trackerboy/engine/Engine.hpp"


namespace trackerboy {


Engine::Engine()
{
}

Engine::Context::Context(Song &song, uint8_t orderNo, uint8_t lastOrder) :
    song(song),
    orderCounter(orderNo),
    lastOrder(lastOrder),
    halted(false),
    command(PatternCommand::none),
    commandParam(0)
{
}

void Engine::reset() {
    // destroy the current context, so that stepping will result in a no-op
    mContext.reset();
}


void Engine::play(Song &song, uint8_t orderNo, uint8_t patternRow) {
    
    // destroy the previous context
    reset();

    auto &orders = song.orders();
    if (orderNo >= orders.size()) {
        throw std::invalid_argument("cannot play order, order does not exist");
    }
    if (patternRow >= song.patterns().rowSize()) {
        throw std::invalid_argument("cannot start at given row, exceeds pattern size");
    }

    mContext.emplace(song, orderNo, static_cast<uint8_t>(orders.size() - 1));

    // reset components
    mTimer.reset();

    //mSr1.reset();
    //mSr2.reset();
    //mSr3.reset();
    //mSr4.reset();

    // set the timer's period to the song's speed setting
    mTimer.setPeriod(song.speed());

    // begin at the given pattern
    mCursor.setPattern(song.getPattern(orderNo), patternRow);
    //mPr.setPattern(song.getPattern(orderNo));
    

}


bool Engine::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    (void)synth; (void)itable; (void)wtable;

    // make sure we have a context, otherwise just return false (no-op)
    if (mContext) {

        auto &ctx = mContext.value();
        if (ctx.halted) {
            return true;
        }

        if (mTimer.active()) {

            // change the current pattern if needed
            switch (ctx.command) {
                case PatternCommand::none:
                    break;
                case PatternCommand::next:
                    if (++ctx.orderCounter > ctx.lastOrder) {
                        // loop back to the first pattern
                        ctx.orderCounter = 0;
                    }
                    mCursor.setPattern(ctx.song.getPattern(ctx.orderCounter),
                                       ctx.commandParam);
                    ctx.command = PatternCommand::none;
                    break;
                case PatternCommand::jump:
                    mCursor.setPattern(ctx.song.getPattern(ctx.commandParam));
                    ctx.command = PatternCommand::none;
                    break;
            }

            // search the row for pattern effects, filter out track effects

            bool halt = setRows();

            if (halt) {
                ctx.halted = true;
                return true;
            }


            //TrackRow &row1

            // timer is active which means our track runtimes get the current row from
            // the pattern


        }

        if (mTimer.step()) {
            // timer overflowed, advance pattern iterator to the next row
            // this also means that this step is the last one for the current row
            if (mCursor.next()) {
                // end of pattern
                if (ctx.command == PatternCommand::none) {
                    // load the next pattern if no command was set
                    ctx.command = PatternCommand::next;
                    ctx.commandParam = 0;
                }

            }
        }

        // step track or sfx runtimes
        // channel locked -> step sfx runtime
        // channel unlocked -> step track runtime

        return false;
    } else {
        // no context, nothing to play: caller should halt
        return true;
    }

}


template <int channel>
bool Engine::setRows() {
    if constexpr (channel <= static_cast<int>(ChType::ch4)) {
        //constexpr ChType ch = static_cast<ChType>(channel);

        //TrackRow &row = mCursor.get<ch>();

        //tr.setRow(row);

        return setRows<channel + 1>();
    } else {
        return false;
    }
}



}
