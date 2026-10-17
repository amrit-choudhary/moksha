/**
 * Time Management classes.
 * Responsible for fixed-timestep simulation ticks and delta-time/fps reporting.
 *
 * Presentation pacing is not this class's job: the game has exactly two modes, both handled
 * elsewhere. Vsync on -> the renderer's Present() paces frame rate to the display's refresh
 * rate. Vsync off -> the loop runs completely unthrottled (no sleep/spin here). Either way,
 * BeginFrame() just measures however much real time actually elapsed and advances the
 * fixed-step accumulator - fixedStepFPS (the deterministic simulation/gameplay/physics tick
 * rate) always gets a constant dt regardless of how fast or slow real frames arrive. See
 * GetFixedDeltaTime()/GetPendingFixedSteps().
 */

#pragma once

#include <chrono>
#include <cstdint>

namespace ME::Time {
// Common target rates, usable for fixedStepFPS.
constexpr double FPS_1 = 1.0;
constexpr double FPS_10 = 10.0;
constexpr double FPS_30 = 30.0;
constexpr double FPS_60 = 60.0;
constexpr double FPS_120 = 120.0;

class TimeConfig {
   public:
    /** Deterministic simulation tick rate (gameplay/physics). */
    double fixedStepFPS = FPS_60;
    /** Clamp applied to a single frame's raw elapsed time before it reaches the accumulator.
     * Guards against a spiral of death after a long stall (debugger break, window resize, etc). */
    double maxFrameTime = 0.25;
    /** Hard cap on how many fixed steps BeginFrame() will report as pending in one call,
     * regardless of how much time has accumulated. */
    int maxFixedStepsPerFrame = 8;
};

class TimeManager {
   public:
    TimeManager();
    ~TimeManager();

    /** Initialise the clocks and timestep configuration. */
    void Init(const TimeConfig& config);

    /** Call once per real loop iteration. Measures real elapsed time and advances the
     * fixed-step accumulator. Does not pace or sleep - see class comment. */
    void BeginFrame();

    /** Stop the timers so final average-fps stats can be read after the game loop ends. */
    void End();

    // Getters
    /** Variable, real per-frame delta time - for input/UI/render. */
    double GetFrameDeltaTime() const;
    /** How many fixed steps to run this frame (0..maxFixedStepsPerFrame). */
    int GetPendingFixedSteps() const;
    /** Constant simulation dt, always 1/fixedStepFPS. */
    double GetFixedDeltaTime() const;
    /** Total real frames (BeginFrame calls) since startup. */
    uint64_t GetFrameCount() const;
    /** Total fixed simulation steps executed since startup. */
    uint64_t GetFixedStepCount() const;
    double GetTimeSinceStartup() const;

    /** Live render frame rate, averaged over a short rolling window (recomputed a couple times
     * per second) rather than the raw, jittery 1/GetFrameDeltaTime() of a single frame - meant
     * for a debug overlay display. */
    double GetCurrentFPS() const;
    /** The configured deterministic simulation rate (== 1/GetFixedDeltaTime()), for display
     * alongside GetCurrentFPS(). */
    double GetFixedStepFPS() const;

   private:
    using Clock = std::chrono::high_resolution_clock;

    TimeConfig config;

    /** Constant simulation dt, = 1/config.fixedStepFPS. */
    double fixedDeltaTime;
    /** Real, variable time the last BeginFrame() call took. */
    double frameDeltaTime;
    /** Fixed-step accumulator: banked real time not yet consumed by a simulation step. */
    double accumulator;
    /** Fixed steps to run this frame, computed by the last BeginFrame() call. */
    int pendingFixedSteps;

    uint64_t frameCount;
    uint64_t fixedStepCount;

    /** GetCurrentFPS()'s rolling-window state: frames and real time accumulated since the last
     * recalculation. */
    double currentFPS;
    double fpsWindowTime;
    uint64_t fpsWindowFrames;

    /** Game time since startup in seconds. */
    double timeSinceStartup;

    std::chrono::time_point<Clock> gameStartTP;
    std::chrono::time_point<Clock> gameEndTP;
    std::chrono::time_point<Clock> previousFrameEndTP;
};
}  // namespace ME::Time
