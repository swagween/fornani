#include "src/setup/Game.hpp"

int main(int argc, char** argv) {
	assert(argc > 0);
	fornani::Game game{argv};
	game.run();
	ImGui::SFML::Shutdown();
	return 0;
}
