
#include <imgui.h>
#include <ccmath/ext/clamp.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/EnumLookups.hpp>
#include <fornani/utils/Math.hpp>

namespace fornani::graphics {

Background::Background(automa::ServiceProvider& svc, std::string_view label) : m_label{label} {
	auto const& in_data = svc.data.background[label];
	dimensions.x = in_data["dimensions"][0].as<int>();
	dimensions.y = in_data["dimensions"][1].as<int>();
	scroll_pane = dimensions * 2;
	if (in_data["lock"]["horizontal"].as_bool()) { lock_horizontally(); }
	if (in_data["lock"]["vertical"].as_bool()) { lock_vertically(); }
	for (auto [i, layer] : std::views::enumerate(in_data["layers"].as_array())) {
		layers.push_back(BackgroundLayer{static_cast<int>(i), layer["scroll_speed"].as<float>(), layer["parallax"].as<float>()});
		layers.back().physics.set_global_friction(1.f);
		for (auto [tod, tex] : std::views::enumerate(layers.back().textures)) {
			tex.setRepeated(true);
			if (!tex.resize(sf::Vector2u{dimensions})) { continue; }
			tex.clear(sf::Color::Transparent);
			auto sprite = sf::Sprite{svc.assets.get_texture("background_" + std::string{label})};
			sprite.setTextureRect(sf::IntRect{{dimensions.x * static_cast<int>(tod), dimensions.y * static_cast<int>(i)}, dimensions});
			tex.draw(sprite);
			tex.display();
		}
	}
}

void Background::update(automa::ServiceProvider& svc) {
	for (auto& layer : layers) {
		layer.physics.velocity.x = layer.scroll_speed;
		layer.physics.update_euler(svc);
	}
}

void Background::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	auto epsilon = 0.9999f;
	for (auto [i, layer] : std::views::enumerate(layers)) {
		// backtrack sprites for infinite scroll effect
		if (layer.physics.position.x < -scroll_pane.x && !locked_horizontally()) { layer.physics.position.x = 0.f; }
		if (layer.physics.position.x > 0.f && !locked_horizontally()) { layer.physics.position.x = static_cast<float>(-scroll_pane.x); }
		if (layer.physics.position.y < -scroll_pane.y && !locked_vertically()) { layer.physics.position.y = layer.physics.position.y + static_cast<float>(scroll_pane.y); }
		if (layer.physics.position.y > 0.f && !locked_vertically()) { layer.physics.position.y = static_cast<float>(-scroll_pane.y) + layer.physics.position.y; }

		layer.final_position = layer.physics.position - cam * layer.parallax;

		if (locked_vertically()) { layer.final_position.y = ccm::ext::clamp(layer.final_position.y, std::min(static_cast<float>(-scroll_pane.y + svc.window->i_screen_dimensions().y), -1 + epsilon), 0.f); }
		if (locked_horizontally()) { layer.final_position.x = ccm::ext::clamp(layer.final_position.x, std::min(static_cast<float>(-scroll_pane.x + svc.window->i_screen_dimensions().x), -1 + epsilon), 0.f); }
		for (auto [tod, tex] : std::views::enumerate(layer.textures)) {
			auto sprite = sf::Sprite{tex.getTexture()};
			sprite.setScale(constants::f_scale_vec);
			sprite.setTextureRect(sf::IntRect{{}, svc.window->i_screen_dimensions() * 4});
			sprite.setPosition(layer.final_position);
			shifter.render(svc, win, sprite, tod);
		}
	}
}

void Background::lock() {
	lock_horizontally();
	lock_vertically();
}

void Background::lock_horizontally() { m_attributes.set(BackgroundAttributes::horizontal_lock); }

void Background::lock_vertically() { m_attributes.set(BackgroundAttributes::vertical_lock); }

void Background::debug() {
	static bool b_debug{};
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGuiViewport const* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos;
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	window_flags |= ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowBgAlpha(0.65f);
	if (ImGui::Begin("Background Parameters", &b_debug, window_flags)) {
		auto index{0};
		for (auto& layer : layers) {
			ImGui::Separator();
			ImGui::Text("Layer %i", index);
			std::string plabel = "parallax" + std::to_string(index);
			std::string slabel = "speed" + std::to_string(index);
			ImGui::SliderFloat(plabel.data(), &layer.parallax, 0.f, 1.f);
			ImGui::SliderFloat(slabel.data(), &layer.scroll_speed, -8.f, 8.f);
			++index;
		}
	}
	ImGui::End();
}

} // namespace fornani::graphics
