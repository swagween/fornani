
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/IWorldPositionable.hpp>
#include <fornani/utils/Shape.hpp>

namespace fornani {

enum class PortalAttributes { activate_on_contact, already_open };
enum class PortalState { activated, ready, locked, unlocked, transitioning };
enum class PortalRenderState { closed, open, locked };
enum class PortalOrientation { top, bottom, left, right, central };

class Portal : public Entity {
  public:
	Portal(automa::ServiceProvider& svc, dj::Json const& in);
	Portal(automa::ServiceProvider& svc, sf::Vector2u dimensions, bool activate_on_contact, bool already_open, int source_map_id, int destination_map_id, bool locked, int key_id);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;
	void close() { m_render_state = PortalRenderState::closed; }

	[[nodiscard]] auto get_source() const -> int { return source_id; }
	[[nodiscard]] auto get_destination() const -> int { return destination_id; }
	[[nodiscard]] auto is_activate_on_contact() const -> bool { return m_attributes.test(PortalAttributes::activate_on_contact); }
	[[nodiscard]] auto is_already_open() const -> bool { return m_attributes.test(PortalAttributes::already_open); }
	[[nodiscard]] auto is_locked() const -> bool { return m_state.test(PortalState::locked); }
	[[nodiscard]] auto is_top_or_bottom() const -> bool { return is_bottom() || is_top(); }
	[[nodiscard]] auto is_left_or_right() const -> bool { return is_left() || is_right(); }
	[[nodiscard]] auto is_bottom() const -> bool { return m_orientation == PortalOrientation::bottom; }
	[[nodiscard]] auto is_top() const -> bool { return m_orientation == PortalOrientation::top; }
	[[nodiscard]] auto is_left() const -> bool { return m_orientation == PortalOrientation::left; }
	[[nodiscard]] auto is_right() const -> bool { return m_orientation == PortalOrientation::right; }

  private:
	void change_states(automa::ServiceProvider& svc, int room_id, graphics::Transition& transition);

	shape::Shape bounding_box{};

	int source_id{};
	int destination_id{};
	int key_id{};

	PortalOrientation m_orientation{};
	PortalRenderState m_render_state{};
	util::BitFlags<PortalAttributes> m_attributes{};
	util::BitFlags<PortalState> m_state{};

	automa::ServiceProvider* m_services;
};

} // namespace fornani
