
#pragma once

#include <djson/json.hpp>

namespace fornani::vfx {

struct WeatherSpecifications {
	WeatherSpecifications(std::string_view type, float const chance);
	WeatherSpecifications(dj::Json const& in);
	void serialize(dj::Json& out);
	std::string type{};
	float chance{};
};

} // namespace fornani::vfx
