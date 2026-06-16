
#pragma once

#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/setup/localization/Localization.hpp>
#include <fornani/utils/TextUtils.hpp>

namespace fornani::data {

class TextManager {
  public:
	explicit TextManager(ResourceFinder& finder, Localization& localization);

	// jsons
	dj::Json console{};
	dj::Json inspectables{};
	dj::Json basic{};
	dj::Json item{};
	dj::Json npc{};

	struct {
		FontSpec title{};
		FontSpec basic{};
		FontSpec config{};
	} fonts{};

  private:
	io::Logger m_logger{"TextManager"};
};

} // namespace fornani::data
