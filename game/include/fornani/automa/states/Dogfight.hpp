
#pragma once

#include <fornani/automa/GameplayState.hpp>
#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/entities/vehicle/DogfightController.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/weapon/Weapon.hpp>

namespace fornani {
class NPC;
}

namespace fornani::automa {

enum class DogfightFlags : std::uint8_t { complete };

class Dogfight final : public GameplayState {
  public:
	Dogfight(ServiceProvider& svc, player::Player& player, int room_number);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

  private:
	void debug();

  private:
	util::BitFlags<DogfightFlags> m_flags{};
	graphics::Background m_background;
	util::Cooldown m_intro_shot;
	util::Cooldown m_bryn_shoot;
	util::Cooldown m_bryn_fire;
	std::optional<arms::Weapon> m_bryns_gun{};
	std::optional<ChampionJ5> m_champion{};
	DogfightController m_controller{};
	sf::Vector2f m_world_position{};
	NPC* m_bryn{};
};

} // namespace fornani::automa
