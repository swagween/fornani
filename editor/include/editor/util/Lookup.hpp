
#pragma once

#include <stdio.h>
#include <string>
#include <unordered_map>
#include "editor/canvas/Tile.hpp"
#include "editor/canvas/Canvas.hpp"
#include "editor/tool/Tool.hpp"

namespace pi {

inline std::unordered_map<ToolType, std::string> get_tool_string{{ToolType::brush, "Brush"}, {ToolType::hand, "Hand"},	   {ToolType::fill, "Fill"},
																 {ToolType::erase, "Erase"}, {ToolType::marquee, "Select"}, {ToolType::entity_editor, "Entity Placer"}};

inline std::unordered_map<Style, char const*> get_style_string{{Style::firstwind, "firstwind"}, {Style::overturned, "overturned"}, {Style::base, "base"},
															   {Style::factory, "factory"},		{Style::greatwing, "greatwing"},   {Style::provisional, "provisional"}, {Style::END, "END"}};

inline std::unordered_map<Backdrop, char const*> get_backdrop_string{
	{Backdrop::BG_DUSK, "DUSK"},		 {Backdrop::BG_SUNRISE, "SUNRISE"}, {Backdrop::BG_OPEN_SKY, "OPEN_SKY"}, {Backdrop::BG_ROSY_HAZE, "ROSY_HAZE"}, {Backdrop::BG_DAWN, "DAWN"}, {Backdrop::BG_NIGHT, "NIGHT"},
	{Backdrop::BG_OVERCAST, "OVERCAST"}, {Backdrop::BG_SLIME, "SLIME"},		{Backdrop::BG_BLACK, "BLACK"},		 {Backdrop::BG_NAVY, "NAVY"},			{Backdrop::BG_DIRT, "DIRT"}, {Backdrop::BG_GEAR, "GEAR"},
	{Backdrop::BG_LIBRARY, "LIBRARY"},	 {Backdrop::BG_GRANITE, "GRANITE"}, {Backdrop::BG_RUINS, "RUINS"},		 {Backdrop::BG_CREVASSE, "CREVASSE"},	{Backdrop::BG_DEEP, "DEEP"}, {Backdrop::BG_GROVE, "GROVE"}};

inline std::unordered_map<int, char const*> layer_name{
	{0, "Background 0"}, {1, "Background 1"}, {2, "Background 2"}, {3, "Background 3"}, {4, "Middleground / Collidable"}, {5, "Foreground 1"}, {6, "Foreground 2"}, {7, "Foreground 3"},
};

}

