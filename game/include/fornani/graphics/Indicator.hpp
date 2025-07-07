
#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/graphics/ColorFade.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::graphics {

enum class IndicatorType : std::uint8_t { health, orb };

class Indicator {
  public:
	explicit Indicator(automa::ServiceProvider& svc, IndicatorType type = IndicatorType::health);
	void update(automa::ServiceProvider& svc, sf::Vector2f pos = {0.f, 0.f});
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void add(float amount);
	void set_position(sf::Vector2f pos);
	void shift();
	[[nodiscard]] auto active() const -> bool { return addition_limit.running(); }
	[[nodiscard]] auto get_amount() const -> float { return variables.amount; }

  private:
	struct {
		float amount{};
	} variables{};

	IndicatorType m_type{};
	sf::Text m_label;
	util::Cooldown addition_limit{};
	util::Cooldown fadeout{};
	int fadeout_time{32};
	int addition_time{300};
	sf::Vector2f position{};
	sf::Vector2f shadow{2.f, 0.f};
	sf::Vector2f offset{-8.f, -40.f};
	vfx::ColorFade color_fade{};
	vfx::Gravitator gravitator{};
};
} // namespace fornani::graphics
