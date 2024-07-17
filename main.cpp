#include "src/setup/Game.hpp"

int main(int argc, char** argv) {
	assert(argc > 0);
	fornani::Game game{argv};
	game.run();
	//for demo testing
	//game.run(true, 200, "OVERTURNED_CANOPY_01", {100, 100});
	return 0;
}
