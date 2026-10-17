/**
 * The link between the engine and a game: the engine declares these, and every game defines both exactly once
 * (or uses MOKSHA_GAME). The engine creates the game at startup, never naming a concrete game type.
 */
#pragma once

#include "moksha/client/app/app_config.h"

namespace ME {

class Game;

// Creates the game this executable runs. Called once, before any engine system or the window exists. config
// holds the engine defaults; the game may set its own (title, size, ...). settings.ini and the command line
// are applied after, so they win.
Game* CreateGame(const AppArgs& args, AppConfig& config);

// Destroys the game returned by CreateGame, after the engine has shut down.
void DestroyGame(Game* game);

}  // namespace ME

// Defines CreateGame/DestroyGame for a game class with a default constructor. Use once, in one .cpp file.
#define MOKSHA_GAME(GameClass, WindowTitle)                                        \
    ME::Game* ME::CreateGame(const ME::AppArgs& /*args*/, ME::AppConfig& config) { \
        config.windowTitle = WindowTitle;                                          \
        return new GameClass();                                                    \
    }                                                                              \
    void ME::DestroyGame(ME::Game* game) {                                         \
        delete game;                                                               \
    }
