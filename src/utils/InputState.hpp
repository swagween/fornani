//
//  InputState.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <memory>
#include <random>
#include <chrono>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "BitFlags.hpp"

namespace util {

    enum class key_state {triggered, held, released};
    enum class key { shift, z, x, a, s, left, right, up, down };

    struct Key {
        key name{};
        BitFlags<key_state> key_state{};
    };

class InputState {
    
public:

    InputState() = default;
    InputState(const InputState&) = delete; // non construction-copyable
    InputState& operator=(const InputState&) = delete; // non copyable

    static InputState* create() {};

    void reset_triggers() {
        keys.at(sf::Keyboard::LShift).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::Z).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::X).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::A).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::S).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::Left).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::Right).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::Up).key_state.reset(key_state::triggered);
        keys.at(sf::Keyboard::Down).key_state.reset(key_state::triggered);

        keys.at(sf::Keyboard::LShift).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::Z).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::X).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::A).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::S).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::Left).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::Right).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::Up).key_state.reset(key_state::released);
        keys.at(sf::Keyboard::Down).key_state.reset(key_state::released);
    };

    void handle_press(sf::Keyboard::Key& k) {
        if (keys.contains(k)) {
            keys.at(k).key_state.set(util::key_state::triggered);
            keys.at(k).key_state.set(util::key_state::held);
        }
    }
    void handle_release(sf::Keyboard::Key& k) {
        if (keys.contains(k)) {
            keys.at(k).key_state.set(util::key_state::released);
            keys.at(k).key_state.reset(util::key_state::held);
        }
    }

    std::unordered_map<sf::Keyboard::Key, Key> keys {
        {sf::Keyboard::LShift,    Key{key::shift}},
        {sf::Keyboard::Z,        Key{key::z}},
        {sf::Keyboard::X,        Key{key::x}},
        {sf::Keyboard::A,        Key{key::a}},
        {sf::Keyboard::S,        Key{key::s}},
        {sf::Keyboard::Left,     Key{key::left}},
        {sf::Keyboard::Right,    Key{key::right}},
        {sf::Keyboard::Up,       Key{key::up}},
        {sf::Keyboard::Down,     Key{key::down}},
    };
    
}; // End InputState

}

 /* InputState_hpp */
