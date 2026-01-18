
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/Entity.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>

namespace fornani::entity {

enum class HPState { hit };

constexpr auto critical_threshold{0.34f};

class Health final {
  public:
	explicit Health(float max);
	void update();

	[[nodiscard]] auto get_i_quantity() const -> int { return static_cast<int>(m_quantity); }
	[[nodiscard]] auto get_quantity() const -> float { return m_quantity; }
	[[nodiscard]] auto get_i_bonus() const -> float { return static_cast<int>(bonus); }
	[[nodiscard]] auto get_bonus() const -> float { return bonus; }
	[[nodiscard]] auto get_i_capacity() const -> float { return static_cast<int>(get_capacity()); }
	[[nodiscard]] auto get_native_capacity() const -> float { return m_capacity; }
	[[nodiscard]] auto get_capacity() const -> float { return m_capacity + bonus; }
	[[nodiscard]] auto get_taken_point() const -> float { return static_cast<float>(taken_point); }
	[[nodiscard]] auto is_taken() const -> bool { return m_taken.running(); }
	[[nodiscard]] auto is_dead() const -> bool { return m_quantity <= 0.f; }
	[[nodiscard]] auto is_critical() const -> float { return get_normalized() < 0.34f; }
	[[nodiscard]] auto invincible() const -> bool { return !invincibility.is_complete(); }
	[[nodiscard]] auto full() const -> bool { return m_quantity >= m_capacity; }
	[[nodiscard]] auto empty() const -> bool { return is_dead(); }
	[[nodiscard]] auto has_bonus() const -> bool { return bonus > 0.f; }
	[[nodiscard]] auto get_normalized() const -> float { return m_quantity / get_capacity(); }

	void set_capacity(float amount, bool memory = false);
	void set_quantity(float amount);
	void add_bonus(float amount = 1.f);
	void set_invincibility(float amount);
	void heal(float amount);
	void refill();
	void inflict(float amount, bool force = false);
	void increase_capacity(float amount);
	void reset();
	void kill();

	void debug();

	util::BitFlags<HPState> flags{};
	util::Cooldown invincibility{};
	util::Cooldown restored{128};

  private:
	float m_capacity;
	float m_quantity;
	float bonus{};

	util::Cooldown m_taken;
	float taken_point{};
	int invincibility_time{};
};

} // namespace fornani::entity
