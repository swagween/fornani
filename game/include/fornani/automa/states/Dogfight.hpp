
#pragma once

#include <fornani/automa/GameplayState.hpp>
#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/entities/vehicle/DogfightController.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/utils/Counter.hpp>
#include <fornani/weapon/Weapon.hpp>

namespace fornani {
class NPC;
}

namespace fornani::automa {

enum class DogfightFlags : std::uint8_t { complete, player_defeated };

constexpr auto dogfight_id_v = 1899;

class Dogfight final : public GameplayState {
  public:
	Dogfight(ServiceProvider& svc, player::Player& player, int room_number);
	void init(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void reload(ServiceProvider& svc, int target_state) override;
	void handle_player_death(ServiceProvider& svc, player::Player& player) override;

  private:
	void debug();

  private:
	util::BitFlags<DogfightFlags> m_flags{};
	graphics::Background m_background;
	util::Cooldown m_intro_shot;
	util::Cooldown m_bryn_shoot;
	util::Cooldown m_bryn_fire;
	util::Cooldown m_nimbus_arrive;
	std::optional<arms::Weapon> m_bryns_gun{};
	std::optional<ChampionJ5> m_champion{};
	DogfightController m_controller{};
	sf::Vector2f m_world_position{};
	util::Counter m_enemy_counter{};
	NPC* m_bryn{};
};

} // namespace fornani::automa
