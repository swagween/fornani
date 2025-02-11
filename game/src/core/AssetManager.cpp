#include "fornani/core/AssetManager.hpp"

namespace fornani::core {

char const* to_string(assets::TextureCategory e) {
	switch (e) {
	case assets::TextureCategory::eAnimators: return "animators";
	case assets::TextureCategory::eApp: return "app";
	case assets::TextureCategory::eBackground: return "background";
	case assets::TextureCategory::eBoss: return "boss";
	case assets::TextureCategory::eCharacter: return "character";
	case assets::TextureCategory::eCritter: return "critter";
	case assets::TextureCategory::eEntity: return "entity";
	case assets::TextureCategory::eGui: return "gui";
	case assets::TextureCategory::eItem: return "item";
	case assets::TextureCategory::eTile: return "tile";
	case assets::TextureCategory::eVfx: return "vfx";
	case assets::TextureCategory::eWardrobe: return "wardrobe";
	case assets::TextureCategory::eWeapon: return "weapon";
	default: return "unknown";
	}
}

char const* to_string(assets::SoundCategory e) {
	switch (e) {
	case assets::SoundCategory::eAmbience: return "ambience";
	case assets::SoundCategory::eSfx: return "sfx";
	case assets::SoundCategory::eSongs: return "songs";
	default: return "unknown";
	}
}

AssetManager::AssetManager(data::ResourceFinder const& finder) : m_finder(finder) {
	namespace fs = std::filesystem;

	auto const img_path = finder.paths.resources / "image";
	auto const aud_path = finder.paths.resources / "audio";

	loadTextures(img_path);
	// loadSounds(aud_path);
}
void AssetManager::loadTextures(std::filesystem::path const& path) {
	namespace fs = std::filesystem;

	for (auto i = 0; i < static_cast<int>(assets::TextureCategory::MAX); ++i) {
		auto const current_category = static_cast<assets::TextureCategory>(i);
		fs::path category_dir = path / to_string(current_category);

		if (!exists(category_dir)) {
			NANI_LOG_ERROR(m_logger, "Texture category directory does not exist: {}", category_dir.string());
			continue;
		}

		for (auto const& entry : fs::directory_iterator(category_dir)) {
			if (entry.is_regular_file()) {
				std::string filePath = entry.path().string();
				std::string fileName = entry.path().filename().string();

				if (std::string fileExt = entry.path().extension().string(); fileExt == ".png") {
					if (sf::Texture texture; texture.loadFromFile(filePath)) {
						m_textures[current_category][fileName] = std::move(texture);
						m_logger.info("Loaded texture: {}", fileName);
					} else {
						m_logger.error("Failed to load texture: {}", fileName);
					}
				}
			}
		}
	}
}

// TODO: Make sound work recursively down the folder tree.
void AssetManager::loadSounds(std::filesystem::path const& path) {
	namespace fs = std::filesystem;

	for (auto i = 0; i < static_cast<int>(assets::SoundCategory::MAX); ++i) {
		auto const current_category = static_cast<assets::SoundCategory>(i);
		fs::path category_dir = path / to_string(current_category);

		if (!exists(category_dir)) {
			NANI_LOG_ERROR(m_logger, "Texture category directory does not exist: {}", category_dir.string());
			continue;
		}

		for (auto const& entry : fs::directory_iterator(category_dir)) {
			if (entry.is_regular_file()) {
				std::string filePath = entry.path().string();
				std::string fileName = entry.path().filename().string();

				if (std::string fileExt = entry.path().extension().string(); fileExt == ".png") {
					if (sf::SoundBuffer buffer; buffer.loadFromFile(filePath)) {
						m_sounds[current_category][fileName] = std::move(buffer);
						m_logger.info("Loaded texture: {}", fileName);
					} else {
						m_logger.error("Failed to load texture: {}", fileName);
					}
				}
			}
		}
	}
}

} // namespace fornani::core
