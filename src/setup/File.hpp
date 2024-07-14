#pragma once

#include <string_view>
#include <djson/json.hpp>
#include "../utils/BitFlags.hpp"

namespace fornani {

enum class FileFlags{ new_file };

class File {
  public:
	[[nodiscard]] auto is_new() const -> bool { return flags.test(FileFlags::new_file); }
	void write() {
		save_data["status"]["new"] = (dj::Boolean) false;
		flags.reset(FileFlags::new_file);
	}
	int id{};
	dj::Json save_data{};
	std::string label{};
	util::BitFlags<FileFlags> flags{};
};

} // namespace lookup
