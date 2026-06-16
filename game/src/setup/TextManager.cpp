
#include <fornani/setup/TextManager.hpp>

namespace fornani::data {

TextManager::TextManager(ResourceFinder& finder, Localization& localization)
	: fonts{.title = FontSpec{sf::Font{finder.resource_path() + "/text/fonts/kongtext.ttf"}},
			.basic = FontSpec{sf::Font{finder.resource_path() + "/text/fonts/pixelFJ8pt1.ttf"}},
			.config = FontSpec{sf::Font{finder.resource_path() + "/text/fonts/Vera.ttf"}}} {
	auto code = localization.get_language_code();
	if (!code) { code.emplace("eng"); }
	if (code.value() == "ara") {
		fonts.basic.font = sf::Font{finder.resource_path() + "/text/fonts/fs-pixel-sans-unicode-regular.ttf"};
		fonts.title.font = sf::Font{finder.resource_path() + "/text/fonts/PixelAE-Bold.ttf"};
	}
	if (code.value() == "fra") {
		fonts.basic.font = sf::Font{finder.resource_path() + "/text/fonts/fs-pixel-sans-unicode-regular.ttf"};
		fonts.basic.glyph_size = 32;
		fonts.basic.line_spacing = 0.8f;
		fonts.basic.offset = {0.f, -16.f};
	}

	console = *dj::Json::from_file((finder.resource_path() + localization.get_folder_string() + "/basic.json").c_str());
	assert(!console.is_null());

	// inspectables
	inspectables = *dj::Json::from_file((finder.resource_path() + localization.get_folder_string() + "/inspectables.json").c_str());
	assert(!inspectables.is_null());

	// basic
	basic = *dj::Json::from_file((finder.resource_path() + localization.get_folder_string() + "/basic.json").c_str());
	assert(!basic.is_null());

	// basic
	item = *dj::Json::from_file((finder.resource_path() + localization.get_folder_string() + "/item.json").c_str());
	assert(!item.is_null());

	// NPCs
	// npc = *dj::Json::from_file((finder.resource_path() + localization.get_folder_string() + "/npc.json").c_str());
	// assert(!npc.is_null());

	auto npc_catalog = fs::path{finder.resource_path() + localization.get_folder_string()} / "npc";
	NANI_LOG_DEBUG(m_logger, "{}", npc_catalog.string());
	NANI_LOG_DEBUG(m_logger, "{}", std::filesystem::exists(npc_catalog));
	NANI_LOG_DEBUG(m_logger, "{}", std::filesystem::is_directory(npc_catalog));
	for (auto const& character : std::filesystem::recursive_directory_iterator(npc_catalog)) {
		if (character.path().extension() != ".json") { continue; }
		auto result = dj::Json::from_file(character.path().string());
		if (!result) { continue; }
		auto char_data = std::move(*result);
		npc[character.path().stem().string()] = char_data;
	}

	fonts.title.font.setSmooth(false);
	fonts.basic.font.setSmooth(false);
}

} // namespace fornani::data
