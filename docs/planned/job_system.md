# Planned: job system and parallel for

Goal: let engine systems and games split CPU work across cores: a **job system** (submit small tasks, wait on a
counter) and **`ParallelFor`** (split an index range into chunks, built on the job system). Not implemented yet;
`moksha/shared/jobs/` holds empty skeleton files (2026-11-20).

## Where it lives
- `moksha/shared/jobs/`, namespace `ME::Jobs`, in `Moksha_Shared`, so clients, servers and tools can all use it.
- `job_system.h/.cpp` (`JobSystem`, `JobCounter`), `parallel_for.h/.cpp` (`ParallelFor`).
- Owned like `PhysicsSystem`: `ClientApp` (and the server apps) hold one `JobSystem` with an explicit
  `Init()` / `End()`, and the client hands it to the game through `Engine::jobs`. No global, no singleton.

## Rules it must follow (from the root CLAUDE.md)
- **The engine must not require threads.** `Init(workerCount = 0)` is a valid, fully working mode: every job runs
  inline on the calling thread. The web build (no pthreads) and headless tests always work this way, and every
  other mode must give the same results.
- **The app loop never blocks.** Waiting on a counter never sleeps the main thread while work is queued: the waiter
  runs queued jobs itself (the "main thread helps" rule), so a frame can't stall on idle workers. Jobs must not
  wait on work that is not already submitted.
- **No exceptions, no `std::vector`.** Fixed-capacity job queue (ring buffer, size set at `Init`); `Submit` returns
  `false` when full and the caller runs the job inline instead.
- **No OS calls outside platform folders.** Threads use `std::thread` / `std::mutex` / `std::condition_variable`
  (standard library). Anything OS-specific (thread names, affinity, core count beyond
  `std::thread::hardware_concurrency`) goes in `moksha/shared/platform/`.
- Callbacks use bound context (function pointer + `void*`), like `Delegate`.

## API sketch (target, may change per step)
```cpp
namespace ME::Jobs {
using JobFn = void (*)(void* context);
using RangeFn = void (*)(void* context, uint32_t begin, uint32_t end);

class JobCounter { public: std::atomic<uint32_t> pending{0}; };  // 0 = all done

class JobSystem {
   public:
    bool Init(uint32_t workerCount, uint32_t queueCapacity);  // 0 workers = run inline
    void End();                                                // joins workers; queue must be empty
    bool Submit(JobFn fn, void* context, JobCounter* counter); // false = queue full
    void Wait(JobCounter& counter);                            // runs queued jobs until counter is 0
    uint32_t GetWorkerCount() const;
};

// Calls fn on [0, count) in chunks of at least minChunk; returns when every chunk is done.
void ParallelFor(JobSystem& jobs, uint32_t count, uint32_t minChunk, RangeFn fn, void* context);
}
```
A typed `Bind<T, &T::Method>` helper, as in `Delegate`, can come later.

## Steps
Each step builds and runs on its own; stop after each one.
1. **Skeleton** (done): this doc, empty `moksha/shared/jobs/` files. Re-run `cmake -B build` (new `.cpp`s).
2. **Inline job system** (done, 2026-11-20): `Init` / `End` / `Submit` / `Wait` with 0 workers: `Submit` runs the job at once and
   decrements the counter. Tests in `tests/jobs/` (counter reaches 0, context passed through, full-queue path).
3. **`ParallelFor`, inline**: chunking math (count 0, count < minChunk, uneven last chunk, chunk count capped by
   workers + 1). Tests check every index is visited exactly once.
4. **Worker threads**: `std::thread` workers, mutex + condition variable around the ring buffer, atomic counters,
   `Wait` helps run jobs. Link `Threads::Threads` on Linux/Android in `moksha/shared/CMakeLists.txt`; on `ME_WEB`
   force 0 workers. Same tests, run with 0 and N workers. Check on the Raspberry Pi (4 cores, Linux ARM64) and
   with a ThreadSanitizer build where the toolchain has one.
5. **Hook-up**: done early (2026-11-20): `ClientApp` (`Engine::jobs`) and `ServerApp` (`GetJobs()`) each own a
   `JobSystem` (0 workers, `Constants::MaxQueuedJobCount`); guide and cookbook updated. Left: worker count from `AppConfig`
   (`settings.ini` `[jobs] workers=`, default `hardware_concurrency() - 1`, capped, 0 on web). Update
   `game_guide.md`, `game_cookbook.md` (a `ParallelFor` recipe) and `moksha/shared/CLAUDE.md`.
6. **First engine user** (optional): pick one system whose loop is independent per element and measure it.

## Later (not planned in detail)
- Lock-free or per-worker queues with stealing, if the single locked queue shows up in profiles.
- Job dependencies / continuations (run B when counter A reaches 0).
- Thread names in debuggers/profilers (`shared/platform/`).
- A pthreads web build (`-pthread`, cross-origin isolation headers in `cmake/web/shell.html.in`).

## Open questions
- Who may submit: main thread only at first, or also jobs from inside jobs (nested `ParallelFor`)?
- Should mobile cap workers lower (big.LITTLE cores, battery), e.g. at 3?
- Determinism: results from `ParallelFor` must not depend on chunk order; anything reduced across chunks (sums,
  `FP_24_8` physics) needs per-chunk outputs combined in index order. Document this as a rule for game code.
