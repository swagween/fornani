
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/HazeShader.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

HazeShader::HazeShader(ResourceFinder& finder, sf::Vector2u size, float parallax) : m_parallax{parallax} {
	if (!sf::Shader::isAvailable()) { NANI_LOG_WARN(m_logger, "Shaders are not available."); }
	auto frag = finder.paths.resources / fs::path{"shader/shaders/haze.frag"};
	if (!m_shader.loadFromFile(frag, sf::Shader::Type::Fragment)) { NANI_LOG_WARN(m_logger, "Failed to load haze shader {}", frag.string()); }
	if (!m_texture.resize(size)) { NANI_LOG_ERROR(m_logger, "Failed to resize haze shader texture!"); }
	m_texture.setSmooth(false);
	m_texture.setRepeated(true);
}

void HazeShader::finalize(float time, sf::Vector2u size, sf::Color highlight, sf::Color shadow, sf::Vector2f cam) {
	m_shader.setUniform("uTime", time);
	m_shader.setUniform("uResolution", sf::Glsl::Vec2(size));
	m_shader.setUniform("uCamera", sf::Glsl::Vec2(cam));
	m_shader.setUniform("uParallax", m_parallax);
	auto float_hr = highlight.r / 255.f;
	auto float_hg = highlight.g / 255.f;
	auto float_hb = highlight.b / 255.f;
	auto float_sr = shadow.r / 255.f;
	auto float_sg = shadow.g / 255.f;
	auto float_sb = shadow.b / 255.f;
	m_shader.setUniform("uHighlightColor", sf::Glsl::Vec3(float_hr, float_hg, float_hb));
	m_shader.setUniform("uShadowColor", sf::Glsl::Vec3(float_sr, float_sg, float_sb));
}

void HazeShader::submit(sf::RenderWindow& win, sf::Drawable const& drawable, sf::Vector2f cam) {
	m_texture.clear(sf::Color::Transparent);
	sf::RectangleShape quad(sf::Vector2f(m_texture.getSize()));
	m_texture.draw(quad, &m_shader);
	m_texture.display();
	auto sprite = sf::Sprite{m_texture.getTexture()};
	sf::RenderStates states;
	states.blendMode = sf::BlendMode(sf::BlendMode::Factor::One, sf::BlendMode::Factor::OneMinusSrcColor, sf::BlendMode::Equation::Add);
	win.draw(sprite, states);
}

} // namespace fornani
