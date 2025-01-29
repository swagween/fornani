
#include "launcher/LauncherApplication.hpp"

#include <steam/steam_api.h>
#include <iostream>

#ifndef FORNANI_STEAM_APP_ID
#error "FORNANI_STEAM_APP_ID was not defined!"
#endif

#if defined(FORNANI_STEAM_APP_ID) && FORNANI_STEAM_APP_ID < 0
#error "FORNANI_STEAM_APP_ID was defined as a negative number!"
#endif

static constexpr const char * logFile{"fornani.log"};

int main(int argc, char** argv) {
	assert(argc > 0);

	// TODO: Maybe move this into a config file?
	auto config = fornani::logger::Config{};
	// Required to initialize the logger for the application. This must also stay outside any try/catch block.
	auto log_instance = fornani::logger::Instance{logFile, config};
	const fornani::Logger main_logger{"Main"};


	launcher::LauncherApplication app{argv};
	app.init(argv);

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
	app.init(argv);
	app.launch(argv);

	return EXIT_SUCCESS;
}
