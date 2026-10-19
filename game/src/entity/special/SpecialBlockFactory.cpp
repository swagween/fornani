
#include <fornani/entity/special/ExpanderBlock.hpp>
#include <fornani/entity/special/SpecialBlockFactory.hpp>

namespace fornani {

auto SpecialBlockFactory::create(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view type) -> std::unique_ptr<SpecialBlock> {
	if (type == "expander") { return std::make_unique<ExpanderBlock>(svc, dimensions, id, type); }

	return std::make_unique<SpecialBlock>(svc, dimensions, id, type);
}

auto SpecialBlockFactory::create(automa::ServiceProvider& svc, dj::Json const& in) -> std::unique_ptr<SpecialBlock> {
	auto const& type = in["type"].as_string();

	if (type == "expander") { return std::make_unique<ExpanderBlock>(svc, in); }

	return std::make_unique<SpecialBlock>(svc, in);
}

auto SpecialBlockFactory::create(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) -> std::unique_ptr<SpecialBlock> {
	auto const& type = in["type"].as_string();

	if (type == "expander") { return std::make_unique<ExpanderBlock>(svc, map, in); }

	return std::make_unique<SpecialBlock>(svc, map, in);
}

} // namespace fornani
