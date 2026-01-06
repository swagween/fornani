#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>
#include <array>
#include <unordered_map>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::util {

class Fader {
  public:
	Fader(automa::ServiceProvider& svc, int time, std::string_view color);
	void update();
	sf::Sprite& get_sprite() { return sprite; }

  private:
	int order{};
	int lookup{};
	int time{};
	int interval{};
	Counter progress{};
	sf::Sprite sprite;
	Cooldown timer{};
};

} // namespace fornani::util
