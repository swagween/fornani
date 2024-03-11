
#include "Critter.hpp"
#include "../../utils/Random.hpp"

namespace critter {

void Critter::sprite_flip() {
	if (flags.test(Flags::flip)) {
		sprite.scale(-1.0f, 1.0f);
		flags.reset(Flags::flip);
	}
	// flip the sprite based on the critter's direction
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	direction.lr == dir::LR::left ? sprite.setScale(left_scale) : sprite.setScale(right_scale);
}

void Critter::init() {

	set_sprite();

	unique_id = svc::randomLocator.get().random_range(-2147483647, 2147483647);

	condition.hp = stats.base_hp;
}

void Critter::update() {

	unique_update();
	behavior.update();
	weapon.firing_direction = direction;

	if (flags.test(Flags::seeking)) { seek_current_target(); }

	if (!colliders.empty()) { sprite_position = colliders.at(0).physics.position + sprite.getOrigin() - colliders.at(0).sprite_offset; }

	for (auto& collider : colliders) {
		collider.physics.update_euler();
		collider.sync_components();
		collider.update();
	}

	// get UV coords
	int u = (int)(behavior.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
	int v = (int)(behavior.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
	sprite.setOrigin(sprite_dimensions.x / 2, dimensions.y / 2);

	int ctr{0};
	for (auto& hbx : hurtboxes) {
		hbx = hurtbox_atlas.at((int)(behavior.get_frame() * num_hurtboxes + ctr));
		hbx.update();
		if (direction.lr == dir::LR::right) {
			hbx.set_position({sprite_position.x + hbx.sprite_offset.x - sprite.getOrigin().x, sprite_position.y - sprite.getOrigin().y + hbx.sprite_offset.y});
		} else if (direction.lr == dir::LR::left) {
			hbx.set_position({sprite_position.x - hbx.sprite_offset.x + sprite.getOrigin().x - hbx.dimensions.x, sprite_position.y - sprite.getOrigin().y + hbx.sprite_offset.y});
		}
		++ctr;
	}
}

void Critter::render(sf::RenderWindow& win, sf::Vector2<float> campos) {
	sprite.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);
	drawbox.setSize(dimensions);

	ar.setSize({(float)(alert_range.dimensions.x), (float)(alert_range.dimensions.y)});
	hr.setSize({(float)hostile_range.dimensions.x, (float)hostile_range.dimensions.y});
	ar.setPosition(alert_range.position.x - campos.x, alert_range.position.y - campos.y);
	hr.setPosition(hostile_range.position.x - campos.x, hostile_range.position.y - campos.y);
	win.draw(sprite);
	

	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		ar.setFillColor(sf::Color{80, 80, 20, 60});
		hr.setFillColor(sf::Color{80, 40, 20, 60});
		ar.setOutlineColor(sf::Color{180, 180, 180});
		hr.setOutlineColor(sf::Color{180, 180, 180});
		ar.setOutlineThickness(-1);
		hr.setOutlineThickness(-1);
		// debug
		drawbox.setPosition(sprite_position.x - sprite.getOrigin().x - campos.x, sprite_position.y - sprite.getOrigin().y - campos.y);
		drawbox.setSize({(float)sprite_dimensions.x, (float)sprite_dimensions.y});
		drawbox.setFillColor(sf::Color::Transparent);
		drawbox.setOutlineColor(flcolor::fucshia);
		drawbox.setOutlineThickness(-1);
		win.draw(drawbox);
		for (auto& collider : colliders) { collider.render(win, campos); }
		for (auto& hbx : hurtboxes) {
			drawbox.setFillColor(sf::Color{255, 255, 20, 20});
			drawbox.setOutlineColor(flcolor::goldenrod);
			drawbox.setOutlineThickness(-1);
			drawbox.setSize(hbx.dimensions);
			drawbox.setPosition(hbx.position.x - campos.x, hbx.position.y - campos.y);
			win.draw(drawbox);
		}
		win.draw(ar);
		win.draw(hr);
	}
	sprite_flip();

	// draw health for debug
	hpbox.setPosition({sprite_position.x - campos.x, sprite_position.y - 14.f - campos.y});
	hpbox.setFillColor(sf::Color{29, 118, 112});
	hpbox.setSize({stats.base_hp, 4.f});
	win.draw(hpbox);
	hpbox.setFillColor(sf::Color{0, 228, 185});
	hpbox.setSize({condition.hp, 4.f});
	win.draw(hpbox);
}

void Critter::set_sprite() {
	try {
		sprite.setTexture(get_critter_texture.at(metadata.id));
	} catch (std::out_of_range) {
		printf("Failed to set sprite for critter.\n");
		return;
	}
}

void Critter::set_position(sf::Vector2<int> pos) {

	for (auto& collider : colliders) {
		sprite_position = static_cast<sf::Vector2<float>>(pos);
		collider.physics.position = sprite_position + collider.sprite_offset;
		collider.sync_components();
	}
}

void Critter::seek_current_target() {
	sf::Vector2<float> desired = current_target - colliders.at(0).physics.position;
	desired *= stats.speed;
	sf::Vector2<float> steering = desired - colliders.at(0).physics.velocity;
	if (abs(steering.x) < 0.5) {
		colliders.at(0).physics.acceleration.x = 0.0f;
		return;
	}
	steering *= 0.08f;
	if (flags.test(Flags::running) || flags.test(Flags::seeking)) { colliders.at(0).physics.acceleration.x = steering.x; }
}
void Critter::wake_up() {
	flags.reset(Flags::asleep);
	flags.set(Flags::awakened);
	flags.reset(Flags::awake);
}
void Critter::sleep() {
	flags.set(Flags::asleep);
	flags.reset(Flags::awakened);
	flags.reset(Flags::awake);
}

void Critter::awake() {
	flags.reset(Flags::asleep);
	flags.reset(Flags::awakened);
	flags.set(Flags::awake);
}

void Critter::cooldown() {

	--stats.cooldown;

	if (stats.cooldown < 0) { stats.cooldown = 0; }
}

} // namespace critter
