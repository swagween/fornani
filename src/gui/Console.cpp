#include "Console.hpp"
//edge_dim
//  Console.cpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#include "Console.hpp"
#include "../setup/ServiceLocator.hpp"

namespace gui {

	Console::Console() {
		for(auto& sprite : sprites) {
			sprite.setTexture(svc::assetLocator.get().t_ui);
		}
		sprites.at(0).setTextureRect(sf::IntRect{ {0, 0}, {corner_factor, corner_factor} });
		sprites.at(1).setTextureRect(sf::IntRect{ {corner_factor, 0}, {edge_factor, corner_factor} });
		sprites.at(2).setTextureRect(sf::IntRect{ {corner_factor + edge_factor, 0}, {corner_factor, corner_factor} });
		sprites.at(3).setTextureRect(sf::IntRect{ {0, corner_factor}, {corner_factor, edge_factor} });
		sprites.at(4).setTextureRect(sf::IntRect{ {corner_factor, corner_factor}, {edge_factor, edge_factor} });
		sprites.at(5).setTextureRect(sf::IntRect{ {corner_factor + edge_factor, corner_factor}, {corner_factor, edge_factor} });
		sprites.at(6).setTextureRect(sf::IntRect{ {0, corner_factor + edge_factor}, {corner_factor, corner_factor} });
		sprites.at(7).setTextureRect(sf::IntRect{ {corner_factor, corner_factor + edge_factor}, {edge_factor, corner_factor} });
		sprites.at(8).setTextureRect(sf::IntRect{ {corner_factor + edge_factor, corner_factor + edge_factor}, {corner_factor, corner_factor} });

		dimensions = sf::Vector2<float>{ (float)cam::screen_dimensions.x - 2 * pad, (float)cam::screen_dimensions.y / height_factor };
		position = sf::Vector2<float>{ origin.x, origin.y - dimensions.y };
		text_origin = sf::Vector2<float>{ 20.0f, 20.0f };

		extent = corner_factor * 2;
	}

	void Console::begin() {
		dimensions.y = corner_factor * 2;
	}

	void Console::update() {
		if (flags.test(ConsoleFlags::active)) { extent += speed; }
		if(extent < (float)cam::screen_dimensions.y / height_factor) {
			dimensions.y = extent;
		} else {
			dimensions.y = (float)cam::screen_dimensions.y / height_factor;
		}
		nine_slice(corner_factor, edge_factor);
		
	}

	void Console::render(sf::RenderWindow& win) {
		for(auto& sprite : sprites) {
			win.draw(sprite);
		}
	}

	void Console::write(sf::RenderWindow& win, std::string_view message) {
		int ctr{ 0 };
		for (auto& letter : message) {
			if (!std::isspace(letter)) {
				try {
					if (auto search = lookup::get_character.find(letter); search != lookup::get_character.end()) {
						svc::assetLocator.get().sp_alphabet.at(lookup::get_character.at(letter)).setPosition(position.x + text_origin.x + (12 * ctr) + 2, position.y + text_origin.y);
						win.draw(svc::assetLocator.get().sp_alphabet.at(lookup::get_character.at(letter)));
					} else {
						printf("Error: %c is not in the map.\n", letter);
					}
				} catch (std::out_of_range) {
					printf("Error: %c is out of range.\n", letter);
				}
			}
			ctr++;
		}
	}

	void Console::write_speech(sf::RenderWindow& win, std::string_view message) {

	}

	void Console::end() {
		extent = dimensions.y = corner_factor * 2;
		flags.reset(ConsoleFlags::active);
	}

	void Console::nine_slice(int corner_dim, int edge_dim) {

		//set sizes for stretched 9-slice sprites
		sprites.at(1).setScale({ (dimensions.x - 2 * corner_dim) / edge_dim, 1 });
		sprites.at(3).setScale(1, (dimensions.y - 2 * corner_dim) / edge_dim);
		sprites.at(4).setScale((dimensions.x - 2 * corner_dim) / edge_dim, (dimensions.y - 2 * corner_dim) / edge_dim);
		sprites.at(5).setScale(1, (dimensions.y - 2 * corner_dim) / edge_dim);
		sprites.at(7).setScale((dimensions.x - 2 * corner_dim) / edge_dim, 1);

		//set position for the 9-slice console box
		sprites.at(0).setPosition(position.x, position.y);
		sprites.at(1).setPosition(position.x + corner_dim, position.y);
		sprites.at(2).setPosition(position.x + dimensions.x - corner_dim, position.y);

		sprites.at(3).setPosition(position.x, position.y + corner_dim);
		sprites.at(4).setPosition(position.x + corner_dim, position.y + corner_dim);
		sprites.at(5).setPosition(position.x + dimensions.x - corner_dim, position.y + corner_dim);

		sprites.at(6).setPosition(position.x, position.y + dimensions.y - corner_dim);
		sprites.at(7).setPosition(position.x + corner_dim, position.y + dimensions.y - corner_dim);
		sprites.at(8).setPosition(position.x + dimensions.x - corner_dim, position.y + dimensions.y - corner_dim);

	}

} // end gui

 /* Console_cpp */
