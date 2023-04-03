cmake_minimum_required(VERSION 3.11)
project(fornani)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-audio)

target_sources(${PROJECT_NAME} PRIVATE

    main.cpp
    particle/Emitter.hpp
    particle/ObjectPool.hpp
    particle/Particle.hpp
    components/PhysicsComponent.hpp
    components/PhysicsComponent.cpp
	components/BehaviorComponent.hpp
	utils/Grid.hpp
	utils/Grid.cpp
	utils/Shape.hpp
	utils/Shape.cpp
	utils/Collider.hpp
	utils/Collider.cpp
	utils/Camera.hpp
	utils/StateMachine.hpp
	utils/AdjacencyList.hpp
	utils/Random.hpp
    utils/Clock.hpp
	setup/AssetManager.hpp
	setup/ServiceLocator.hpp
    setup/LookupTables.hpp
    setup/EnumLookups.hpp
    setup/MapLookups.hpp
	entities/player/Player.hpp
	entities/player/Player.cpp
    entities/behavior/Behavior.hpp
    entities/behavior/Behavior.cpp
    entities/behavior/EntityBehavior.hpp
    entities/critter/Critter.hpp
    entities/critter/Critter.cpp
    entities/critter/CritterTables.hpp
    entities/critter/CritterActions.cpp
    entities/critter/Bestiary.hpp
    entities/world/Portal.hpp
    entities/world/Portal.cpp
	weapon/Weapon.hpp
	weapon/Arsenal.hpp
	weapon/Projectile.hpp
	level/Map.hpp
	level/Map.cpp
	automa/GameState.hpp
	automa/GameState.cpp
	automa/StateManager.hpp
	automa/StateManager.cpp
    automa/StateController.hpp
    gui/HUD.hpp
	service/Service.hpp
    graphics/FLColor.hpp
    graphics/Background.hpp
    graphics/Background.cpp

)

# Copy DLLs needed for runtime on Windows
if(WIN32)
  if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:ImGui-SFML::ImGui-SFML>
          $<TARGET_FILE:sfml-graphics>
	      $<TARGET_FILE:sfml-audio>
          $<TARGET_FILE:sfml-window>
          $<TARGET_FILE:sfml-system>
          $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  endif()
endif()
