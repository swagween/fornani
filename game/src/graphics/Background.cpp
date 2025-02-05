
#include "fornani/graphics/Background.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Math.hpp"
#include <algorithm>
#include <imgui.h>

#include <tracy/Tracy.hpp>

namespace bg {

int const tile_dim{256};

Background::Background(automa::ServiceProvider& svc, int bg_id) : labels{{0, "dusk"}, {5, "night"}, {3, "woods"}, {18, "woods"}} {
	auto type = labels.contains(bg_id) ? labels.at(bg_id) : "black";
	auto const& in_data = svc.data.background[type];
	dimensions.x = in_data["dimensions"][0].as<int>();
	dimensions.y = in_data["dimensions"][1].as<int>();
	scroll_pane = dimensions;
	if (in_data["lock"]["horizontal"].as_bool()) { lock_horizontally(); }
	if (in_data["lock"]["vertical"].as_bool()) { lock_vertically(); }
	auto index{0};
	for (auto& layer : in_data["layers"].array_view()) {
		layers.push_back({index, layer["scroll_speed"].as<float>(), layer["parallax"].as<float>()});
		layers.back().physics.set_global_friction(1.f);
		for (auto i{0}; i < svc.world_clock.num_cycles(); ++i) {
			layers.back().sprites.push_back(sf::Sprite{svc.assets.get_background(bg_id)});
			layers.back().sprites.back().setTextureRect(sf::IntRect{{i * dimensions.x, dimensions.y * index}, dimensions});
		}
		++index;
	}
}

void Background::update(automa::ServiceProvider& svc) {
	for (auto& layer : layers) {
		layer.physics.velocity.x = layer.scroll_speed;
		layer.physics.update_euler(svc);
	}
}

void Background::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	ZoneScopedN("Background::render");
	auto epsilon = 0.99999f;
	auto cycle = static_cast<int>(svc.world_clock.get_time_of_day());
	auto from_cycle = static_cast<int>(svc.world_clock.get_previous_time_of_day());
	
	for (auto& layer : layers) {
		// backtrack sprites for infinite scroll effect
		if (layer.physics.position.x < -scroll_pane.x && !locked_horizontally()) { layer.physics.position.x = 0.f; }
		if (layer.physics.position.x > 0.f && !locked_horizontally()) { layer.physics.position.x = static_cast<float>(-scroll_pane.x); }
		if (layer.physics.position.y < -scroll_pane.y && !locked_vertically()) { layer.physics.position.y = layer.physics.position.y + static_cast<float>(scroll_pane.y);
		}
		if (layer.physics.position.y > 0.f && !locked_vertically()) { layer.physics.position.y = static_cast<float>(-scroll_pane.y) + layer.physics.position.y;
		}

		layer.final_position = layer.physics.position - cam * layer.parallax;

		if (locked_vertically()) { layer.final_position.y = std::clamp(layer.final_position.y, std::min(static_cast<float>(-scroll_pane.y + svc.window->screen_dimensions.y), -1 + epsilon), 0.f); }
		if (locked_horizontally()) { layer.final_position.x = std::clamp(layer.final_position.x, std::min(static_cast<float>(-scroll_pane.x + svc.window->screen_dimensions.x), -1 + epsilon), 0.f); }
		auto ctr{0};
		for (auto& sprite : layer.sprites) {
			for (auto i{0}; i < 2; ++i) {
				for (auto j{0}; j < 2; ++j) {
					sprite.setPosition(layer.final_position + sf::Vector2<float>{static_cast<float>(dimensions.x * epsilon) * static_cast<float>(i), static_cast<float>(dimensions.y * epsilon) * static_cast<float>(j)});
					if (cycle == ctr) {
						uint8_t alpha = from_cycle > cycle ? 255 : util::get_uint8_from_normal(1.f - svc.world_clock.get_transition());
						sprite.setColor({255, 255, 255, alpha});
						win.draw(sprite);
					} else if (ctr == from_cycle && svc.world_clock.is_transitioning()) {
						uint8_t alpha = from_cycle <= cycle ? 255 : util::get_uint8_from_normal(svc.world_clock.get_transition());
						sprite.setColor({255, 255, 255, alpha});
						win.draw(sprite);
					}
				}
			}
			sprite.setColor(sf::Color::White);
			++ctr;
		}
	}
}

void Background::lock() {
	lock_horizontally();
	lock_vertically();
}

void Background::lock_horizontally() { attributes.horizontal.set(BackgroundAttributes::lock); }

void Background::lock_vertically() { attributes.vertical.set(BackgroundAttributes::lock); }

void Background::debug() {
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
	if (ImGui::Begin("Background Parameters", b_debug, window_flags)) {
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

} // namespace bg
