
#pragma once

#include <fornani/setup/UserSettings.hpp>
#include <fornani/setup/Version.hpp>
#include <fornani/setup/localization/Localization.hpp>

namespace fornani {

struct AppContext {
	UserSettings settings;
	Localization localization;
	Version version;
};

} // namespace fornani
