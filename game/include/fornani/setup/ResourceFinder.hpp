
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace data {

namespace fs = std::filesystem;

class ResourceFinder {
  public:
	ResourceFinder(char** argv) {
		paths.resources = find_directory(argv[0], "resources");
		paths.editor = find_directory(argv[0], "resources/editor");
		paths.levels = find_directory(argv[0], "resources/level");
		paths.out = find_directory(argv[0], "resources/editor/export");
	}

	fs::path find_directory(fs::path const& exe, std::string const& target) {
		auto execpy = fs::path{exe};
		auto check = [target](fs::path const& prefix) {
			auto path = prefix / target;
			if (fs::is_directory(path)) { return path; }
			return fs::path{};
		};
		while (!execpy.empty()) {
			if (auto ret = check(execpy); !ret.empty()) { return ret; }
			auto parent = execpy.parent_path();
			if (execpy == parent) { break; }
			execpy = std::move(parent);
		}
		return {};
	}

	[[nodiscard]] auto resource_path() const -> std::string { return paths.resources.string(); }

	struct {
		fs::path editor{};	   // local assets
		fs::path levels{};	   // all level data
		fs::path resources{}; // game texture data
		fs::path out{};	   // save destination for external use
		std::string room_name{};
	} paths{};
};

} // namespace data
