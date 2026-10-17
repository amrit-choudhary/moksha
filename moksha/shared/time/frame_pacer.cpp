#include "frame_pacer.h"

#include <chrono>
#include <thread>

#include "time_manager.h"

void ME::Time::WaitForNextFixedStep(const TimeManager& timeManager) {
    const double seconds = timeManager.GetTimeUntilNextFixedStep();
    if (seconds <= 0.0) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}
