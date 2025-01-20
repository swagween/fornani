#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
namespace fornani {

class Version {
  public:
	std::string version() const { return "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(hotfix); }
	std::string long_title() const { return title.data() + std::string{" ("} + build.data() + std::string{" "} + version().data() + std::string{")"}; }
	std::string version_title() const { return build.data() + std::string{" "} + version().data(); }
	std::string_view title{};
	std::string_view build{};
	int major{};
	int minor{};
	int hotfix{};

};
} // namespace fornani