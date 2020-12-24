
#pragma once

#include <atomic>

// if performance is an issue using this, try this https://timur.audio/using-locks-in-real-time-audio-processing-safely

// Locks occur when:
//  - the audio thread is rendering a frame (typically once every 16 ms)
//  - the gui thread is modifying the document

// excessive spinning should rarely occur

class Spinlock {

public:

    Spinlock() {
        mFlag.clear();
    }
    ~Spinlock() = default;

    // no copying
    Spinlock(Spinlock const &ref) = delete;
    void operator=(Spinlock const &ref) = delete;

    
    void lock() noexcept {
        while (mFlag.test_and_set(std::memory_order_acquire));
    }

    bool tryLock() noexcept {
        return !mFlag.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept {
        mFlag.clear(std::memory_order_release);
    }

private:

    std::atomic_flag mFlag = ATOMIC_FLAG_INIT;


};
