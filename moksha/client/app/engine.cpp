#include "engine.h"

void ME::Engine::Quit() {
    quitRequested = true;
}

bool ME::Engine::IsQuitRequested() const {
    return quitRequested;
}

bool ME::Engine::IsHeadless() const {
#ifdef ME_HEADLESS
    return true;
#else
    return false;
#endif
}
