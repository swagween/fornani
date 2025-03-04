#include "fornani/core/AssetManager.hpp"

namespace fornani::core {

AssetManager::AssetManager(data::ResourceFinder const& finder) {

	// doing stuff the new way will be done up here. the goal is to slowly but surely get rid of the bottom

	namespace fs = std::filesystem;
	auto p_folder{fs::path{"image"}};
	auto p_gui{p_folder / fs::path{"gui"}};
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
}

sf::Texture const& AssetManager::get_texture(std::string const& label) { return m_textures.contains(label) ? m_textures.at(label) : m_null_texture; }

sf::Texture& AssetManager::get_texture_modifiable(std::string const& label) { return m_textures.contains(label) ? m_textures.at(label) : m_null_texture; }

sf::Texture const& AssetManager::get_tileset(std::string const& label) { return get_texture(label + "_tiles"); }

sf::Texture const& AssetManager::get_npc_texture(std::string const& label) { return get_texture("npc_" + label); }

} // namespace fornani::core
