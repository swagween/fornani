
#pragma once

#include <string>
#include "fornani/utils/CircleCollider.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/particle/Sparkler.hpp"
#include "fornani/utils/StateFunction.hpp"
#define DROP_BIND(f) std::bind(&Drop::f, this)

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
enum class DropFlags { neutral, shining };

class Drop {

  public:
	Drop(automa::ServiceProvider& svc, std::string_view key, float probability, int delay_time = 0, int special_id = 0);
	void seed(automa::ServiceProvider& svc, float probability);
	void set_value();
	void set_texture(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos);
	void apply_force(sf::Vector2<float> force);

	void destroy_completely();
	void deactivate();

	bool is_completely_gone() const;
	bool is_inactive() const;
	[[nodiscard]] auto collides_with(shape::Shape& shape) const -> bool { return collider.collides_with(shape); }
	[[nodiscard]] auto get_rarity() const -> Rarity { return rarity; }
	[[nodiscard]] auto delay_over() const -> bool { return delay.is_complete(); }

	DropType get_type() const;
	int get_value() const;

	// animation state machine
	fsm::StateFunction state_function = std::bind(&Drop::update_neutral, this);
	fsm::StateFunction update_neutral();
	fsm::StateFunction update_shining();

  private:
	DropType type{};
	sf::Vector2<float> drop_dimensions{20.f, 20.f};
	shape::CircleCollider collider{16.f};
	sf::Vector2<int> spritesheet_dimensions{};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_offset{};
	anim::AnimatedSprite sprite;
	util::Cooldown shine_cooldown{600};

	int num_sprites{}; // 2 for hearts, 4 for orbs

	Rarity rarity{};
	int value{};
	int special_id{};

	util::Cooldown lifespan{};
	util::Cooldown afterlife{}; // so sparkles remain after destruction
	util::Cooldown delay{};

	vfx::Sparkler sparkler;

	util::BitFlags<DropFlags> flags{};

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
