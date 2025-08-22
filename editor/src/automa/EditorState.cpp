
#include <editor/automa/EditorState.hpp>
#include <imgui-SFML.h>

namespace pi {

EditorState::EditorState(fornani::automa::ServiceProvider& svc) : p_services(&svc) {};

void EditorState::render(sf::RenderWindow& win) {

	// ImGui update
	ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	ImGui::SFML::Update(p_services->window->get(), p_delta_clock.getElapsedTime());
	p_delta_clock.restart();

	p_services->window->get().clear();
	p_services->window->get().draw(p_wallpaper);
}

} // namespace pi
