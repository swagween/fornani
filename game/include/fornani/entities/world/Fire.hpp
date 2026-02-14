#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/physics/Shape.hpp>
#include <memory>
#include <optional>

namespace fornani::player {
class Player;
}

namespace fornani::gui {
class Console;
}

namespace fornani::world {

class Map;

class Fire : public Animatable {
  public:
	Fire(automa::ServiceProvider& svc, sf::Vector2f position, int lookup);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map, std::optional<std::unique_ptr<gui::Console>>& console);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

  private:
	int size{};
	shape::Shape bounding_box{};
	vfx::Sparkler sparkler;
	sf::Vector2f sprite_offset{};
};

} // namespace fornani::world
