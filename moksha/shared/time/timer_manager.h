/**
 * Fires Delegates after a delay or on a repeating interval, counted in fixed simulation ticks.
 * Durations are given in seconds and rounded to whole ticks (minimum 1) at schedule time.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "moksha/shared/datastructure/span.h"
#include "moksha/shared/misc/delegate.h"

namespace ME::Time {

// Identifies one scheduled timer. 0 = no timer. Ids are never reused, so a handle whose
// timer has finished or been cleared can never affect a newer timer.
class TimerHandle {
   public:
    uint64_t id = 0;
};

// Internal to TimerManager: one timer's storage, free when id is 0.
// Not for game-side use; schedule and refer to timers through TimerManager and TimerHandle.
class TimerSlot {
   public:
    uint64_t id = 0;
    Delegate callback;
    uint64_t expireTick = 0;
    uint64_t intervalTicks = 0;  // 0 = one-shot
};

class TimerManager {
   public:
    TimerManager();
    ~TimerManager();

    TimerManager(const TimerManager&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;
    TimerManager(TimerManager&&) = delete;
    TimerManager& operator=(TimerManager&&) = delete;

    // Allocates Constants::MaxTimerCount slots. fixedStepFPS must match the rate Tick() is called at.
    void Init(double fixedStepFPS);

    // Drops every timer and frees the slots. Call before the objects the callbacks point at are destroyed.
    void End();

    // Calls callback once, delaySeconds from now. Returns a 0 handle if full or callback is unbound.
    TimerHandle SetTimeout(const Delegate& callback, double delaySeconds);

    // Calls callback every intervalSeconds until cleared; first call one interval from now.
    TimerHandle SetInterval(const Delegate& callback, double intervalSeconds);

    // Calls callback once, on the next Tick().
    TimerHandle SetNextTick(const Delegate& callback);

    // Cancels the timer and resets handle to 0. Returns false if it had already finished or been cleared.
    bool Clear(TimerHandle& handle);

    // Cancels every scheduled timer, e.g. before the objects their callbacks point at are destroyed.
    void ClearAll();

    // True while the timer is still scheduled (one-shot not yet fired, or repeating not cleared).
    bool IsActive(TimerHandle handle) const;

    // Number of currently scheduled timers, out of Constants::MaxTimerCount.
    size_t GetActiveCount() const;

    // Advances one fixed step and fires every due timer. Call at the start of each fixed step.
    // fixedDeltaTime matches the other systems' fixed update; timers count ticks, not this value.
    void Tick(double fixedDeltaTime);

   private:
    TimerHandle Schedule(const Delegate& callback, uint64_t delayTicks, uint64_t intervalTicks);
    uint64_t SecondsToTicks(double seconds) const;
    TimerSlot* FindSlot(uint64_t id) const;

    Span<TimerSlot> slots;  // owned: allocated in Init, freed in End (or the destructor)
    uint64_t nextId = 1;
    uint64_t currentTick = 0;
    double fixedStepFPS = 0.0;
};

}  // namespace ME::Time
