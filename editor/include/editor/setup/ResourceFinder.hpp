
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace pi {

namespace fs = std::filesystem;

class ResourceFinder {

  public:
	fs::path find_resources(fs::path exe) {
		auto check = [](fs::path const& prefix) {
			auto path = prefix / "assets";
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

	fs::path find_directory(fs::path exe, std::string const& target) {
		auto check = [target](fs::path const& prefix) {
			auto path = prefix / target;
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

	
	struct {
		std::filesystem::path local{};	   // local assets
		std::filesystem::path levels{};	   // all level data
		std::filesystem::path resources{}; // game texture data
		std::filesystem::path out{};	   // save destination for external use
		std::string room_name{};
	} paths{};
};

} // namespace pi