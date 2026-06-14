
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/FlatShader.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

FlatShader::FlatShader(ResourceFinder& finder) {
	if (!sf::Shader::isAvailable()) { NANI_LOG_WARN(m_logger, "Shaders are not available."); }
	auto vert = finder.paths.resources / fs::path{"shader/shaders/flat.vert"};
	auto frag = finder.paths.resources / fs::path{"shader/shaders/flat.frag"};
	if (!m_shader.loadFromFile(vert, frag)) { NANI_LOG_WARN(m_logger, "Failed to load flat shader {}", frag.string()); }
}

void FlatShader::finalize(sf::Color color) { m_shader.setUniform("u_color", sf::Glsl::Vec4(color)); }

void FlatShader::submit(sf::RenderWindow& win, sf::Sprite const& sprite) {
	m_shader.setUniform("u_texture", sprite.getTexture());
	sf::RenderStates states;
	states.shader = &m_shader;
	states.blendMode = sf::BlendAlpha;
	win.draw(sprite, states);
}

} // namespace fornani
