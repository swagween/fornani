
#include <editor/automa/EditorContext.hpp>
#include <editor/automa/EditorState.hpp>
#include <imgui-SFML.h>

namespace pi {

EditorState::EditorState(fornani::automa::ServiceProvider& svc, EditorContext& ctx) : p_services(&svc), p_context(&ctx) {};

void EditorState::render(sf::RenderWindow& win) {

	// ImGui update
	ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	if (!win.setActive(true)) {}
	ImGui::SFML::Update(p_services->window->get(), p_delta_clock.getElapsedTime());
	p_delta_clock.restart();

	p_services->window->get().clear();
	p_services->window->get().draw(p_wallpaper);
}

bool EditorState::create_new_room() {
	ImGui::Text("Please enter a new room name.");
	ImGui::Text("Convention is all lowercase, snake-case, and of the format `room_name`.");
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::InputTextWithHint("Region Name", "firstwind", regbuffer, IM_ARRAYSIZE(regbuffer));
	ImGui::InputTextWithHint("Room Name", "boiler_room", roombuffer, IM_ARRAYSIZE(roombuffer));
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("Please specify the dimensions of the level in chunks (16x16 tiles)");
	ImGui::Separator();
	ImGui::NewLine();

	width = ccm::ext::clamp(width, 1, std::numeric_limits<int>::max());
	height = ccm::ext::clamp(height, 1, std::numeric_limits<int>::max());

	ImGui::InputInt("Width", &width);
	ImGui::NewLine();

	ImGui::InputInt("Height", &height);
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("Metagrid Position");
	ImGui::InputInt("X", &p_context->metagrid_position.x);
	ImGui::SameLine();

	ImGui::InputInt("Y", &p_context->metagrid_position.y);
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
	ImGui::SameLine();
	if (ImGui::Button("Create")) { return true; }
	ImGui::EndPopup();

	return false;
}

} // namespace pi
