#pragma once

#include <memory>
#include <string_view>

#include <fornani/entity/SpecialBlock.hpp>

namespace fornani {

class SpecialBlockFactory {
  public:
	[[nodiscard]] static auto create(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view type) -> std::unique_ptr<SpecialBlock>;

	[[nodiscard]] static auto create(automa::ServiceProvider& svc, dj::Json const& in) -> std::unique_ptr<SpecialBlock>;

	[[nodiscard]] static auto create(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) -> std::unique_ptr<SpecialBlock>;

	[[nodiscard]] static auto create(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view type, std::string_view tag) -> std::unique_ptr<SpecialBlock>;

	[[nodiscard]] static auto create_expander(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view tag, ExpanderBlockSpecifications specs) -> std::unique_ptr<SpecialBlock>;
};

} // namespace fornani
