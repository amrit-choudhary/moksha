/**
 * The link between the engine and a game: the engine declares these, and every game defines both exactly once.
 * The engine creates the game at startup and destroys it at shutdown; it never names a concrete game type.
 */
#pragma once

namespace ME {

class Game;

// Creates the game this executable runs. Called once, before any engine system is initialized.
Game* CreateGame();

// Destroys the game returned by CreateGame, after the engine has shut down.
void DestroyGame(Game* game);

}  // namespace ME
