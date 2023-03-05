//
//  main.cpp
//  For Nani
//

#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>
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
auto window = sf::RenderWindow();

const int NUM_TIMESTEPS = 64;
int TIME_STEP_MILLI = 8;
int frame{};
using Clock = std::chrono::steady_clock;
using Time = std::chrono::duration<float>;
auto elapsed_time = Time{};
auto elapsed_marker = Time{};
float time_markers[NUM_TIMESTEPS]{};
auto time_step = Time{std::chrono::milliseconds(TIME_STEP_MILLI)}; //FPS
float seconds = 0.0;
int FPS_counter = 0;
float FPS = 0.0;

int shake_counter = 0;



static void show_overlay() {
    bool* debug{};
    const float PAD = 10.0f;
    static int corner = 1;
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0;
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
            ImGui::Text("Window Focused: ");
            ImGui::SameLine();
            if(window.hasFocus()) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
            if(!window.hasFocus()) {
                window.RenderTarget::setActive();
            }
            ImGui::Separator();
            if (ImGui::IsMousePosValid()) {
                ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
            } else {
                ImGui::Text("Mouse Position: <invalid>");
            }
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Time"))
                {
                    ImGui::Separator();
                    ImGui::Text("Time");
                    ImGui::TextUnformatted(std::to_string(time_markers[frame % NUM_TIMESTEPS]).c_str());
                    ImGui::Text("Time (seconds): %.1f", seconds);
                    ImGui::Text("FPS: %.3f", FPS);
                    ImGui::SliderInt("Time Step (in milliseconds)", &TIME_STEP_MILLI, 0, 500);
                    ImGui::Separator();
                    ImGui::EndTabItem();
                    ImGui::PlotHistogram("Frame Times", time_markers, NUM_TIMESTEPS, 0, NULL, 0.0f, 0.02f, ImVec2(0, 80.0f));
                }
                if (ImGui::BeginTabItem("Player"))
                {
                    ImGui::Text("Player Stats");
                    if(!svc::playerLocator.get().hurtbox.vertices.empty()) {
                        ImGui::Text("Player Hurtbox Pos: (%.1f,%.1f)", svc::playerLocator.get().hurtbox.vertices.at(0).x, svc::playerLocator.get().hurtbox.vertices.at(0).y);
                    }
                    ImGui::Text("Player Behavior: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().behavior.current_state.get()) {
                        ImGui::TextUnformatted(svc::playerLocator.get().behavior.current_state.get()->params.behavior_id.c_str());
                    } else {
                        ImGui::Text("nullptr");
                    }
                    ImGui::Text("Behavior Restricted: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().behavior.current_state.get()) {
                        if(svc::playerLocator.get().behavior.restricted()) {
                            ImGui::TextUnformatted("Yes");
                        } else {
                            ImGui::TextUnformatted("No");
                        }
                    }
                    ImGui::Text("Player Facing: ");
                    ImGui::SameLine();
                    ImGui::TextUnformatted(svc::playerLocator.get().print_direction().c_str());
                    ImGui::Text("Colliding with Level: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().is_colliding_with_level) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Text("Grounded: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().grounded) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Text("Is Wall Sliding: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().is_wall_sliding) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Text("Wall Slide Trigger: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().wall_slide_trigger) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    //                    ImGui::Text("Jump Request: ");
                    //                    ImGui::SameLine();
                    //                    ImGui::TextUnformatted(std::to_string(svc::playerLocator.get().jump_request).c_str());
                    //                    ImGui::Text("Jump Height Counter: ");
                    //                    ImGui::SameLine();
                    //                    ImGui::TextUnformatted(std::to_string(svc::playerLocator.get().jump_height_counter).c_str());
                    //                    ImGui::Text("Jump Hold: ");
                    //                    ImGui::SameLine();
                    //                    if(svc::playerLocator.get().jump_hold) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    //                    ImGui::Text("Just Jumped: ");
                    //                    ImGui::SameLine();
                    //                    if(svc::playerLocator.get().just_jumped) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Text("Anim Frame: ");
                    ImGui::SameLine();
                    ImGui::TextUnformatted(std::to_string(svc::playerLocator.get().behavior.current_state.get()->params.current_frame).c_str());
                    ImGui::Text("Real Frame: ");
                    ImGui::SameLine();
                    ImGui::TextUnformatted(std::to_string(svc::playerLocator.get().behavior.current_state.get()->params.anim_frame).c_str());
                    ImGui::Text("Has Right Collision: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().has_right_collision) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Text("Has Left Collision: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().has_left_collision) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Text("Player Pos: (%.4f,%.4f)", svc::playerLocator.get().physics.position.x, svc::playerLocator.get().physics.position.y);
                    ImGui::Text("Player Vel: (%.4f,%.4f)", svc::playerLocator.get().physics.velocity.x, svc::playerLocator.get().physics.velocity.y);
                    ImGui::Text("Player Acc: (%.4f,%.4f)", svc::playerLocator.get().physics.acceleration.x, svc::playerLocator.get().physics.acceleration.y);
                    
                    ImGui::SliderFloat("GRAV",              &svc::playerLocator.get().stats.PLAYER_GRAV, 0.0f, 2.0f);
                    ImGui::SliderFloat("AIR MULTIPLIER",    &svc::playerLocator.get().stats.AIR_MULTIPLIER, 0.0f, 5.0f);
                    ImGui::SliderFloat("PLAYER MAX XVEL",   &svc::playerLocator.get().stats.PLAYER_MAX_XVEL, 0.1f, 8.0f);
                    ImGui::SliderFloat("PLAYER MAX YVEL",   &svc::playerLocator.get().stats.PLAYER_MAX_YVEL, 0.1f, 8.0f);
                    ImGui::SliderFloat("PLAYER FRIC",       &svc::playerLocator.get().stats.PLAYER_HORIZ_FRIC, 0.1f, 1.0f);
                    ImGui::SliderFloat("PLAYER AIR FRIC",   &svc::playerLocator.get().stats.PLAYER_HORIZ_AIR_FRIC, 0.1f, 1.0f);
                    ImGui::SliderFloat("X ACC",             &svc::playerLocator.get().stats.X_ACC, 0.0f, 1.0f);
                    ImGui::SliderFloat("Y ACC",             &svc::playerLocator.get().stats.Y_ACC, 0.0f, 1.0f);
                    ImGui::SliderFloat("JUMP MAX",          &svc::playerLocator.get().stats.JUMP_MAX, 0.0f, 10.0f);
                    ImGui::SliderInt("JUMP TIME",           &svc::playerLocator.get().stats.JUMP_TIME, 0, 200);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Weapon"))
                {
                    ImGui::Separator();
                    ImGui::Text("Equipped Weapon: ");
                    ImGui::SameLine();
                    ImGui::TextUnformatted(svc::playerLocator.get().loadout.get_equipped_weapon().label.c_str());
                    ImGui::Text("Weapon Fired: ");
                    ImGui::SameLine();
                    if(svc::playerLocator.get().weapon_fired) { ImGui::Text("Yes"); } else { ImGui::Text("No"); }
                    ImGui::Separator();
                    ImGui::Text("Weapon Stats: ");
                    ImGui::Indent();
                    ImGui::Text("Rate: (%i)",     svc::playerLocator.get().loadout.get_equipped_weapon().attributes.rate);
                    ImGui::Text("Cooldown: (%i)", svc::playerLocator.get().loadout.get_equipped_weapon().attributes.cooldown_time);
                    ImGui::Text("Recoil: (%.2f)", svc::playerLocator.get().loadout.get_equipped_weapon().attributes.recoil);
                    ImGui::Separator();
                    ImGui::Unindent();
                    ImGui::Text("Projectile Stats: ");
                    ImGui::Indent();
                    ImGui::Text("Damage: (%i)",   svc::playerLocator.get().loadout.get_equipped_weapon().projectile.stats.damage);
                    ImGui::Text("Lifespan: (%i)", svc::playerLocator.get().loadout.get_equipped_weapon().projectile.stats.lifespan);
                    ImGui::Text("Speed: (%.2f)",  svc::playerLocator.get().loadout.get_equipped_weapon().projectile.stats.speed);
                    ImGui::Text("Velocity: (%.4f,%.4f)", svc::playerLocator.get().loadout.get_equipped_weapon().projectile.physics.velocity.x,
                                svc::playerLocator.get().loadout.get_equipped_weapon().projectile.physics.velocity.y);
                    ImGui::Text("Position: (%.4f,%.4f)", svc::playerLocator.get().loadout.get_equipped_weapon().projectile.physics.position.x,
                                svc::playerLocator.get().loadout.get_equipped_weapon().projectile.physics.position.y);
                    ImGui::EndTabItem();
                    
                }
                if (ImGui::BeginTabItem("Camera"))
                {
                    ImGui::Text("Camera Position: (%.8f,%.8f)", svc::cameraLocator.get().physics.position.x, svc::cameraLocator.get().physics.position.y);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Resources"))
                {
                    ImGui::Text("Size of Asset Manager (Bytes): %lu", sizeof(svc::assetLocator.get()));
                    ImGui::Text("Size of Camera (Bytes): %lu", sizeof(svc::cameraLocator.get()));
                    ImGui::Text("Size of Player (Bytes): %lu", sizeof(svc::playerLocator.get()));
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("State"))
                {
                    ImGui::Separator();
                    ImGui::Text("State");
                    ImGui::Text("Current State: ");
                    ImGui::SameLine();
                    ImGui::TextUnformatted(SM.get_current_state_string().c_str());
                    if(ImGui::Button("Menu")) {
//                        svc::assetLocator.get().click.play();
                        SM.set_current_state(std::make_unique<flstates::MainMenu>());
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            
            
        }
        ImVec2 prev_size = ImGui::GetWindowSize();
        ImGui::End();
    }
}


void run(char** argv) {
    
    //load all assets
    //images
    svc::assetLocator.get().setResourcePath(argv);
    svc::assetLocator.get().importTextures();
    svc::assetLocator.get().assignSprites();
    //sounds
    svc::assetLocator.get().load_audio();
    
    //state manager
    SM.set_current_state(std::make_unique<automa::GameState>());
    
    window.create(sf::VideoMode(screen_dimensions.x, screen_dimensions.y), "For Nani (beta v1.0)");
    
    bool debug_mode = true;
    //init clock
    
    //some SFML variables for drawing a basic window + background
    window.setVerticalSyncEnabled(true);
    
    window.setKeyRepeatEnabled(false);
    
    ImGui::SFML::Init(window);
    window.requestFocus();
    
    
    sf::RectangleShape background{};
    background.setSize(static_cast<sf::Vector2<float> >(screen_dimensions));
    background.setPosition(0, 0);
    background.setFillColor(sf::Color(10, 10, 20));
    
    
    //game loop
    sf::Clock deltaClock{};
    auto current_time = Clock::now();
    
    while (window.isOpen()) {
        time_step = Time{std::chrono::milliseconds(TIME_STEP_MILLI)};
        frame++;
        auto new_time = Clock::now();
        auto frame_time = Time{new_time - current_time};
        current_time = new_time;
        elapsed_time += frame_time;
        elapsed_marker = elapsed_time;
        time_markers[frame%NUM_TIMESTEPS] = frame_time.count();
        seconds += elapsed_time.count();
        FPS_counter++;
        
        
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
                        SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/HOARDER_DEADEND_01");
                    }
                    break;
                default:
                    break;
            }
            SM.get_current_state().handle_events(event);
        }
        
        //game logic and rendering
        
        //        elapsed_time = Time::zero();
        if(elapsed_time.count() > time_step.count()) {
            SM.get_current_state().logic(elapsed_time);
            FPS = FPS_counter / seconds;
            elapsed_time = Time::zero();
            if((int)floor(seconds) % 8 == 0) {
//                svc::cameraLocator.get().begin_shake();
            }
        }
        
        
            
        ImGui::SFML::Update(window, deltaClock.restart());
//        ImGui::ShowDemoWindow();
        
        //ImGui stuff
        if(debug_mode) {
            show_overlay();
        }
        
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


