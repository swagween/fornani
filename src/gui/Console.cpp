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

		dimensions = sf::Vector2<float>{ (float)cam::screen_dimensions.x - 2 * pad, (float)cam::screen_dimensions.y / 3 };
		position = sf::Vector2<float>{ origin.x, origin.y - dimensions.y };
	}

	void Console::begin(std::string_view message) {
		dimensions.x = corner_factor * 2;
	}

	void Console::update() {
		if (flags.test(ConsoleFlags::active)) { extent += speed; }
		if(extent < (float)cam::screen_dimensions.x - 2 * pad) {
			dimensions.x = extent;
		} else {
			dimensions.x = (float)cam::screen_dimensions.x - 2 * pad;
		}
		nine_slice(corner_factor, edge_factor);
		
	}

	void Console::render(sf::RenderWindow& win) {
		for(auto& sprite : sprites) {
			win.draw(sprite);
		}
	}

	void Console::write(std::string_view message) {
	}

	void Console::write_speech(std::string_view message) {
	}

	void Console::end() {
		extent = 0;
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
