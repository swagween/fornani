
#include "editor/setup/EditorApplication.hpp"

#include <imgui.h>
#include <steam/steam_api.h>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <random>
#include <imgui-SFML.h>

#ifndef FORNANI_STEAM_APP_ID
#error "FORNANI_STEAM_APP_ID was not defined!"
#endif

#if defined(FORNANI_STEAM_APP_ID) && FORNANI_STEAM_APP_ID < 0
#error "FORNANI_STEAM_APP_ID was defined as a negative number!"
#endif

static constexpr char const* logFile{"pioneer.log"};

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

// int main() {
//	sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3.0 + ImGui-SFML Modal Test");
//	window.setFramerateLimit(60);
//
//	// Important: set the OpenGL context as active before initializing ImGui
//	if (!window.setActive(true)) { return 0; }
//	if (!ImGui::SFML::Init(window)) { return 0; }
//
//	sf::Clock deltaClock;
//	bool show_modal = false;
//
//	while (window.isOpen()) {
//		while (std::optional const event = window.pollEvent()) {
//			ImGui::SFML::ProcessEvent(window, *event);
//			if (event->is<sf::Event::Closed>()) { return 0; }
//
//			// Toggle modal on M key press
//			if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
//				if (key_pressed->scancode == sf::Keyboard::Scancode::M) { show_modal = true; }
//			}
//		}
//
//		// Ensure OpenGL context is active
//		if (!window.setActive(true)) { return 0; }
//
//		ImGui::SFML::Update(window, deltaClock.restart());
//
//		static bool modal_open = false;
//		if (modal_open) {
//			ImGui::Text("Popup is open (according to IsPopupOpen)");
//		} else {
//			ImGui::Text("Popup is NOT open (according to IsPopupOpen)");
//		}
//		if (show_modal) {
//			ImGui::OpenPopup("Test Modal");
//			modal_open = true;
//			show_modal = false;
//		}
//
//		if (ImGui::BeginPopupModal("Test Modal")) {
//			modal_open = true;
//			// Modal content
//			if (ImGui::Button("Close")) {
//				ImGui::CloseCurrentPopup();
//				modal_open = false;
//			}
//			ImGui::EndPopup();
//		} else {
//			modal_open = false;
//		}
//
//		window.clear(sf::Color(30, 30, 30));
//		ImGui::SFML::Render(window);
//		window.display();
//	}
//
//	ImGui::SFML::Shutdown();
//	return 0;
// }

int main(int argc, char** argv) {
	assert(argc > 0);

	// TODO: Maybe move this into a config file?
	auto config = fornani::io::logger::Config{};
	// Required to initialize the logger for the application. This must also stay outside any try/catch block.
	auto log_instance = fornani::io::logger::Instance{logFile, config};
	fornani::io::Logger const main_logger{"PioneerMain"};

	constexpr auto steam_id = FORNANI_STEAM_APP_ID;
	NANI_LOG_INFO(main_logger, "Current passed steam ID: {}", steam_id);

	if (SteamAPI_RestartAppIfNecessary(steam_id)) {
		NANI_LOG_INFO(main_logger, "Steam requested we re-launch through Steam.");
		return EXIT_SUCCESS;
	}
	SteamErrMsg errMsg;
	if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK) {
		NANI_LOG_ERROR(main_logger, "Failed to init Steam: {}", static_cast<char const*>(errMsg));
		return EXIT_FAILURE;
	}

	NANI_LOG_INFO(main_logger, "SteamAPI has been initialized.");

	pi::EditorApplication app{argv};
	app.run(argv);
	ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
