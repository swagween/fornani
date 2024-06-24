
#pragma once
#include <vector>
#include <string_view>
#include <SFML/Graphics.hpp>
#include "../utils/Cooldown.hpp"

namespace automa {
struct ServiceProvider;
}

namespace vfx {

class Spark {
  public:
	Spark(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Color color, std::string_view type);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win,sf::Vector2<float> cam);
	[[nodiscard]] auto done() const -> bool { return lifespan.is_complete(); }

  private:
	sf::RectangleShape box{};
	sf::Vector2<float> position{};
	util::Cooldown lifespan{};
	int frame{};
	std::string_view type{};

	struct {
		float wobble{0.1f};
		float frequency{0.02f};
		float speed{0.1f};
		float volatility{0.0f};
	} parameters{};

	struct {
		float offset{};
		float energy{};
	} variables{};
	
};

} // namespace vfx
