
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/io/Logger.hpp>

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include <fornani/entity/Animator.hpp>
#include <fornani/entity/Bed.hpp>
#include <fornani/entity/Chest.hpp>
#include <fornani/entity/CutsceneTrigger.hpp>
#include <fornani/entity/Destructible.hpp>
#include <fornani/entity/Enemy.hpp>
#include <fornani/entity/Inspectable.hpp>
#include <fornani/entity/Light.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/entity/Platform.hpp>
#include <fornani/entity/Portal.hpp>
#include <fornani/entity/SavePoint.hpp>
#include <fornani/entity/SwitchBlock.hpp>
#include <fornani/entity/SwitchButton.hpp>
#include <fornani/entity/TimerBlock.hpp>
#include <fornani/entity/Vine.hpp>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace fornani {
class ResourceFinder;

template <typename T>
std::unique_ptr<Entity> create_entity(automa::ServiceProvider& svc, dj::Json const& in) {
	return std::make_unique<T>(svc, in);
}

using CreateEntitySignature = decltype(&create_entity<Entity>);

class EntitySet {
  public:
	EntitySet() = default;
	EntitySet(automa::ServiceProvider& svc, ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f origin, float cell_size);
	void load(automa::ServiceProvider& svc, ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	bool save(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void clear();
	bool has_entity_at(sf::Vector2<std::uint32_t> pos, bool highlighted_only = false) const;
	bool overlaps(Entity& other) const;

	struct {
		sf::Vector2<std::uint32_t> player_start{};
		sf::Vector2<std::uint32_t> player_hot_start{};
		std::vector<std::unique_ptr<Entity>> entities{};
	} variables{};

  private:
	sf::RectangleShape player_box{};
	std::unordered_map<std::string, CreateEntitySignature> create_map;
	io::Logger m_logger{"Pioneer"};
};

} // namespace fornani
