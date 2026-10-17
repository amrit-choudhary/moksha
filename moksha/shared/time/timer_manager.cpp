#include "moksha/shared/time/timer_manager.h"

#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"

using namespace ME::Time;

TimerManager::TimerManager() {}

TimerManager::~TimerManager() {
    End();
}

void TimerManager::Init(double inFixedStepFPS) {
    delete[] slots.data;
    slots.data = new TimerSlot[Constants::MaxTimerCount];
    slots.count = Constants::MaxTimerCount;

    // nextId is not reset: ids stay unique for the manager's lifetime, even across re-Init.
    currentTick = 0;
    fixedStepFPS = inFixedStepFPS;
}

void TimerManager::End() {
    delete[] slots.data;
    slots.data = nullptr;
    slots.count = 0;
}

TimerHandle TimerManager::SetTimeout(const Delegate& callback, double delaySeconds) {
    return Schedule(callback, SecondsToTicks(delaySeconds), 0);
}

TimerHandle TimerManager::SetInterval(const Delegate& callback, double intervalSeconds) {
    uint64_t intervalTicks = SecondsToTicks(intervalSeconds);
    return Schedule(callback, intervalTicks, intervalTicks);
}

TimerHandle TimerManager::SetNextTick(const Delegate& callback) {
    return Schedule(callback, 1, 0);
}

bool TimerManager::Clear(TimerHandle& handle) {
    TimerSlot* slot = FindSlot(handle.id);
    handle.id = 0;
    if (slot == nullptr) {
        return false;
    }
    *slot = TimerSlot{};
    return true;
}

void TimerManager::ClearAll() {
    for (size_t i = 0; i < slots.count; ++i) {
        slots[i] = TimerSlot{};
    }
}

bool TimerManager::IsActive(TimerHandle handle) const {
    return FindSlot(handle.id) != nullptr;
}

size_t TimerManager::GetActiveCount() const {
    size_t count = 0;
    for (size_t i = 0; i < slots.count; ++i) {
        if (slots[i].id != 0) {
            ++count;
        }
    }
    return count;
}

void TimerManager::Tick(double fixedDeltaTime) {
    ++currentTick;

    // Anything scheduled from inside a callback expires at currentTick + 1 or later, so it
    // can never fire during this pass even if it lands in a slot not yet visited.
    for (size_t i = 0; i < slots.count; ++i) {
        TimerSlot& slot = slots[i];
        if (slot.id == 0 || slot.expireTick > currentTick) {
            continue;
        }

        // Update the slot before firing, so the callback may Clear its own handle or reuse the slot.
        // Repeating timers advance from their scheduled tick, not the current one, so they never drift.
        Delegate callback = slot.callback;
        if (slot.intervalTicks > 0) {
            slot.expireTick += slot.intervalTicks;
        } else {
            slot = TimerSlot{};
        }
        callback.Execute();
    }
}

TimerHandle TimerManager::Schedule(const Delegate& callback, uint64_t delayTicks, uint64_t intervalTicks) {
    if (callback.invoke == nullptr) {
        LogError("TimerManager: cannot schedule a timer with an unbound callback.");
        return TimerHandle{};
    }

    for (size_t i = 0; i < slots.count; ++i) {
        TimerSlot& slot = slots[i];
        if (slot.id != 0) {
            continue;
        }

        slot.id = nextId++;
        slot.callback = callback;
        slot.expireTick = currentTick + delayTicks;
        slot.intervalTicks = intervalTicks;
        return TimerHandle{slot.id};
    }

    LogError("TimerManager: all ", Constants::MaxTimerCount, " timer slots are in use (or Init was not called).");
    return TimerHandle{};
}

uint64_t TimerManager::SecondsToTicks(double seconds) const {
    double ticks = seconds * fixedStepFPS + 0.5;
    if (ticks < 1.0) {
        return 1;
    }
    return static_cast<uint64_t>(ticks);
}

TimerSlot* TimerManager::FindSlot(uint64_t id) const {
    if (id == 0) {
        return nullptr;
    }
    for (size_t i = 0; i < slots.count; ++i) {
        if (slots[i].id == id) {
            return &slots[i];
        }
    }
    return nullptr;
}
