#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include "fornani/io/Logger.hpp"
#include "fornani/setup/ResourceFinder.hpp"

namespace fornani {

class Version {
  public:
	Version(dj::Json& info, data::ResourceFinder& finder) {
		// TODO: Move this to a cmake config file instead of looking for a json file.
		// load version info
		info = dj::Json::from_file((finder.resource_path() + "/data/config/version.json").c_str());
		assert(!info.is_null());

		title = info["title"].as_string();
		build = info["build"].as_string();
		major = info["version"]["major"].as<int>();
		minor = info["version"]["minor"].as<int>();
		hotfix = info["version"]["hotfix"].as<int>();
	}
	~Version() { NANI_LOG_DEBUG(m_logger, "Version destroyed"); }
	[[nodiscard]] std::string version() const { return "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(hotfix); }
	[[nodiscard]] std::string long_title() const { return title.data() + std::string{" ("} + build.data() + std::string{" "} + version().data() + std::string{")"}; }
	[[nodiscard]] std::string version_title() const { return build.data() + std::string{" "} + version().data(); }

  private:
	std::string title{};
	std::string build{};
	int major{};
	int minor{};
	int hotfix{};
	io::Logger m_logger{"core"};
};

} // namespace fornani
