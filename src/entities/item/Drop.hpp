
#pragma once

#include <string>
#include "../../utils/Collider.hpp"
#include "../animation/AnimatedSprite.hpp"
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

enum class DropType { heart, orb, gem };
enum Rarity { common, uncommon, rare, priceless };
enum class GemType { rhenite, sapphire };

class Drop {

  public:
	Drop() = default;
	Drop(automa::ServiceProvider& svc, std::string_view key, float probability, int delay_time = 0);
	void seed(automa::ServiceProvider& svc, float probability);
	void set_value();
	void set_texture(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos);

	void destroy_completely();
	void deactivate();

	bool is_completely_gone() const;
	bool is_inactive() const;
	[[nodiscard]] auto get_rarity() const -> Rarity { return rarity; }
	[[nodiscard]] auto delay_over() const -> bool { return delay.is_complete(); }

	shape::Collider& get_collider();
	DropType get_type() const;
	int get_value() const;

  private:
	DropType type{};
	sf::Vector2<float> drop_dimensions{16.f, 16.f};
	shape::Collider collider{};
	sf::Vector2<int> spritesheet_dimensions{};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_offset{};
	anim::AnimatedSprite sprite{};

	int num_sprites{}; // 2 for hearts, 4 for orbs

	Rarity rarity{};
	int value{};

	util::Cooldown lifespan{};
	util::Cooldown afterlife{}; // so sparkles remain after destruction
	util::Cooldown delay{};

	vfx::Sparkler sparkler;

	int cooldown_constant{2500};

	struct {
		float priceless{0.001f};
		float rare{0.01f};
		float uncommon{0.1f};
		float special{0.001f};
	} constants{};

	bool dead{};
};

} // namespace item
