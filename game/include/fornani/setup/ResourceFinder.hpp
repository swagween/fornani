
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <filesystem>
#include <string>

namespace fornani {

namespace fs = std::filesystem;

class ResourceFinder {
  public:
	explicit ResourceFinder(char** argv);
	fs::path region_and_room() const;
	fs::path find_directory(fs::path const& exe, fs::path const& target);

	[[nodiscard]] auto resource_path() const -> std::string { return paths.resources.string(); }

	struct {
		fs::path editor{};	  // local assets
		fs::path levels{};	  // all level data
		fs::path resources{}; // game texture data
		fs::path out{};		  // save destination for external use
		fs::path screenshots{};
		std::string region{};
		std::string room_name{};
	} paths{};
};

} // namespace fornani
