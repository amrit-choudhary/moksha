#include "moksha/shared/jobs/job_system.h"

#include "moksha/logging/logging.h"

namespace ME::Jobs {

bool JobSystem::Init(uint32_t inWorkerCount, uint32_t queueCapacity) {
    (void)queueCapacity;  // Used once jobs are queued for workers.

    if (isInitialized) {
        LogError("JobSystem: Init called twice.");
        return false;
    }
    if (inWorkerCount != 0) {
        LogError("JobSystem: worker threads are not supported yet; use 0 workers.");
        return false;
    }

    workerCount = inWorkerCount;
    isInitialized = true;
    return true;
}

void JobSystem::End() {
    isInitialized = false;
    workerCount = 0;
}

bool JobSystem::Submit(JobFn fn, void* context, JobCounter* counter) {
    if (!isInitialized) {
        LogError("JobSystem: Submit called before Init.");
        return false;
    }
    if (fn == nullptr) {
        LogError("JobSystem: cannot submit a null job.");
        return false;
    }

    if (counter != nullptr) {
        counter->pending.fetch_add(1, std::memory_order_relaxed);
    }

    // Inline mode: the job runs now, on the caller.
    fn(context);

    if (counter != nullptr) {
        counter->pending.fetch_sub(1, std::memory_order_release);
    }
    return true;
}

void JobSystem::Wait(JobCounter& counter) {
    // Inline mode: Submit has already run every job, so the counter is 0 here.
    // Worker mode will run queued jobs on the caller until the counter reaches 0.
    if (counter.pending.load(std::memory_order_acquire) != 0) {
        LogError("JobSystem: Wait on a counter that no submitted job will bring to 0.");
    }
}

uint32_t JobSystem::GetWorkerCount() const {
    return workerCount;
}

}  // namespace ME::Jobs
