/**
 * Job system: runs small tasks (function pointer + context) and tracks them with counters.
 * With 0 workers every job runs inline in Submit, so nothing requires threads (web, tests).
 * Only the inline mode exists so far; worker threads come later (docs/planned/job_system.md).
 */

#pragma once

#include <atomic>
#include <cstdint>

namespace ME::Jobs {

// A job: a plain function called with the context it was submitted with.
using JobFn = void (*)(void* context);

// Counts submitted jobs that have not finished; 0 = all done. Must outlive every job submitted with it.
class JobCounter {
   public:
    std::atomic<uint32_t> pending{0};
};

class JobSystem {
   public:
    JobSystem() = default;
    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    // Starts the system. 0 workers = every job runs inline in Submit (the only mode supported so far).
    // queueCapacity is reserved for worker mode. Returns false on bad arguments or if already initialized.
    bool Init(uint32_t workerCount, uint32_t queueCapacity);

    // Stops the system. Every counter must have reached 0.
    void End();

    // Runs fn(context). counter (may be null) is incremented before and decremented after the job runs.
    // Returns false (nothing runs) if fn is null or the system is not initialized.
    bool Submit(JobFn fn, void* context, JobCounter* counter);

    // Returns once counter reaches 0. The caller runs queued jobs while waiting, so it never idles on work.
    void Wait(JobCounter& counter);

    // Number of worker threads, not counting the caller.
    uint32_t GetWorkerCount() const;

   private:
    bool isInitialized = false;
    uint32_t workerCount = 0;
};

}  // namespace ME::Jobs
