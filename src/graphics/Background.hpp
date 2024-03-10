
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <unordered_map>
#include "../setup/ServiceLocator.hpp"

namespace bg {

static size_t const num_layers{5};
static int const scroll_size{1920};

enum class BACKDROP {
	BG_NULL,
	BG_DUSK,
	BG_SUNRISE,
	BG_OPEN_SKY,
	BG_ROSY_HAZE,
	BG_NIGHT,
	BG_DAWN,
	BG_OVERCAST,

	BG_SLIME,
	BG_BLACK,
	BG_NAVY,
	BG_DIRT,
	BG_GEAR,
	BG_LIBRARY,
	BG_GRANITE,
	BG_RUINS,
	BG_CREVASSE,
	BG_DEEP,
	BG_GROVE
};



struct BackgroundBehavior {
	int used_layers{};
	float scroll_speed{};
	bool scrolling{};
	float parallax_multiplier{0.1};
};

inline std::unordered_map<int, BackgroundBehavior> bg_behavior_lookup{
	{-1, BackgroundBehavior{0, 0.0f, false}},	   {0, BackgroundBehavior{5, 5.f, true}},		  {1, BackgroundBehavior{5, 3.f, true}},		 {2, BackgroundBehavior{5, 0.4f, true}},		{3, BackgroundBehavior{5, 3.1f, true}},
	{4, BackgroundBehavior{5, 3.f, true}},		   {5, BackgroundBehavior{5, 4.f, true}},		  {6, BackgroundBehavior{5, 4.f, true}},		 {7, BackgroundBehavior{1, 0.0f, false, 0.2}},	{8, BackgroundBehavior{1, 0.0f, false, 0.0}},
	{9, BackgroundBehavior{1, 0.0f, false, 0.0}},  {10, BackgroundBehavior{1, 0.0f, false, 0.8}}, {11, BackgroundBehavior{1, 0.0f, false, 0.8}}, {12, BackgroundBehavior{1, 0.0f, false, 0.8}}, {13, BackgroundBehavior{1, 0.0f, false, 0.2}},
	{14, BackgroundBehavior{1, 0.0f, false, 0.2}}, {15, BackgroundBehavior{1, 0.0f, false, 0.2}}, {16, BackgroundBehavior{1, 0.0f, false, 0.2}}, {17, BackgroundBehavior{1, 0.0f, false, 0.2}}};

class Background {

  public:
	Background() = default;
	Background(BackgroundBehavior b, int bg_id, services::ServiceLocator& svc);


	void update(cam::Camera& camera, services::ServiceLocator& svc);
	void render(sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim);

	std::array<sf::Sprite, num_layers> sprites{};
	BackgroundBehavior behavior{};

	std::vector<components::PhysicsComponent> physics{};

  private:
	std::unordered_map<int, sf::Texture&> get_backdrop_texture{};
};

} // namespace bg
