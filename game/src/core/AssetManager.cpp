#include "fornani/core/AssetManager.hpp"

namespace fornani::core {

AssetManager::AssetManager(ResourceFinder const& finder) {

	// doing stuff the new way will be done up here. the goal is to slowly but surely get rid of the bottom

	namespace fs = std::filesystem;
	auto p_folder{fs::path{"image"}};
	auto p_app{p_folder / fs::path{"app"}};
	auto image_dir = fs::path{finder.resource_path()} / "image";

	// null texture for lookup failures
	if (!m_null_texture.loadFromFile(finder.resource_path() / p_app / fs::path{"null.png"})) { NANI_LOG_WARN(m_logger, "Failed to load texture."); };

	// recursively load textures
	for (auto const& image_genre : fs::recursive_directory_iterator(image_dir)) {
		if (!image_genre.is_directory()) { continue; }
		for (auto const& image : fs::recursive_directory_iterator(image_genre)) {
			if (image.path().extension() != ".png") { continue; }
			auto image_str = image.path().filename().string();
			m_textures.insert({image_str.substr(0, image_str.find('.')), sf::Texture{image.path()}});
		}
	}

	// load palettes
	auto palette_dir = image_dir / "palette";
	auto pal_dir{(finder.paths.resources / fs::path{"shader/palettes/palette.json"}).string()};
	auto swatches = dj::Json{};
	for (auto const& palette : fs::recursive_directory_iterator(palette_dir)) {
		if (palette.path().extension() != ".png") { continue; }
		auto filename = palette.path().filename();
		filename.replace_extension();
		auto filestr = filename.string();
		auto lookup = filestr.substr(0, std::distance(filestr.begin(), std::find(filestr.begin(), filestr.end(), '.')));
		auto tag = filestr.substr(8, filestr.size());
		auto& palette_tex = get_texture(lookup);
		auto palette_data = palette_tex.copyToImage();
		auto palette_image_data = palette_data.getPixelsPtr();
		int width = palette_data.getSize().x;
		int height = palette_data.getSize().y;
		int total_array_size = width * height * 4;
		for (int i = 0; i < total_array_size; ++i) {
			if (i % 4 == 0) {
				auto next = dj::Json::empty_array();
				next.push_back(palette_image_data[i]);
				next.push_back(palette_image_data[i + 1]);
				next.push_back(palette_image_data[i + 2]);
				swatches[tag].push_back(next);
			}
		}
	}
	if (!swatches.to_file(pal_dir)) { NANI_LOG_ERROR(m_logger, "Failed to save palette data."); }
}

sf::Texture const& AssetManager::get_texture(std::string const& label) { return m_textures.contains(label) ? m_textures.at(label) : m_null_texture; }

sf::Texture& AssetManager::get_texture_modifiable(std::string const& label) { return m_textures.contains(label) ? m_textures.at(label) : m_null_texture; }

sf::Texture const& AssetManager::get_tileset(std::string const& label) { return get_texture(label + "_tiles"); }

sf::Texture const& AssetManager::get_npc_texture(std::string const& label) { return get_texture("npc_" + label); }

} // namespace fornani::core
