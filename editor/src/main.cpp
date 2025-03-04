
#include "editor/setup/EditorApplication.hpp"

#include <SFML/Graphics.hpp>
#include "editor/util/Lookup.hpp"
#include <chrono>
#include <imgui-SFML.h>
#include <imgui.h>
#include <random>
#include <steam/steam_api.h>

#ifndef FORNANI_STEAM_APP_ID
#error "FORNANI_STEAM_APP_ID was not defined!"
#endif

#if defined(FORNANI_STEAM_APP_ID) && FORNANI_STEAM_APP_ID < 0
#error "FORNANI_STEAM_APP_ID was defined as a negative number!"
#endif

static constexpr const char * logFile{"pioneer.log"};


int main(int argc, char** argv) {
	assert(argc > 0);

	// TODO: Maybe move this into a config file?
	auto config = fornani::io::logger::Config{};
	// Required to initialize the logger for the application. This must also stay outside any try/catch block.
	auto log_instance = fornani::io::logger::Instance{logFile, config};
	const fornani::io::Logger main_logger{"PioneerMain"};

	constexpr auto steam_id = FORNANI_STEAM_APP_ID;
	NANI_LOG_INFO(main_logger, "Current passed steam ID: {}", steam_id);

	if (SteamAPI_RestartAppIfNecessary(steam_id)) {
		NANI_LOG_INFO(main_logger, "Steam requested we re-launch through Steam.");
		return EXIT_SUCCESS;
	}
	SteamErrMsg errMsg;
	if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK) {
		NANI_LOG_ERROR(main_logger, "Failed to init Steam: {}", static_cast<const char *>(errMsg));
		return EXIT_FAILURE;
	}

	NANI_LOG_INFO(main_logger, "SteamAPI has been initialized.");

	pi::EditorApplication app{argv};
	app.launch(argv);
    ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
