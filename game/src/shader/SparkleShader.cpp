
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/SparkleShader.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

SparkleShader::SparkleShader(ResourceFinder& finder, sf::Vector2u size) {
	if (!sf::Shader::isAvailable()) { NANI_LOG_WARN(m_logger, "Shaders are not available."); }
	auto frag = finder.paths.resources / fs::path{"shader/shaders/sparkle.frag"};
	if (!m_shader.loadFromFile(frag, sf::Shader::Type::Fragment)) { NANI_LOG_WARN(m_logger, "Failed to load sparkle shader {}", frag.string()); }
	if (!m_texture.resize(size)) { NANI_LOG_ERROR(m_logger, "Failed to resize sparkle shader texture!"); }
	m_texture.setSmooth(false);
	m_texture.setRepeated(true);
}

void SparkleShader::finalize(float time, sf::Vector2u size, sf::Vector2f position, sf::Texture const& white_noise, sf::Texture const& diagonal_mask, sf::Color highlight, float intensity) {
	m_shader.setUniform("u_texture", sf::Shader::CurrentTexture);
	m_shader.setUniform("u_white_noise", white_noise);
	m_shader.setUniform("u_diagonal_mask", diagonal_mask);
	m_shader.setUniform("u_time", time);
	m_shader.setUniform("u_resolution", sf::Glsl::Vec2{sf::Vector2f{size}});
	m_shader.setUniform("u_position", sf::Glsl::Vec2{position});
	m_shader.setUniform("u_highlight", sf::Glsl::Vec4{highlight});
	m_shader.setUniform("u_intensity", intensity);

	m_texture.display();
}

void SparkleShader::submit(sf::RenderWindow& win, sf::Vector2f /*cam*/) {
	sf::Sprite sprite{m_texture.getTexture()};
	sprite.setScale({2.f, -2.f});
	sprite.setPosition({0.f, static_cast<float>(m_texture.getSize().y) * 2.f});
	win.draw(sprite, &m_shader);
}

void SparkleShader::draw(sf::Drawable const& target) { m_texture.draw(target); }

void SparkleShader::draw(sf::Texture const& texture) {
	sf::Sprite sprite{texture};
	m_texture.draw(sprite);
}

} // namespace fornani
