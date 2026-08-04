
#pragma once

#include <fornani/io/Loader.hpp>
#include <fornani/setup/UserSettings.hpp>
#include <fornani/setup/Version.hpp>
#include <fornani/setup/localization/Localization.hpp>

namespace fornani {

struct AppContext {
	UserSettings settings;
	Localization localization;
	Version version;
	io::Loader loader;
};

} // namespace fornani
