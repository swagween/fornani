#pragma once
#include <SFML/Graphics.hpp>
#include "../Entity.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../utils/Counter.hpp"

namespace entity {

	enum HPState { light = 0, filled, taken, gone };

	struct Heart {
	HPState state{};
	};

class Health : public Entity {
	float const default_max{8.f};
  public:
	void update();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto get_hp() const -> float { return hp; }
	[[nodiscard]] auto get_max() const -> float { return max_hp; }
	[[nodiscard]] auto get_taken_point() const -> float { return taken_point; }
	[[nodiscard]] auto get_state(int index) const -> int { return (int)health_states.at(index).state; }
	[[nodiscard]] auto is_dead() const -> bool { return hp <= 0.f; }
	[[nodiscard]] auto invincible() const -> bool { return !invincibility.is_complete(); }
	void set_max(float amount);
	void set_hp(float amount);
	void set_invincibility(float amount);
	void heal(float amount);
	void inflict(float amount);
	void reset();

	util::Cooldown invincibility{};

  private:
	float max_hp{default_max};
	float hp{default_max};
	int taken_point{};
	util::Counter taken{};
	util::Cooldown restored{};
	int invincibility_time{};
	std::vector<Heart> health_states{};
};

} // namespace entity