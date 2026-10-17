/**
 * Tests for TimerManager.
 */

#include "moksha/shared/misc/delegate.h"
#include "moksha/shared/misc/engine_limits.h"
#include "moksha/shared/time/timer_manager.h"
#include "moksha/test_framework/test_framework.h"

namespace {

constexpr double kFixedDeltaTime = 1.0 / 60.0;

class CallCounter {
   public:
    int calls = 0;

    void OnFire() {
        ++calls;
    }
};

// From inside its callback, schedules two next-tick timers on target.
class Rescheduler {
   public:
    ME::Time::TimerManager* timers = nullptr;
    CallCounter* target = nullptr;

    void OnFire() {
        timers->SetNextTick(ME::Delegate::Make<CallCounter, &CallCounter::OnFire>(target));
        timers->SetNextTick(ME::Delegate::Make<CallCounter, &CallCounter::OnFire>(target));
    }
};

// From inside its callback, clears the handle it was given.
class HandleClearer {
   public:
    ME::Time::TimerManager* timers = nullptr;
    ME::Time::TimerHandle* handle = nullptr;
    int calls = 0;
    bool clearResult = true;

    void OnFire() {
        ++calls;
        clearResult = timers->Clear(*handle);
    }
};

ME::Delegate CounterDelegate(CallCounter& counter) {
    return ME::Delegate::Make<CallCounter, &CallCounter::OnFire>(&counter);
}

void TickN(ME::Time::TimerManager& timers, int count, double fixedDeltaTime = kFixedDeltaTime) {
    for (int i = 0; i < count; ++i) {
        timers.Tick(fixedDeltaTime);
    }
}

}  // namespace

TEST(TimerManager, TimeoutFiresAfterRoundedTicks) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetTimeout(CounterDelegate(counter), 0.5);  // 30 ticks
    TickN(timers, 29);
    EXPECT(counter.calls == 0);

    timers.Tick(kFixedDeltaTime);
    EXPECT(counter.calls == 1);
}

TEST(TimerManager, TimeoutFiresOnlyOnce) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetTimeout(CounterDelegate(counter), 0.1);
    TickN(timers, 100);

    EXPECT(counter.calls == 1);
}

TEST(TimerManager, NextTickFiresAfterOneTick) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetNextTick(CounterDelegate(counter));
    EXPECT(counter.calls == 0);

    timers.Tick(kFixedDeltaTime);
    EXPECT(counter.calls == 1);
}

TEST(TimerManager, ZeroTinyAndNegativeDelaysFireNextTick) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetTimeout(CounterDelegate(counter), 0.0);
    timers.SetTimeout(CounterDelegate(counter), 0.001);
    timers.SetTimeout(CounterDelegate(counter), -1.0);
    timers.Tick(kFixedDeltaTime);

    EXPECT(counter.calls == 3);
}

TEST(TimerManager, DelayRoundsToNearestTick) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetTimeout(CounterDelegate(counter), 0.025);  // 1.5 ticks -> 2
    timers.Tick(kFixedDeltaTime);
    EXPECT(counter.calls == 0);

    timers.Tick(kFixedDeltaTime);
    EXPECT(counter.calls == 1);
}

TEST(TimerManager, DelayUsesConfiguredRate) {
    ME::Time::TimerManager timers;
    timers.Init(30.0);
    CallCounter counter;

    timers.SetTimeout(CounterDelegate(counter), 1.0);  // 30 ticks at 30 Hz
    TickN(timers, 29, 1.0 / 30.0);
    EXPECT(counter.calls == 0);

    timers.Tick(1.0 / 30.0);
    EXPECT(counter.calls == 1);
}

TEST(TimerManager, HandleActiveUntilFired) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetTimeout(CounterDelegate(counter), 2.0 / 60.0);
    EXPECT(handle.id != 0);
    EXPECT(timers.IsActive(handle));

    timers.Tick(kFixedDeltaTime);
    EXPECT(timers.IsActive(handle));

    timers.Tick(kFixedDeltaTime);
    EXPECT(!timers.IsActive(handle));
}

TEST(TimerManager, ClearCancelsAndResetsHandle) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetNextTick(CounterDelegate(counter));
    ME::Time::TimerHandle copy = handle;

    EXPECT(timers.Clear(handle));
    EXPECT(handle.id == 0);
    EXPECT(!timers.IsActive(copy));
    EXPECT(!timers.Clear(copy));

    TickN(timers, 10);
    EXPECT(counter.calls == 0);
}

TEST(TimerManager, ZeroHandleIsNeverActive) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    ME::Time::TimerHandle handle;

    EXPECT(!timers.IsActive(handle));
    EXPECT(!timers.Clear(handle));
}

TEST(TimerManager, StaleHandleDoesNotAffectNewTimer) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter first;
    CallCounter second;

    ME::Time::TimerHandle stale = timers.SetNextTick(CounterDelegate(first));
    timers.Tick(kFixedDeltaTime);

    // Likely reuses the slot `stale` used, but gets a new id.
    ME::Time::TimerHandle fresh = timers.SetNextTick(CounterDelegate(second));
    EXPECT(fresh.id != stale.id);
    EXPECT(!timers.Clear(stale));
    EXPECT(timers.IsActive(fresh));

    timers.Tick(kFixedDeltaTime);
    EXPECT(first.calls == 1);
    EXPECT(second.calls == 1);
}

TEST(TimerManager, UnboundCallbackRejected) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    ME::Delegate unbound;

    ME::Time::TimerHandle handle = timers.SetNextTick(unbound);

    EXPECT(handle.id == 0);
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, ScheduleBeforeInitRejected) {
    ME::Time::TimerManager timers;
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetNextTick(CounterDelegate(counter));

    EXPECT(handle.id == 0);
}

TEST(TimerManager, RejectsWhenFull) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    for (size_t i = 0; i < ME::Constants::MaxTimerCount; ++i) {
        ASSERT(timers.SetTimeout(CounterDelegate(counter), 1.0).id != 0);
    }
    EXPECT(timers.GetActiveCount() == ME::Constants::MaxTimerCount);
    EXPECT(timers.SetNextTick(CounterDelegate(counter)).id == 0);

    TickN(timers, 60);
    EXPECT(counter.calls == static_cast<int>(ME::Constants::MaxTimerCount));
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, ClearAllCancelsEverything) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetNextTick(CounterDelegate(counter));
    timers.SetTimeout(CounterDelegate(counter), 1.0);
    timers.ClearAll();

    EXPECT(timers.GetActiveCount() == 0);
    EXPECT(!timers.IsActive(handle));
    TickN(timers, 120);
    EXPECT(counter.calls == 0);
}

TEST(TimerManager, GetActiveCountTracksTimers) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    EXPECT(timers.GetActiveCount() == 0);
    timers.SetNextTick(CounterDelegate(counter));
    ME::Time::TimerHandle later = timers.SetTimeout(CounterDelegate(counter), 1.0);
    EXPECT(timers.GetActiveCount() == 2);

    timers.Tick(kFixedDeltaTime);
    EXPECT(timers.GetActiveCount() == 1);

    timers.Clear(later);
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, ScheduledFromCallbackFiresNextTick) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter target;
    Rescheduler rescheduler;
    rescheduler.timers = &timers;
    rescheduler.target = &target;

    // Its two new timers land in the freed slot and in a slot not yet visited this tick.
    timers.SetNextTick(ME::Delegate::Make<Rescheduler, &Rescheduler::OnFire>(&rescheduler));
    timers.Tick(kFixedDeltaTime);
    EXPECT(target.calls == 0);

    timers.Tick(kFixedDeltaTime);
    EXPECT(target.calls == 2);
}

TEST(TimerManager, CallbackClearingOwnHandleIsSafe) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    HandleClearer clearer;
    ME::Time::TimerHandle handle;
    clearer.timers = &timers;
    clearer.handle = &handle;

    handle = timers.SetNextTick(ME::Delegate::Make<HandleClearer, &HandleClearer::OnFire>(&clearer));
    timers.Tick(kFixedDeltaTime);

    EXPECT(clearer.calls == 1);
    EXPECT(!clearer.clearResult);  // one-shot is already finished when its callback runs
    EXPECT(handle.id == 0);
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, CallbackCanClearLaterTimerInSameTick) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter victim;
    HandleClearer clearer;
    ME::Time::TimerHandle victimHandle;
    clearer.timers = &timers;
    clearer.handle = &victimHandle;

    // Both due on the same tick; the clearer sits in the earlier slot and fires first.
    timers.SetNextTick(ME::Delegate::Make<HandleClearer, &HandleClearer::OnFire>(&clearer));
    victimHandle = timers.SetNextTick(CounterDelegate(victim));
    timers.Tick(kFixedDeltaTime);

    EXPECT(clearer.calls == 1);
    EXPECT(clearer.clearResult);
    EXPECT(victim.calls == 0);
}

TEST(TimerManager, IntervalFiresEveryInterval) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetInterval(CounterDelegate(counter), 0.5);  // every 30 ticks
    TickN(timers, 29);
    EXPECT(counter.calls == 0);

    timers.Tick(kFixedDeltaTime);
    EXPECT(counter.calls == 1);

    TickN(timers, 29);
    EXPECT(counter.calls == 1);

    timers.Tick(kFixedDeltaTime);
    EXPECT(counter.calls == 2);
}

TEST(TimerManager, IntervalDoesNotDrift) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    // 0.1s is 6 ticks; summing 1/60 in floating point would fire a tick late each time.
    timers.SetInterval(CounterDelegate(counter), 0.1);
    TickN(timers, 600);

    EXPECT(counter.calls == 100);
}

TEST(TimerManager, IntervalStaysActiveWithSameHandle) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetInterval(CounterDelegate(counter), 1.0 / 60.0);
    uint64_t id = handle.id;
    TickN(timers, 5);

    EXPECT(counter.calls == 5);
    EXPECT(timers.IsActive(handle));
    EXPECT(handle.id == id);
    EXPECT(timers.GetActiveCount() == 1);
}

TEST(TimerManager, IntervalMinimumIsOneTick) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetInterval(CounterDelegate(counter), 0.0);
    TickN(timers, 5);

    EXPECT(counter.calls == 5);
}

TEST(TimerManager, ClearStopsInterval) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetInterval(CounterDelegate(counter), 1.0 / 60.0);
    TickN(timers, 3);
    EXPECT(timers.Clear(handle));
    TickN(timers, 10);

    EXPECT(counter.calls == 3);
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, IntervalCanClearItselfFromCallback) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    HandleClearer clearer;
    ME::Time::TimerHandle handle;
    clearer.timers = &timers;
    clearer.handle = &handle;

    handle = timers.SetInterval(ME::Delegate::Make<HandleClearer, &HandleClearer::OnFire>(&clearer), 0.1);
    TickN(timers, 6);
    EXPECT(clearer.calls == 1);
    EXPECT(clearer.clearResult);  // a repeating timer is still scheduled while its callback runs
    EXPECT(handle.id == 0);

    TickN(timers, 60);
    EXPECT(clearer.calls == 1);
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, IntervalUnboundCallbackRejected) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    ME::Delegate unbound;

    EXPECT(timers.SetInterval(unbound, 1.0).id == 0);
    EXPECT(timers.GetActiveCount() == 0);
}

TEST(TimerManager, ClearAllStopsIntervals) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    timers.SetInterval(CounterDelegate(counter), 1.0 / 60.0);
    timers.Tick(kFixedDeltaTime);
    timers.ClearAll();
    TickN(timers, 10);

    EXPECT(counter.calls == 1);
}

TEST(TimerManager, EndDropsTimersAndRejectsNewOnes) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter counter;

    ME::Time::TimerHandle handle = timers.SetInterval(CounterDelegate(counter), 1.0 / 60.0);
    timers.End();

    TickN(timers, 10);
    EXPECT(counter.calls == 0);
    EXPECT(!timers.IsActive(handle));
    EXPECT(timers.GetActiveCount() == 0);
    EXPECT(timers.SetNextTick(CounterDelegate(counter)).id == 0);

    timers.End();  // second End is a no-op
}

TEST(TimerManager, HandleFromBeforeReInitDoesNotMatchNewTimer) {
    ME::Time::TimerManager timers;
    timers.Init(60.0);
    CallCounter first;
    CallCounter second;

    ME::Time::TimerHandle old = timers.SetTimeout(CounterDelegate(first), 1.0);
    timers.Init(60.0);
    ME::Time::TimerHandle fresh = timers.SetTimeout(CounterDelegate(second), 1.0);

    EXPECT(fresh.id != old.id);
    EXPECT(!timers.IsActive(old));
    EXPECT(!timers.Clear(old));
    EXPECT(timers.IsActive(fresh));
}
