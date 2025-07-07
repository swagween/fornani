
#pragma once

#include <optional>
#include <SFML/Graphics.hpp>
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Fader.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::vfx {

class Spark {
  public:
	Spark(automa::ServiceProvider& svc, sf::Vector2f pos, sf::Color color, std::string_view type);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win,sf::Vector2f cam);
	[[nodiscard]] auto done() const -> bool { return lifespan.is_complete(); }

  private:
	sf::RectangleShape box{};
	std::optional<util::Fader> fader{};
	sf::Vector2f position{};
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
