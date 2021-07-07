
#include "trackerboy/export/Player.hpp"

namespace trackerboy {

Player::LoopContext::LoopContext(int loopAmount) :
    currentPattern(0),
    visits(),
    loopAmount(loopAmount)
{
}

Player::DurationContext::DurationContext(int framesToPlay) :
    frameCounter(0),
    framesToPlay(framesToPlay)
{
}


Player::Player(Engine &engine) :
    mEngine(engine),
    mPlaying(false),
    mContext()
{
}

void Player::start(Duration duration) {
    bool init = false;
    if (std::holds_alternative<int>(duration)) {
        auto loopCount = std::get<int>(duration);
        if (loopCount == 0) {
            // if loopCount is 0 then we play the song 0 times
            // ...
            // done!
            mPlaying = false;
            mContext = {};
        } else {
            auto mod = mEngine.getModule();
            if (mod) {
                mContext.emplace<LoopContext>(loopCount);
                auto &ctx = std::get<LoopContext>(mContext);
                ctx.visits.resize(mod->song().order().size());
                ctx.visits[0] = 1; // visit the first pattern
                init = true;
            }
        }
    } else {
        // determine how many frames to play
        auto secs = std::get<std::chrono::seconds>(duration);
        if (secs.count() == 0) {
            mPlaying = false;
            mContext = {};
        } else {
            auto mod = mEngine.getModule();
            if (mod) {
                auto frames = mod->framerate() * secs.count();
                mContext.emplace<DurationContext>((unsigned)frames);
                init = true;
            }
        }
        
    }

    if (init) {
        mEngine.play(0, 0);
        mPlaying = true;
    }


}

bool Player::isPlaying() const {
    return mPlaying;
}

int Player::progress() const {
    return std::visit([](auto&& ctx) {
        using T = std::decay_t<decltype(ctx)>;

        if constexpr (std::is_same_v<T, LoopContext>) {
            return ctx.visits[ctx.currentPattern];
        } else if constexpr (std::is_same_v<T, DurationContext>) {
            return ctx.frameCounter;
        } else {
            return 0;
        }
    }, mContext);
}

int Player::progressMax() const {
    return std::visit([](auto&& ctx) {
        using T = std::decay_t<decltype(ctx)>;

        if constexpr (std::is_same_v<T, LoopContext>) {
            return ctx.loopAmount;
        } else if constexpr (std::is_same_v<T, DurationContext>) {
            return ctx.framesToPlay;
        } else {
            return 0;
        }
    }, mContext);
}

void Player::step() {
    if (mPlaying) {
        Frame frame;
        mEngine.step(frame);

        if (frame.halted) {
            mPlaying = false;
        } else {
            // check if we have another step via our context variant
            std::visit([this, &frame](auto&& ctx) {
                using T = std::decay_t<decltype(ctx)>;

                if constexpr (std::is_same_v<T, LoopContext>) {
                    if (frame.startedNewPattern) {
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
