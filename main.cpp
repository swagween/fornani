#include "src/setup/Game.hpp"

#include <steam/steam_api.h>

#include <iostream>

int main(int argc, char** argv) {
	assert(argc > 0);

	if (SteamAPI_RestartAppIfNecessary(480)) {
		std::cout << "Re-launching through Steam." << std::endl;
		return 0;
	}
	SteamAPI_Init();

	fornani::Game game{argv};
	game.run();
	// for demo testing
	// game.run(true, 200, "OVERTURNED_CANOPY_01", {100, 100});
	return 0;
}
