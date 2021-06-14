
#include "trackerboy/export/Player.hpp"

namespace trackerboy {

Player::LoopContext::LoopContext(unsigned loopAmount) :
    currentPattern(0u),
    visits(),
    loopAmount(loopAmount)
{
}

Player::DurationContext::DurationContext(unsigned framesToPlay) :
    frameCounter(0),
    framesToPlay(framesToPlay)
{
}


Player::Player(Engine &engine, unsigned loopCount) :
    Player(engine)
{
    if (loopCount == 0) {
        // if loopCount is 0 then we play the song 0 times
        // ...
        // done!
        mPlaying = false;
    } else if (mPlaying) {
        auto mod = mEngine.getModule();
        mContext.emplace<LoopContext>(loopCount);
        auto &ctx = std::get<LoopContext>(mContext);
        ctx.visits.resize(mod->song().order().size());
        ctx.visits[0] = 1; // visit the first pattern
    }

}

Player::Player(Engine &engine, std::chrono::seconds duration) :
    Player(engine)
{
    if (mPlaying) {
        // determine how many frames to play
        auto mod = mEngine.getModule();
        auto frames = mod->framerate() * duration.count();
        mContext.emplace<DurationContext>((unsigned)frames);
    }
}

Player::Player(Engine &engine) :
    mEngine(engine),
    mPlaying(false),
    mContext()
{
    auto mod = mEngine.getModule();
    if (mod) {
        mEngine.play(0);
        mPlaying = true;
    }
}

bool Player::isPlaying() const {
    return mPlaying;
}

void Player::step() {
    if (mPlaying) {
        Engine::Frame frame;
        mEngine.step(frame);

        if (frame.halted) {
            mPlaying = false;
        } else {
            // check if we have another step via our context variant
            std::visit([this, &frame](auto&& ctx) {
                using T = std::decay_t<decltype(ctx)>;

                if constexpr (std::is_same_v<T, LoopContext>) {
                    if (frame.order != ctx.currentPattern) {
                        if (ctx.visits[frame.order]++ == ctx.loopAmount) {
                            mPlaying = false;
                        }
                        ctx.currentPattern = frame.order;
                    }
                } else if constexpr (std::is_same_v<T, DurationContext>) {
                    if (++ctx.frameCounter >= ctx.framesToPlay) {
                        mPlaying = false;
                    }
                } else {
                    // T is monostate, should not happen
                    mPlaying = false; // stop playing, class was incorrectly constructed
                }
                

            }, mContext);

        }


    }
}


}
