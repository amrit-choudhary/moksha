#pragma once

/**
 * Will have all the global variables of the game.
 */

#include <cstdint>

#include "moksha/shared/misc/utils.h"

namespace ME {

/**
 * Global variables class.
 * Contains all the global variables used in the game.
 */
class GlobalVars {
   public:
    inline static bool gameRunning = false;

    static uint16_t GetWindowWidth() {
        return windowWidth;
    }

    static uint16_t GetWindowHeight() {
        return windowHeight;
    }

    /**
     * Gets the combined window size as a single uint32_t.
     * The higher 16 bits are the width and the lower 16 bits are the height.
     */
    static uint32_t GetWindowSizeCombined() {
        return ME::Utils::Pack16To32(windowWidth, windowHeight);
    }

    static void SetWindowSize(uint16_t width, uint16_t height) {
        windowWidth = width;
        windowHeight = height;
    }

   private:
    inline static uint16_t windowWidth = 0;
    inline static uint16_t windowHeight = 0;
};

}  // namespace ME
