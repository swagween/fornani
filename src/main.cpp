//
//  main.cpp
//  For Nani
//

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
//#include "utils/Camera.hpp"
//all services and providers included first
#include "setup/ServiceLocator.hpp"
#include "automa/StateManager.hpp"
#include "utils/Grid.hpp"
#include "utils/Shape.hpp"

#include <imgui-SFML.h>
#include <imgui.h>
#include <random>

namespace {

auto SM = automa::StateManager{};
//auto assetManager = forloop::Service<AssetManager>::Instance{};

static void show_overlay(bool* debug) {
    const float PAD = 10.0f;
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Debug Mode", debug, window_flags)) {
            ImGui::Text("Debug Window\n" "For Nani (beta version 1.0.0)");
            ImGui::Separator();
            if (ImGui::IsMousePosValid()) {
                ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
            } else {
                ImGui::Text("Mouse Position: <invalid>");
            }
            ImGui::Separator();
            ImGui::Text("Player Stats");
            if(!svc::playerLocator.get().hurtbox.vertices.empty()) {
                ImGui::Text("Player Hurtbox Pos: (%.1f,%.1f)", svc::playerLocator.get().hurtbox.vertices.at(0).x, svc::playerLocator.get().hurtbox.vertices.at(0).y);
            }
            ImGui::Text("Player Pos: (%.1f,%.1f)", svc::playerLocator.get().physics.position.x, svc::playerLocator.get().physics.position.y);
            ImGui::Text("Player Vel: (%.1f,%.1f)", svc::playerLocator.get().physics.velocity.x, svc::playerLocator.get().physics.velocity.y);
            ImGui::Text("Player Acc: (%.1f,%.1f)", svc::playerLocator.get().physics.acceleration.x, svc::playerLocator.get().physics.acceleration.y);
            ImGui::Text("Player MTV: (%.1f,%.1f)", svc::playerLocator.get().mtv.x, svc::playerLocator.get().mtv.y);
            ImGui::Separator();
            ImGui::Text("Camera Position: (%.1f,%.1f)", svc::cameraLocator.get().physics.position.x, svc::cameraLocator.get().physics.position.y);
            if(ImGui::Button("Player Gravity")) {
                svc::playerLocator.get().grav = !svc::playerLocator.get().grav;
            }
            ImGui::SliderFloat("GRAV", &svc::playerLocator.get().PLAYER_GRAV, 0.0f, 10.0f);
            ImGui::SliderFloat("MAX X VEL", &svc::playerLocator.get().PLAYER_MAX_XVEL, 0.0f, 10.0f);
            ImGui::SliderFloat("MAX Y VEL", &svc::playerLocator.get().PLAYER_MAX_YVEL, 0.0f, 10.0f);
            ImGui::SliderFloat("X ACC", &svc::playerLocator.get().X_ACC, 0.0f, 10.0f);
            ImGui::SliderFloat("Y ACC", &svc::playerLocator.get().Y_ACC, 0.0f, 10.0f);
        }
        ImVec2 prev_size = ImGui::GetWindowSize();
        ImGui::End();
        //next window
        window_pos.y = work_pos.y + prev_size.y + 2 * PAD;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Parameters", debug, window_flags)) {
            ImGui::Text("Parameter Adjustments\n");
            ImGui::Separator();
            ImGui::Text("State");
            ImGui::Text("Current State: ");
            ImGui::SameLine();
            ImGui::TextUnformatted(SM.get_current_state_string().c_str());
            if(ImGui::Button("Menu")) {
                SM.set_current_state(std::make_unique<flstates::MainMenu>());
            }
        }
        ImGui::End();
    }
}

const sf::Vector2<uint32_t> aspect_ratio { 3840, 2160 };
const sf::Vector2<uint32_t> screen_dimensions { aspect_ratio.x / 4, aspect_ratio.y / 4 };
const int TIME_STEP_MILLI = 100;

void run(char** argv) {
    
    svc::assetLocator.get().setResourcePath(argv);
    svc::assetLocator.get().importTextures();
    svc::assetLocator.get().assignSprites();
    SM.set_current_state(std::make_unique<automa::GameState>());
    
    
    bool debug_mode = false;
    //init clock
    using Clock = std::chrono::steady_clock;
    using Time = std::chrono::duration<float>;
    auto elapsed_time = Time{};
    auto time_step = Time{std::chrono::milliseconds(TIME_STEP_MILLI)}; //FPS
    //some SFML variables for drawing a basic window + background
    auto window = sf::RenderWindow{sf::VideoMode{screen_dimensions.x, screen_dimensions.y}, "For Nani (beta v1.0)"};
    window.setKeyRepeatEnabled(false);
    ImGui::SFML::Init(window);
    
    window.setVerticalSyncEnabled(true);
    sf::RectangleShape background{};
    background.setSize(static_cast<sf::Vector2<float> >(screen_dimensions));
    background.setPosition(0, 0);
    background.setFillColor(sf::Color(20, 20, 30));
    
    
    //game loop
    sf::Clock deltaClock{};
    auto start = Clock::now();
    int frame{};
    while (window.isOpen()) {
        frame++;
        auto now = Clock::now();
        auto dt = Time{now - start};
        start = now;
        elapsed_time += dt;
        
        //SFML event variable
        auto event = sf::Event{};
        //check window events
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            switch(event.type) {
                case sf::Event::Closed:
                    return;
                case sf::Event::KeyPressed:
                    //player can refresh grid by pressing 'Z'
                    if(event.key.code == sf::Keyboard::Escape) {
                        return;
                    }
                    if(event.key.code == sf::Keyboard::D) {
                        debug_mode = !debug_mode;
                    }
                    if (event.key.code == sf::Keyboard::Q) {
                        SM.set_current_state(std::make_unique<flstates::MainMenu>());
                    }
                    if (event.key.code == sf::Keyboard::W) {
                        SM.set_current_state(std::make_unique<flstates::Dojo>());
                        SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/DOJO");
                        SM.get_current_state().setTilesetTexture(svc::assetLocator.get().sp_tileset);
                    }
                    break;
                default:
                    break;
            }
            SM.get_current_state().handle_events(event);
        }
        
        //game logic and rendering
        if(elapsed_time > time_step) {
            SM.get_current_state().logic();
        }
        
        //ImGui update
        ImGui::SFML::Update(window, deltaClock.restart());
        
        //ImGui stuff
        show_overlay(&debug_mode);
        
        //my renders
        window.clear();
        window.draw(background);
        
        SM.get_current_state().render(window);
        
        ImGui::SFML::Render(window);
        window.display();
    }
    
}

} //end namespace

int main(int argc, char** argv) {
    assert(argc > 0);
    run(argv);
    ImGui::SFML::Shutdown();
    return 0;
}


