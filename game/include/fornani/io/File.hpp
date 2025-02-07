#pragma once

#include <string_view>
#include <djson/json.hpp>
#include "fornani/utils/BitFlags.hpp"

namespace fornani::io {

enum class FileFlags{ new_file };

class File {
  public:
	[[nodiscard]] auto is_new() const -> bool { return flags.test(FileFlags::new_file); }
	void write() {
		dj::Boolean flag{};
		save_data["status"]["new"] = flag;
		flags.reset(FileFlags::new_file);
	}
	int id{};
	dj::Json save_data{};
	std::string label{};
	util::BitFlags<FileFlags> flags{};
};

} // namespace lookup
