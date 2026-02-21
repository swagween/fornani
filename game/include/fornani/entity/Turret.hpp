
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/StateFunction.hpp>
#define TURRET_BIND(f) std::bind(&Turret::f, this)

namespace fornani {

enum class TurretType { laser, projectile };
enum class TurretPattern { constant, repeater, triggerable };
enum class TurretState { off, charging, firing, cooling_down };
enum class TurretFlags { platform };

struct TurretSettings {
	float delay{};
	int duration{};
};

class Turret : public Entity, public Flaggable<TurretFlags> {
  public:
	Turret(automa::ServiceProvider& svc, dj::Json const& in);
	Turret(automa::ServiceProvider& svc, int id, TurretType type, TurretPattern pattern, CardinalDirection dir, TurretSettings settings);

	// Copy constructor
	Turret(Turret const& other) : Entity(other), m_type(other.m_type), m_pattern(other.m_pattern), m_direction(other.m_direction), m_settings(other.m_settings) {}

	// Copy assignment
	Turret& operator=(Turret const& other) {
		if (this != &other) {
			Entity::operator=(other);
			m_type = other.m_type;
			m_pattern = other.m_pattern;
			m_direction = other.m_direction;
			m_settings = other.m_settings;
		}
		return *this;
	}

	void init();
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

	void set_position(sf::Vector2f const to) { m_position = to; }

	[[nodiscard]] auto get_position() const -> sf::Vector2f;
	[[nodiscard]] auto get_offset() const -> sf::Vector2f;

  private:
	/* animation methods */
	fsm::StateFunction state_function = std::bind(&Turret::update_off, this);
	fsm::StateFunction update_off();
	fsm::StateFunction update_charging();
	fsm::StateFunction update_firing();
	fsm::StateFunction update_cooling_down();
	bool change_state(TurretState next, std::string_view tag);
	void request(TurretState to) { m_state.desired = to; }
	struct {
		TurretState actual{};
		TurretState desired{};
	} m_state{};

	TurretType m_type{};
	TurretPattern m_pattern{};
	CardinalDirection m_direction{};
	TurretSettings m_settings{};

	sf::Vector2f m_position{};

	util::Cooldown m_rate{};
	util::Cooldown m_firing{};
};

} // namespace fornani
