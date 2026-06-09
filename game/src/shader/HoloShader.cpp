
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/HoloShader.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

HoloShader::HoloShader(ResourceFinder& finder) {
	if (!sf::Shader::isAvailable()) { NANI_LOG_WARN(m_logger, "Shaders are not available."); }
	auto frag = finder.paths.resources / fs::path{"shader/shaders/holo.frag"};
	if (!m_shader.loadFromFile(frag, sf::Shader::Type::Fragment)) { NANI_LOG_WARN(m_logger, "Failed to load holo shader {}", frag.string()); }
	// auto vert = finder.paths.resources / fs::path{"shader/shaders/holo.vert"};
	// if (!m_shader.loadFromFile(vert, frag)) { NANI_LOG_WARN(m_logger, "Failed to load shader {}", frag.string()); }
}

void HoloShader::finalize(float time, sf::Color highlight, sf::Color shadow, float glow) {
	m_shader.setUniform("u_time", time);
	m_shader.setUniform("u_highlight", sf::Glsl::Vec4(highlight));
	m_shader.setUniform("u_shadow", sf::Glsl::Vec4(shadow));
	m_shader.setUniform("u_glowStrength", glow);
}

void HoloShader::submit(sf::RenderWindow& win, sf::Sprite const& sprite) {
	m_shader.setUniform("u_textureSize", sf::Glsl::Vec2(sprite.getTexture().getSize()));
	win.draw(sprite, &m_shader);
}

} // namespace fornani
