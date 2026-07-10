#pragma once

#include <djson/json.hpp>
#include <string_view>
#include "fornani/utils/BitFlags.hpp"

namespace fornani::io {

class File {
  public:
	[[nodiscard]] auto is_new() const -> bool { return save_data["status"]["new"].as_bool(); }
	[[nodiscard]] auto has_inspect_hint() const -> bool { return save_data["status"]["inspect_hint"].as_bool(); }
	void write() {
		save_data["status"]["new"] = false;
		save_data["status"]["inspect_hint"] = false;
	}
	int id{};
	dj::Json save_data{};
	std::string label{};
};

} // namespace fornani::io
