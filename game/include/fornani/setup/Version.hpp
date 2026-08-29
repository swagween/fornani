#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/app/build_version.hpp>

namespace fornani {

class Version {
  public:
	Version() : title{"fornani"}, channel{fornani::version::channel}, major{fornani::version::major}, minor{fornani::version::minor}, patch{fornani::version::patch} {}
	[[nodiscard]] std::string version() const { return "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch); }
	[[nodiscard]] auto get_title() const -> std::string_view { return title; }
	[[nodiscard]] std::string long_title() const { return title.data() + std::string{" ("} + channel.data() + std::string{" "} + version().data() + std::string{")"}; }
	[[nodiscard]] std::string version_title() const { return channel.data() + std::string{" "} + version().data(); }

  private:
	std::string title{};
	std::string channel{};
	int major{};
	int minor{};
	int patch{};
};

} // namespace fornani
