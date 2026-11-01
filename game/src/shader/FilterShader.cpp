
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/FilterShader.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Math.hpp>

namespace fornani {

FilterShader::FilterShader(ResourceFinder& finder, sf::Vector2u size) {
	if (!sf::Shader::isAvailable()) { NANI_LOG_WARN(m_logger, "Shaders are not available."); }
	auto frag = finder.paths.resources / fs::path{"shader/shaders/filter.frag"};
	if (!m_shader.loadFromFile(frag, sf::Shader::Type::Fragment)) { NANI_LOG_WARN(m_logger, "Failed to load haze shader {}", frag.string()); }
	if (!m_texture.resize(size)) { NANI_LOG_ERROR(m_logger, "Failed to resize haze shader texture!"); }
	m_texture.setSmooth(false);
	m_texture.setRepeated(true);
}

void FilterShader::finalize(float time, sf::Vector2u size, sf::Vector2f position, sf::Color highlight, float intensity, sf::Texture const* overlay, float radius) {
	m_shader.setUniform("uTime", time);
	m_shader.setUniform("uResolution", sf::Glsl::Vec2(size));

	auto const float_hr = highlight.r / 255.f;
	auto const float_hg = highlight.g / 255.f;
	auto const float_hb = highlight.b / 255.f;

	m_shader.setUniform("uHighlightColor", sf::Glsl::Vec3(float_hr, float_hg, float_hb));
	m_shader.setUniform("uIntensity", intensity);
	m_shader.setUniform("uCenter", sf::Glsl::Vec2{position});
	m_shader.setUniform("uVignette", 1.0f);
	m_shader.setUniform("uRadius", radius);

	m_shader.setUniform("uUseOverlay", overlay != nullptr);
	if (overlay) { m_shader.setUniform("uOverlay", *overlay); }
}

void FilterShader::submit(sf::RenderWindow& win) {
	auto const window_size = win.getSize();
	auto const blanket_size = sf::Vector2f{window_size};

	auto blanket = sf::RectangleShape{blanket_size};
	auto states = sf::RenderStates{};
	states.blendMode = sf::BlendAdd;
	states.shader = &m_shader;
	win.draw(blanket, states);
}

void FilterShader::draw(sf::Drawable const& target) { m_texture.draw(target); }

} // namespace fornani
