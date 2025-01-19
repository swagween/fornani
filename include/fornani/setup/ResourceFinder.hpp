
#pragma once

#include <../../../out/vs22-internal/_deps/sfml-src/include/SFML/Graphics.hpp>
#include <../../../out/vs22-internal/_deps/djson-src/include/djson/json.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace data {

namespace fs = std::filesystem;

class ResourceFinder {

  public:
	ResourceFinder() = default;
	ResourceFinder(char** argv) {
		set_resource_path(argv);
		set_scene_path(argv);
	}

	void set_resource_path(char** argv) { resource_path = find_resources(argv[0]).string(); }
	void set_scene_path(char** argv) { scene_path = find_scenes(argv[0]).string(); }

	fs::path find_resources(fs::path exe) {
		auto check = [](fs::path const& prefix) {
			auto path = prefix / "resources";
			if (fs::is_directory(path)) { return path; }
			return fs::path{};
		};
		while (!exe.empty()) {
			if (auto ret = check(exe); !ret.empty()) { return ret; }
			auto parent = exe.parent_path();
			if (exe == parent) { break; }
			exe = std::move(parent);
		}
		return {};
	}
	fs::path find_scenes(fs::path exe) {
		auto check = [](fs::path const& prefix) {
			auto path = prefix / "resources/scenes";
			if (fs::is_directory(path)) { return path; }
			return fs::path{};
		};
		while (!exe.empty()) {
			if (auto ret = check(exe); !ret.empty()) { return ret; }
			auto parent = exe.parent_path();
			if (exe == parent) { break; }
			exe = std::move(parent);
		}
		return {};
	}

	std::string resource_path{};
	std::string scene_path{};
};

} // namespace data