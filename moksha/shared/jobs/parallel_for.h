/**
 * ParallelFor: splits an index range [0, count) into chunks and runs them as jobs on a JobSystem,
 * returning when every chunk is done. Results must not depend on the order chunks run in.
 * Plan: docs/planned/job_system.md. Skeleton only; not implemented yet.
 */

#pragma once

#include <cstdint>

#include "moksha/shared/jobs/job_system.h"

namespace ME::Jobs {

// TODO(step 3): void ParallelFor(JobSystem& jobs, uint32_t count, uint32_t minChunk, RangeFn fn, void* context);

}  // namespace ME::Jobs
