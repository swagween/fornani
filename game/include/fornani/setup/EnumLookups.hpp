
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace fornani::lookup {


inline constexpr float SPACING{ 32.0f };

inline constexpr float unit_size_f{ 32.0f };
inline constexpr int unit_size_i{32};

inline constexpr float min_hook_length{32.f};
inline constexpr float max_hook_length{256.f};

enum class Style { firstwind, overturned, base, factory, greatwing, provisional, END };

inline std::unordered_map<Style, char const*> get_style_string{{Style::firstwind, "firstwind"}, {Style::overturned, "overturned"}, {Style::base, "base"},
															   {Style::greatwing, "greatwing"}, {Style::factory, "factory"},	   {Style::provisional, "provisional"}};

inline std::unordered_map<char, int> get_character {
    {'a', 0},
    {'b', 1},
    {'c', 2},
    {'d', 3},
    {'e', 4},
    {'f', 5},
    {'g', 6},
    {'h', 7},
    {'i', 8},
    {'j', 9},
    {'k', 10},
    {'l', 11},
    {'m', 12},
    {'n', 13},
    {'o', 14},
    {'p', 15},
    {'q', 16},
    {'r', 17},
    {'s', 18},
    {'t', 19},
    {'u', 20},
    {'v', 21},
    {'w', 22},
    {'x', 23},
    {'y', 24},
    {'z', 25},
    {'1', 26},
    {'2', 27},
    {'3', 28},
    {'4', 29},
    {'5', 30},
    {'6', 31},
    {'7', 32},
    {'8', 33},
    {'9', 34},
    {'0', 35},
    {'\'',36},
    {'.', 37},
    {'!', 38},
    {'%', 39},
    {'<', 40},
    {'>', 41},
    {'?', 42},
    {':', 43},
    // {'�', 44},
    {'_', 45},
    {'-', 46},
    {'(', 47},
    {')', 48},
    {'#', 49},
    {'+', 50},
    {'`', 51}
};

}
