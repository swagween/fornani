
#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/Cooldown.hpp"
#include "fornani/graphics/ColorFade.hpp"
#include "fornani/particle/Gravitator.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::player {
enum class IndicatorType : uint8_t { health, orb };
class Indicator {
  public:
	explicit Indicator(automa::ServiceProvider& svc);
	void init(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> pos = {0.f, 0.f});
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void add(float amount);
	void set_position(sf::Vector2<float> pos);
	void shift();
	[[nodiscard]] auto active() const -> bool { return addition_limit.running(); }
	[[nodiscard]] auto get_amount() const -> float { return variables.amount; }

  private:
	struct {
		float amount{};
	} variables{};

	IndicatorType type{};
	sf::Text label;
	util::Cooldown addition_limit{};
	util::Cooldown fadeout{};
	int fadeout_time{32};
	int addition_time{300};
	sf::Vector2<float> position{};
	sf::Vector2<float> shadow{2.f, 0.f};
	sf::Vector2<float> offset{-8.f, -40.f};
	vfx::ColorFade color_fade{};
	vfx::Gravitator gravitator{};
};
} // namespace fornani::player
