
#include <imgui.h>
#include <ccmath/ext/clamp.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>

namespace fornani::graphics {

Background::Background(automa::ServiceProvider& svc, std::string_view label, sf::Vector2f map_dimensions) : m_label{label}, map_dimensions{map_dimensions} {
	auto const& in_data = svc.data.background[label];
	if (label == "black") {
		m_attributes.set(BackgroundAttributes::no_draw);
	} else {
		m_attributes.reset(BackgroundAttributes::no_draw);
	}
	dimensions.x = in_data["dimensions"][0].as<int>();
	dimensions.y = in_data["dimensions"][1].as<int>();
	origin = sf::Vector2f{in_data["origin"][0].as<float>(), in_data["origin"][1].as<float>()};
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
			tex.setRepeated(true);
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
	if (m_attributes.test(BackgroundAttributes::no_draw)) { return; }
	auto epsilon = 0.9999f;
	for (auto [i, layer] : std::views::enumerate(layers)) {
		// backtrack sprites for infinite scroll effect
		if (layer.physics.position.x < -scroll_pane.x && !locked_horizontally()) { layer.physics.position.x = 0.f; }
		if (layer.physics.position.x > 0.f && !locked_horizontally()) { layer.physics.position.x = static_cast<float>(-scroll_pane.x); }
		if (layer.physics.position.y < -scroll_pane.y && !locked_vertically()) { layer.physics.position.y = layer.physics.position.y + static_cast<float>(scroll_pane.y); }
		if (layer.physics.position.y > 0.f && !locked_vertically()) { layer.physics.position.y = static_cast<float>(-scroll_pane.y) + layer.physics.position.y; }

		layer.final_position = layer.physics.position + origin.componentWiseMul(map_dimensions);

		if (locked_horizontally()) { layer.final_position.x = ccm::ext::clamp(layer.final_position.x, std::min(static_cast<float>(-scroll_pane.x + svc.window->i_screen_dimensions().x), -1 + epsilon), 0.f); }
		for (auto [tod, tex] : std::views::enumerate(layer.textures)) {
			auto chunks = map_dimensions / constants::f_cell_size / constants::f_chunk_size;
			auto multiplier = std::max(chunks.x, chunks.y);
			auto fdim = sf::Vector2i{map_dimensions};
			auto map_mult = std::max(fdim.x, fdim.y);
			auto sprite = sf::Sprite{tex.getTexture()};
			sprite.setScale(constants::f_scale_vec);
			sprite.setTextureRect(sf::IntRect{{}, dimensions * std::max(static_cast<int>(multiplier), map_mult)});

			auto fpos = layer.final_position - cam * layer.parallax;
			if (locked_vertically()) {
				float minY = svc.window->f_screen_dimensions().y - dimensions.y * constants::f_scale_factor;
				if (minY == 0.f) {
					fpos.y = 0.f;
				} else {
					float t = fpos.y / minY;
					t = std::clamp(t, 0.f, 1.f);
					t = t * t * (3.f - 2.f * t);
					fpos.y = std::lerp(0.f, minY, t);
				}
			}
			sprite.setPosition(fpos);
			sprite.setOrigin(sf::Vector2f{{sf::Vector2f{dimensions} * multiplier}}.componentWiseMul(origin));
			shifter.render(svc, win, sprite, tod);
		}
	}
	// debug();
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
		ImGui::Text("Origin X: %.1f", sf::Vector2f{{sf::Vector2f{dimensions} * 3.f}}.componentWiseMul(origin).x);
		ImGui::Text("Origin Y: %.1f", sf::Vector2f{{sf::Vector2f{dimensions} * 3.f}}.componentWiseMul(origin).y);

		ImGui::SeparatorText("Layers");
		auto index{0};
		for (auto& layer : layers) {
			ImGui::Separator();
			ImGui::Text("Position X: %.1f", layer.final_position.x);
			ImGui::Text("Position Y: %.1f", layer.final_position.y);
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
