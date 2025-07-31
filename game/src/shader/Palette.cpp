
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/Palette.hpp>

#include <djson/json.hpp>

#include <cassert>

namespace fornani {

Palette::Palette(std::string_view source, ResourceFinder& finder) {
	auto dir{(finder.paths.resources / fs::path{"shader/palettes/palette.json"}).string()};
	auto in_swatches = *dj::Json::from_file(dir.c_str());
	assert(!in_swatches.is_null());
	sf::RectangleShape swatch_box{};
	auto offset = sf::Vector2f{};
	m_texture.clear(sf::Color::Transparent);
	if (!m_texture.resize(sf::Vector2u{static_cast<unsigned int>(static_cast<float>(in_swatches[source].as_array().size())), 1})) { NANI_LOG_WARN(m_logger, "Failed to resize palette texture."); }
	for (auto const& entry : in_swatches[source].as_array()) {
		m_swatches.push_back(sf::Color{entry[0].as<uint8_t>(), entry[1].as<uint8_t>(), entry[2].as<uint8_t>()});
		swatch_box.setSize({1.f, 1.f});
		swatch_box.setFillColor(m_swatches.back());
		swatch_box.setPosition(offset);
		m_texture.draw(swatch_box);
		offset.x += 1.f;
	}
	m_texture.display();
}

void Palette::render(sf::RenderWindow& win) {
	auto sprite{sf::Sprite{m_texture.getTexture()}};
	win.draw(sprite);
}

sf::RenderTexture& Palette::get_texture() { return m_texture; }

} // namespace fornani
