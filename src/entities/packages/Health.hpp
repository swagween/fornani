#pragma once
#include <SFML/Graphics.hpp>
#include "../Entity.hpp"
#include "../../utils/Cooldown.hpp"

namespace entity {

class Health : public Entity {
	float const default_max{8.f};
	int const default_invincibility{0};
  public:
	void update();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto get_hp() const -> float { return hp; }
	[[nodiscard]] auto get_max() const -> float { return max_hp; }
	[[nodiscard]] auto is_dead() const -> bool { return hp <= 0.f; }
	[[nodiscard]] auto invincible() const -> bool { return !invincibility.is_complete(); }
	void set_max(float amount);
	void set_hp(float amount);
	void heal(float amount);
	void inflict(float amount);
	void reset();

  //private:
	float max_hp{default_max};
	float hp{default_max};
	util::Cooldown invincibility{};
	int invincibility_time{default_invincibility};
};

} // namespace entity