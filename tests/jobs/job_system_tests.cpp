/**
 * Tests for JobSystem in inline mode (0 workers).
 */

#include <cstdint>

#include "moksha/shared/jobs/job_system.h"
#include "moksha/test_framework/test_framework.h"

namespace {

class Recorder {
   public:
    int calls = 0;
    uint32_t pendingSeen = 0;  // Counter value seen from inside the last job.
    ME::Jobs::JobCounter* counter = nullptr;
};

void RecordJob(void* context) {
    Recorder* recorder = static_cast<Recorder*>(context);
    ++recorder->calls;
    if (recorder->counter != nullptr) {
        recorder->pendingSeen = recorder->counter->pending.load();
    }
}

void AddOneJob(void* context) {
    ++*static_cast<int*>(context);
}

}  // namespace

TEST(JobSystem, InitWithZeroWorkers) {
    ME::Jobs::JobSystem jobs;
    EXPECT(jobs.Init(0, 64));
    EXPECT(jobs.GetWorkerCount() == 0);
    jobs.End();
}

TEST(JobSystem, InitRejectsWorkersForNow) {
    ME::Jobs::JobSystem jobs;
    EXPECT(!jobs.Init(4, 64));
}

TEST(JobSystem, InitTwiceFails) {
    ME::Jobs::JobSystem jobs;
    EXPECT(jobs.Init(0, 64));
    EXPECT(!jobs.Init(0, 64));
    jobs.End();
}

TEST(JobSystem, CanInitAgainAfterEnd) {
    ME::Jobs::JobSystem jobs;
    EXPECT(jobs.Init(0, 64));
    jobs.End();
    EXPECT(jobs.Init(0, 64));
    jobs.End();
}

TEST(JobSystem, SubmitBeforeInitFails) {
    ME::Jobs::JobSystem jobs;
    int value = 0;
    EXPECT(!jobs.Submit(&AddOneJob, &value, nullptr));
    EXPECT(value == 0);
}

TEST(JobSystem, SubmitNullJobFails) {
    ME::Jobs::JobSystem jobs;
    jobs.Init(0, 64);
    ME::Jobs::JobCounter counter;

    EXPECT(!jobs.Submit(nullptr, nullptr, &counter));
    EXPECT(counter.pending.load() == 0);
    jobs.End();
}

TEST(JobSystem, JobRunsOnceWithItsContext) {
    ME::Jobs::JobSystem jobs;
    jobs.Init(0, 64);
    Recorder recorder;

    EXPECT(jobs.Submit(&RecordJob, &recorder, nullptr));
    EXPECT(recorder.calls == 1);
    jobs.End();
}

TEST(JobSystem, CounterIsHeldWhileJobRuns) {
    ME::Jobs::JobSystem jobs;
    jobs.Init(0, 64);
    ME::Jobs::JobCounter counter;
    Recorder recorder;
    recorder.counter = &counter;

    jobs.Submit(&RecordJob, &recorder, &counter);
    EXPECT(recorder.pendingSeen == 1);
    EXPECT(counter.pending.load() == 0);
    jobs.End();
}

TEST(JobSystem, ManyJobsAllRunAndWaitReturns) {
    ME::Jobs::JobSystem jobs;
    jobs.Init(0, 64);
    ME::Jobs::JobCounter counter;
    int value = 0;

    for (int i = 0; i < 100; ++i) {
        EXPECT(jobs.Submit(&AddOneJob, &value, &counter));
    }
    jobs.Wait(counter);

    EXPECT(value == 100);
    EXPECT(counter.pending.load() == 0);
    jobs.End();
}

TEST(JobSystem, WaitOnUnusedCounterReturns) {
    ME::Jobs::JobSystem jobs;
    jobs.Init(0, 64);
    ME::Jobs::JobCounter counter;

    jobs.Wait(counter);
    EXPECT(counter.pending.load() == 0);
    jobs.End();
}
