
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/gui/Gizmo.hpp>
#include <fornani/utils/Circuit.hpp>

namespace fornani::gui {

template <typename Enum>
class ZoneCollection {
	static_assert(std::is_enum_v<Enum>);

	static constexpr std::size_t Count = static_cast<std::size_t>(Enum::COUNT);

  public:
	template <typename... Zones>
	explicit ZoneCollection(Zones&&... zones)
		requires(sizeof...(Zones) == Count)
		: m_zones{std::forward<Zones>(zones)...}, m_zone_iterator(Count) {}

	InventoryZone& current() { return m_zones.at(m_zone_iterator.get()); }
	InventoryZone& at(Enum zone) { return m_zones.at(static_cast<std::size_t>(zone)); }
	InventoryZone const& at(Enum zone) const { return m_zones.at(static_cast<std::size_t>(zone)); }
	[[nodiscard]] auto contains(Enum zone) const -> bool { return static_cast<std::size_t>(zone) < Count; }

	sf::Vector2i& get_current_location() { return m_remembered_locations.at(m_zone_iterator.get()); }
	sf::Vector2i const& get_current_location() const { return m_remembered_locations.at(m_zone_iterator.get()); }
	sf::Vector2i const& get_location(Enum zone) const { return m_remembered_locations.at(static_cast<std::size_t>(zone)); }
	[[nodiscard]] auto get_zone() const -> Enum { return static_cast<Enum>(m_zone_iterator.get()); }

	void set_current_location(sf::Vector2i location) { m_remembered_locations.at(m_zone_iterator.get()) = location; }
	void set_location(Enum zone, sf::Vector2i location = {}) {
		m_remembered_locations.at(static_cast<std::size_t>(zone)) = location;
		m_zone_iterator.set(static_cast<int>(zone));
	}

	void modulate(int amount) { m_zone_iterator.modulate(amount); }

  private:
	std::array<InventoryZone, Count> m_zones;
	std::array<sf::Vector2i, Count> m_remembered_locations{};
	util::Circuit m_zone_iterator;
};

} // namespace fornani::gui
