
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/Entity.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>

namespace fornani::entity {
enum class HPState { hit };
class Health final : public Entity {
	float const default_max{8.f};
	float const critical_threshold{0.34f};

  public:
	void update();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	[[nodiscard]] auto get_i_hp() const -> int { return static_cast<int>(hp); }
	[[nodiscard]] auto get_hp() const -> float { return hp; }
	[[nodiscard]] auto get_i_max() const -> float { return static_cast<int>(max_hp); }
	[[nodiscard]] auto get_max() const -> float { return max_hp; }
	[[nodiscard]] auto get_limit() const -> float { return hp_limit; }
	[[nodiscard]] auto get_taken_point() const -> float { return static_cast<float>(taken_point); }
	[[nodiscard]] auto is_dead() const -> bool { return hp <= 0.f; }
	[[nodiscard]] auto is_critical() const -> float { return get_normalized() < 0.34f; }
	[[nodiscard]] auto invincible() const -> bool { return !invincibility.is_complete(); }
	[[nodiscard]] auto full() const -> bool { return hp == max_hp; }
	[[nodiscard]] auto empty() const -> bool { return is_dead(); }
	[[nodiscard]] auto get_normalized() const -> float { return hp / max_hp; }

	void set_max(float amount, bool memory = false);
	void set_hp(float amount);
	void set_invincibility(float amount);
	void heal(float amount);
	void refill();
	void inflict(float amount, bool force = false);
	void increase_max_hp(float amount);
	void reset();

	void debug();

	util::BitFlags<HPState> flags{};
	util::Cooldown invincibility{};
	util::Cooldown restored{128};
	float taken_point{};
	int taken_time{200};

  private:
	float hp_limit{24.f};
	float max_hp{default_max};
	float hp{default_max};
	util::Counter taken{};
	int invincibility_time{};
};

} // namespace fornani::entity
