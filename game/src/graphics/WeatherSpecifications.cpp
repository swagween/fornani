
#include <fornani/graphics/WeatherSpecifications.hpp>

namespace fornani::vfx {

WeatherSpecifications::WeatherSpecifications(std::string_view type, float const chance) : type{type}, chance{chance} {}

WeatherSpecifications::WeatherSpecifications(dj::Json const& in) {
	type = in["type"].as_string();
	chance = in["chance"].as<float>();
}

void WeatherSpecifications::serialize(dj::Json& out) {
	out["type"] = type;
	out["chance"] = chance;
}

} // namespace fornani::vfx
