
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../graphics/ColorFade.hpp"
#include "../../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace player {
class Indicator {
  public:
	Indicator() = default;
	Indicator(automa::ServiceProvider& svc);
	void init(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> pos = {0.f, 0.f});
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void add(float amount);
	void set_position(sf::Vector2<float> pos);
	void shift();
	[[nodiscard]] auto active() const -> bool { return !addition_limit.is_complete(); }
	[[nodiscard]] auto get_amount() const -> float { return variables.amount; }

  private:
	struct {
		float amount{};
	} variables{};

	struct {
		int id{};
	} meta{};

	sf::Text label{};
	sf::Font font{};
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
} // namespace player
