
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

void FilterShader::finalize(float time, sf::Vector2u size, sf::Vector2f position, sf::Color highlight, float intensity) {
	NANI_LOG_INFO(m_logger, "FilterShader::finalize");
	NANI_LOG_INFO(m_logger, "  resolution: {} x {}", size.x, size.y);
	NANI_LOG_INFO(m_logger, "  center (SFML pixels): ({}, {})", position.x, position.y);
	NANI_LOG_INFO(m_logger, "  expected GL center: ({}, {})", position.x, static_cast<float>(size.y) - position.y);
	NANI_LOG_INFO(m_logger, "  intensity: {}", intensity);

	m_shader.setUniform("uTime", time);
	m_shader.setUniform("uResolution", sf::Glsl::Vec2(size));
	auto float_hr = highlight.r / 255.f;
	auto float_hg = highlight.g / 255.f;
	auto float_hb = highlight.b / 255.f;
	m_shader.setUniform("uHighlightColor", sf::Glsl::Vec3(float_hr, float_hg, float_hb));
	m_shader.setUniform("uIntensity", intensity);
	m_shader.setUniform("uCenter", sf::Glsl::Vec2{position});
}

void FilterShader::submit(sf::RenderWindow& win) {
	auto const window_size = win.getSize();
	auto const blanket_size = sf::Vector2f{window_size};

	NANI_LOG_INFO(m_logger, "FilterShader::submit");
	NANI_LOG_INFO(m_logger, "  window size: {} x {}", window_size.x, window_size.y);
	NANI_LOG_INFO(m_logger, "  blanket size: {} x {}", blanket_size.x, blanket_size.y);

	auto blanket = sf::RectangleShape{blanket_size};
	win.draw(blanket, &m_shader);
}

void FilterShader::draw(sf::Drawable const& target) { m_texture.draw(target); }

} // namespace fornani
