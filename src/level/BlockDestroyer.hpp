#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {

class Map;

enum class DestroyerState { detonated };

class BlockDestroyer {
  public:
	BlockDestroyer(sf::Vector2<int> pos, int quest_id);
	void update(automa::ServiceProvider& svc, Map& map);
	[[nodiscard]] auto detonated() const -> bool { return flags.test(DestroyerState::detonated); }
  private:
	int quest_id{};
	sf::Vector2<int> position{};
	util::BitFlags<DestroyerState> flags{};
};
} // namespace world