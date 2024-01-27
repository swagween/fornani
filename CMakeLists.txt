
cmake_minimum_required(VERSION 3.11)
project(fornani
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME})

include(FetchContent)
FetchContent_Declare(
    djson
    GIT_REPOSITORY https://github.com/karnkaul/djson
    GIT_TAG 8d08162f1463ec8a236e65190f5db0a82132682d # 2.0
)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(djson)

target_link_libraries(${PROJECT_NAME} PRIVATE djson::djson)

set(BUILD_SHARED_LIBS ON)
add_subdirectory(dependencies)

target_link_libraries(${PROJECT_NAME} PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-audio)

target_sources(${PROJECT_NAME} PRIVATE
main.cpp
src/particle/Emitter.hpp
src/particle/ObjectPool.hpp
src/particle/Particle.hpp
src/particle/Attractor.hpp
src/particle/Attractor.cpp
src/components/PhysicsComponent.hpp
src/components/PhysicsComponent.cpp
src/components/BehaviorComponent.hpp
src/utils/Grid.hpp
src/utils/Grid.cpp
src/utils/Shape.hpp
src/utils/Shape.cpp
src/utils/Collider.hpp
src/utils/Collider.cpp
src/utils/Camera.hpp
src/utils/StateMachine.hpp
src/utils/StateFunction.hpp
src/utils/AdjacencyList.hpp
src/utils/Random.hpp
src/utils/Clock.hpp
src/utils/BitFlags.hpp
src/utils/InputState.hpp
src/setup/AssetManager.hpp
src/setup/ServiceLocator.hpp
src/setup/LookupTables.hpp
src/setup/EnumLookups.hpp
src/setup/MapLookups.hpp
src/setup/DataManager.hpp
src/setup/DataManager.cpp
src/setup/ResourceFinder.hpp
src/entities/player/Player.hpp
src/entities/player/Player.cpp
src/entities/behavior/Behavior.hpp
src/entities/behavior/Behavior.cpp
src/entities/behavior/Animation.hpp
src/entities/behavior/Animation.cpp
src/entities/behavior/EntityBehavior.hpp
src/entities/critter/Critter.hpp
src/entities/critter/Critter.cpp
src/entities/critter/CritterActions.cpp
src/entities/critter/Bestiary.hpp
src/entities/critter/critters/Hulmet.hpp
src/entities/critter/critters/Hulmet.cpp
src/entities/critter/critters/Frdog.hpp
src/entities/critter/critters/Frdog.cpp
src/entities/critter/Bestiary.hpp
src/entities/world/Portal.hpp
src/entities/world/Portal.cpp
src/entities/world/Inspectable.hpp
src/entities/world/Inspectable.cpp
src/entities/world/Animator.hpp
src/entities/world/Animator.cpp
src/entities/item/Drop.hpp
src/entities/item/Drop.cpp
src/entities/item/Loot.hpp
src/entities/item/Loot.cpp
src/weapon/Weapon.hpp
src/weapon/Weapon.cpp
src/weapon/Arsenal.hpp
src/weapon/Projectile.hpp
src/weapon/Projectile.cpp
src/level/Map.hpp
src/level/Map.cpp
src/automa/GameState.hpp
src/automa/GameState.cpp
src/automa/StateManager.hpp
src/automa/StateManager.cpp
src/automa/StateController.hpp
src/gui/HUD.hpp
src/gui/Console.hpp
src/gui/Console.cpp
src/service/Service.hpp
src/graphics/FLColor.hpp
src/graphics/Background.hpp
src/graphics/Background.cpp
src/graphics/Transition.hpp
src/graphics/Transition.cpp
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
