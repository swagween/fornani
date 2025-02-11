
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace fornani::core {
namespace assets {
enum class TextureCategory { eAnimators, eApp, eBackground, eBoss, eCharacter, eCritter, eEntity, eGui, eItem, eTile, eVfx, eWardrobe, eWeapon, MAX };

enum class SoundCategory { eAmbience, eSfx, eSongs, MAX };
} // namespace assets

char const* to_string(assets::TextureCategory e);
char const* to_string(assets::SoundCategory e);

class AssetManager {
  public:
	explicit AssetManager(data::ResourceFinder const& finder);

	sf::Texture& getTexture(assets::TextureCategory category, std::string_view name);
	sf::SoundBuffer& getSound(assets::SoundCategory category, std::string_view name);

  private:
	void loadTextures(std::filesystem::path const& path);
	void loadSounds(std::filesystem::path const& path);

	data::ResourceFinder const& m_finder;

	std::unordered_map<assets::TextureCategory, std::unordered_map<std::string_view, sf::Texture>> m_textures;
	std::unordered_map<assets::SoundCategory, std::unordered_map<std::string_view, sf::SoundBuffer>> m_sounds;

	io::Logger m_logger{"core"};
};

} // namespace fornani::core
