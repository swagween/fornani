
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class HaunchEscapeFlags { done, over };

class HaunchEscape final : public Cutscene {
  public:
	explicit HaunchEscape(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

	[[nodiscard]] auto round_two() const -> bool { return progress >= 20; }

  private:
	util::Cooldown m_intro;
	util::Cooldown m_dynamite;
	util::Cooldown m_bomb_tick;
	util::Cooldown m_heroes_exit;
	util::Cooldown m_outro;
	util::Cooldown m_champion_entry;
	util::Cooldown m_player_jump;
	std::optional<ChampionJ5> m_champion;
	components::SteeringComponent m_player_steering{};

	util::BitFlags<HaunchEscapeFlags> m_flags{};

	sf::Vector2f m_exit_point{};
};

} // namespace fornani
