
#include <fornani/gui/console/Message.hpp>
#include <ranges>

namespace fornani::gui {

MessageCode::MessageCode(dj::Json const& in) {
	type = static_cast<MessageCodeType>(in[0].as<int>());
	value = in[1].as<int>();
	if (in.as_array().size() > 2) {
		extras = std::vector<int>{};
		for (auto [i, extra] : std::views::enumerate(in.as_array())) {
			if (i > 2) { extras->push_back(extra.as<int>()); }
		}
	}
}

} // namespace fornani::gui
