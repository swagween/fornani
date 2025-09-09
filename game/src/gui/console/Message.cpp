
#include <fornani/gui/console/Message.hpp>
#include <ranges>

namespace fornani::gui {

MessageCode::MessageCode(dj::Json const& in) {
	type = static_cast<MessageCodeType>(in[0].as<int>());
	value = in[1].as<int>();
	if (in.as_array().size() > 2) {
		extras = std::vector<int>{};
		for (auto [i, extra] : std::views::enumerate(in.as_array())) {
			if (i > 1) { extras->push_back(extra.as<int>()); }
		}
	}
}

void MessageCode::serialize(dj::Json& out) const {
	auto this_code = dj::Json{};
	this_code.push_back(static_cast<int>(type));
	this_code.push_back(value);
	if (extras) {
		for (auto const& extra : extras.value()) { this_code.push_back(extra); }
	}
	out.push_back(this_code);
}

} // namespace fornani::gui
