
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/RectPath.hpp>

namespace fornani {

enum class HaunchIntroFlags { done, over };

class HaunchIntro final : public Cutscene {
  public:
	explicit HaunchIntro(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::Cooldown m_intro;
	util::Cooldown m_outro;
	util::Cooldown m_hulmet_spawn_delay;
	util::FloatCounter m_truck_shake;
	float m_truck_y_offset{};

	// cinematic elements
	Animatable m_army_truck_body;
	Animatable m_army_truck_undercarriage;
	sf::Vector2f m_exhaust_socket{};
	util::RectPath m_truck_path;

	util::BitFlags<HaunchIntroFlags> m_flags{};
};

} // namespace fornani
