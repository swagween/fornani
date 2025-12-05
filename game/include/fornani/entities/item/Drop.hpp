
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/entities/animation/AnimatedSprite.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/utils/CircleCollider.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/StateFunction.hpp>
#define DROP_BIND(f) std::bind(&Drop::f, this)

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::player {
class Player;
}

namespace fornani::item {

enum class DropType { heart, orb, gem };
enum class DropState { neutral, shining };
enum class GemType { rhenite, sapphire };
enum class DropFlags { neutral, shining };

class Drop : public Animatable {

  public:
	Drop(automa::ServiceProvider& svc, std::string_view key, float probability, int delay_time = 0, int special_id = 0);
	void seed(float probability);
	void set_value();
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos);
	void apply_force(sf::Vector2f force);

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
	struct {
		DropState actual{};
		DropState desired{};
	} m_state{};
	void request(DropState to) { m_state.desired = to; }
	bool change_state(DropState next, anim::Parameters params);

	DropType type{};
	sf::Vector2f drop_dimensions{20.f, 20.f};
	shape::CircleCollider collider{16.f};
	std::vector<anim::Parameters> m_parameters{};
	sf::Vector2f m_sprite_dimensions{};
	util::Cooldown shine_cooldown{600};
	std::string m_label{};

	int num_sprites{}; // 2 for hearts, 4 for orbs

	Rarity rarity{};
	int value{};
	int special_id{};

	util::Cooldown lifespan{};
	util::Cooldown afterlife{}; // so sparkles remain after destruction
	util::Cooldown delay{};

	vfx::Sparkler sparkler;

	components::SteeringBehavior m_steering{};

	util::BitFlags<DropFlags> flags{};

	int cooldown_constant{2500};

	struct {
		float priceless{0.001f};
		float rare{0.01f};
		float uncommon{0.1f};
		float special{0.001f};
	} constants{};

	bool dead{};

	io::Logger m_logger{"drop"};
};

} // namespace fornani::item
