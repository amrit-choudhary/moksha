/**
 * Pacing for loops that nothing else paces (servers, headless clients, vsync off): sleeps until the next fixed
 * step is due instead of spinning a CPU core. Platforms whose OS drives the frame (web, mobile) never call it.
 */
#pragma once

namespace ME::Time {

class TimeManager;

// Sleeps until timeManager's next fixed step is due (as of its last BeginFrame); returns at once if one is due.
void WaitForNextFixedStep(const TimeManager& timeManager);

}  // namespace ME::Time
