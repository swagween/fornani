
#pragma once

#include <string>
#include "../../utils/Collider.hpp"
#include "../animation/Animation.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../particle/Sparkler.hpp"
#include "../../graphics/FLColor.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace item {

enum class DropType { heart, orb };
enum Rarity { common, uncommon, rare, priceless };

struct DropParameters {
	DropType type{};
	anim::Parameters animation_parameters{};
};

class Drop {

  public:
	Drop() = default;
	Drop(automa::ServiceProvider& svc, std::string_view key, float probability);
	void seed(float probability);
	void set_value();
	void set_texture(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_position(sf::Vector2<float> pos);

	void destroy_completely();
	void deactivate();

	bool is_completely_gone() const;
	bool is_inactive() const;

	shape::Collider& get_collider();
	DropType get_type() const;
	int get_value() const;

  private:
	sf::Vector2<float> drop_dimensions{16.f, 16.f};
	shape::Collider collider{};
	sf::Vector2<int> spritesheet_dimensions{};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_offset{};
	anim::Animation animation{};
	sf::Sprite sprite{};

	int num_sprites{}; // 2 for hearts, 4 for orbs

	DropParameters parameters{};
	Rarity rarity{};
	int value{};

	util::Cooldown lifespan{};
	util::Cooldown afterlife{}; // so sparkles remain after destruction

	vfx::Sparkler sparkler;

	int cooldown_constant{2500};

	float priceless_constant{0.001f};
	float rare_constant{0.01f};
	float uncommon_constant{0.1f};

	bool dead{};
};

} // namespace item
