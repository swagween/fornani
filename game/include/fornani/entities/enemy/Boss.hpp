
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace fornani::enemy {

enum class BossFlags : std::uint8_t { start_battle, battle_mode, second_phase, post_death, end_battle, defeated_player, ignore_oob, facing_player };

class Boss : public Enemy, public Flaggable<BossFlags> {
  public:
	Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void start_battle();
	void end_battle();

	[[nodiscard]] auto battle_mode() const -> bool { return has_flag_set(BossFlags::battle_mode); }
	[[nodiscard]] auto second_phase() const -> bool { return has_flag_set(BossFlags::second_phase); }
	[[nodiscard]] auto quarter_health() const -> bool { return health.get_quantity() < health.get_capacity() * 0.25f; }
	[[nodiscard]] auto is_staggered() -> bool;
	[[nodiscard]] auto is_facing_player() -> bool;

  protected:
	std::shared_ptr<Slot const> slot{std::make_shared<Slot const>()};
	automa::ServiceProvider* p_services;
	world::Map* p_map;
	gui::BossHealth p_health_bar;

	std::vector<float> p_stagger_points;

  private:
	util::Counter m_oob_counter{};
};

} // namespace fornani::enemy
