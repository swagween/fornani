
#include "editor/canvas/Background.hpp"
#include <imgui.h>

#include "editor/canvas/Canvas.hpp"
#include "fornani/setup/ResourceFinder.hpp"

namespace pi {

Background::Background(fornani::data::ResourceFinder& finder, Backdrop backdrop) : type{backdrop} {
	std::string bg_type = type.get_label();
	std::string doc = bg_type + ".png";
	auto bg = dj::Json::from_file((finder.paths.resources / "data/level/background_behaviors.json").string().c_str());
	assert(!bg.is_null());
	if (!texture.loadFromFile((finder.paths.resources / "image/background" / doc).string())) { NANI_LOG_WARN(m_logger, "Failed to load background {} ", type.get_label()); }

	auto const& in_data = bg[bg_type];
	dimensions.x = in_data["dimensions"][0].as<int>();
	dimensions.y = in_data["dimensions"][1].as<int>();
	scroll_pane = dimensions;
	auto index{0};
	for (auto& layer : in_data["layers"].array_view()) {
		layers.push_back(BackgroundLayer(texture, index, layer["scroll_speed"].as<float>(), layer["parallax"].as<float>()));
		layers.back().sprite.setTextureRect(sf::IntRect{{0, dimensions.y * index}, dimensions});
		++index;
	}
}

void Background::update() {
	for (auto& layer : layers) {
		layer.velocity.x = layer.scroll_speed * 0.1f;
		layer.position += layer.velocity;
	}
}

void Background::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float>& campos) {
	for (auto& layer : layers) {
		layer.sprite.setOrigin(sf::Vector2<float>{static_cast<float>(dimensions.x), static_cast<float>(dimensions.y)} * 0.5f + canvas.get_offset_from_center());
		layer.final_position = canvas.get_scaled_center() + canvas.get_position() + canvas.get_offset_from_center() * layer.parallax;
		layer.sprite.setScale({canvas.get_scale(), canvas.get_scale()});
		layer.sprite.setPosition(layer.final_position);
		win.draw(layer.sprite);
	}
}

void Background::debug() {
	auto& io = ImGui::GetIO();
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

BackgroundLayer::BackgroundLayer(sf::Texture& texture, int index, float speed, float parallax) : sprite(texture), render_layer(index), scroll_speed(speed), parallax(parallax) {}

} // namespace pi
