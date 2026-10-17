#include "time_manager.h"

using namespace ME::Time;

namespace {
/** How often GetCurrentFPS() recalculates its rolling average. */
constexpr double FPS_WINDOW_INTERVAL = 0.5;
}  // namespace

TimeManager::TimeManager() {}

TimeManager::~TimeManager() {}

void TimeManager::Init(const TimeConfig& inConfig) {
    config = inConfig;

    fixedDeltaTime = 1.0 / config.fixedStepFPS;
    frameDeltaTime = 0.0;
    accumulator = 0.0;
    pendingFixedSteps = 0;

    frameCount = 0;
    fixedStepCount = 0;
    timeSinceStartup = 0.0;

    currentFPS = 0.0;
    fpsWindowTime = 0.0;
    fpsWindowFrames = 0;

    gameStartTP = Clock::now();
    previousFrameEndTP = gameStartTP;
}

void TimeManager::BeginFrame() {
    auto now = Clock::now();
    frameDeltaTime = std::chrono::duration<double>(now - previousFrameEndTP).count();
    if (frameDeltaTime > config.maxFrameTime) {
        frameDeltaTime = config.maxFrameTime;
    }
    previousFrameEndTP = now;

    timeSinceStartup = std::chrono::duration<double>(now - gameStartTP).count();
    ++frameCount;

    accumulator += frameDeltaTime;

    pendingFixedSteps = 0;
    while (accumulator >= fixedDeltaTime && pendingFixedSteps < config.maxFixedStepsPerFrame) {
        accumulator -= fixedDeltaTime;
        ++pendingFixedSteps;
    }
    fixedStepCount += pendingFixedSteps;

    fpsWindowTime += frameDeltaTime;
    ++fpsWindowFrames;
    if (fpsWindowTime >= FPS_WINDOW_INTERVAL) {
        currentFPS = static_cast<double>(fpsWindowFrames) / fpsWindowTime;
        fpsWindowTime = 0.0;
        fpsWindowFrames = 0;
    }
}

void TimeManager::End() {
    gameEndTP = Clock::now();
}

double TimeManager::GetFrameDeltaTime() const {
    return frameDeltaTime;
}

int TimeManager::GetPendingFixedSteps() const {
    return pendingFixedSteps;
}

double TimeManager::GetFixedDeltaTime() const {
    return fixedDeltaTime;
}

uint64_t TimeManager::GetFrameCount() const {
    return frameCount;
}

uint64_t TimeManager::GetFixedStepCount() const {
    return fixedStepCount;
}

double TimeManager::GetTimeSinceStartup() const {
    return timeSinceStartup;
}

double TimeManager::GetCurrentFPS() const {
    return currentFPS;
}

double TimeManager::GetFixedStepFPS() const {
    return config.fixedStepFPS;
}
