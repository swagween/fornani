
#include "Spark.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Spark::Spark(sf::Vector2<float> pos, sf::Color color) {
	position = pos;
	box.setFillColor(color);
	box.setSize({3.f, 3.f});
	lifespan.start(100);
}

void Spark::update() {
	++frame;
	position.x += 0.1 * sin(0.02f * frame);
	if (svc::tickerLocator.get().every_x_frames(2)) { --position.y; }
		
	lifespan.update();
}

void Spark::render(sf::RenderWindow& win, sf::Vector2<float> cam) { 
	
	box.setPosition(position - cam);
	win.draw(box);
}

bool Spark::done() const { return lifespan.is_complete(); }


} // namespace vfx