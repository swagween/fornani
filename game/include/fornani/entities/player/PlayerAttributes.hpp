
#pragma once

#include <djson/json.hpp>

namespace fornani::player {

struct PlayerAttributes {
	PlayerAttributes() = default;
	PlayerAttributes(dj::Json const& in);
	void serialize(dj::Json const& out);
	float luck{1.f};
};

} // namespace fornani::player
