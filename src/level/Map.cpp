
#include "Map.hpp"
#include <imgui.h>
#include "../setup/EnumLookups.hpp"
#include "../utils/Math.hpp"
#include "../setup/ServiceLocator.hpp"

namespace world {

void Map::load(std::string const& path, services::ServiceLocator& svc, player::Player& player) {

	std::string filepath = path + "/map_data.txt";

	int value{};
	int counter = 0;
	std::ifstream input{};
	input.open(filepath);
	if (!input.is_open()) {
		printf("Failed to open file.\n");
		return;
	}

	// dimensions and layers
	input >> value;
	room_id = value;
	input.ignore();
	input >> value;
	dimensions.x = value;
	input.ignore();
	input >> value;
	dimensions.y = value;
	input.ignore();
	input >> value;
	chunk_dimensions.x = value;
	input.ignore();
	input >> value;
	chunk_dimensions.y = value;
	input.ignore();
	if ((dimensions.x / chunk_dimensions.x != CHUNK_SIZE) || (dimensions.y / chunk_dimensions.y != CHUNK_SIZE)) {
		printf("File is corrupted: Invalid dimensions.\n");
		return;
	}
	real_dimensions = {(float)dimensions.x * CELL_SIZE, (float)dimensions.y * CELL_SIZE};
	for (int i = 0; i < NUM_LAYERS; ++i) { layers.push_back(Layer(i, (i == MIDDLEGROUND), dimensions)); }
	// style
	input >> value;
	input.ignore();
	if (value >= lookup::get_style.size()) {
		printf("File is corrupted: Invalid style.\n");
		return;
	} else {
		style = lookup::get_style.at(value);
	}
	// bg;
	input >> value;
	bg = value;
	background = std::make_unique<bg::Background>(bg::bg_behavior_lookup.at(bg), bg);
	input.close();

	// get map tiles from text files
	for (auto& layer : layers) {
		input.open(path + "/map_tiles_" + std::to_string(counter) + ".txt");
		for (auto& cell : layer.grid.cells) {
			input >> value;
			lookup::TILE_TYPE typ = lookup::tile_lookup.at(value);
			cell.value = value;
			cell.type = typ;

			input.ignore(); // ignore the delimiter
		}
		layer.grid.update();
		// close the current file
		input.close();
		++counter;
	}

	// get portal data
	input.open(path + "/map_portals.txt");
	if (input.is_open()) {
		while (!input.eof()) {
			entity::Portal p{};
			input >> p.scaled_dimensions.x;
			input.ignore();
			input >> p.scaled_dimensions.y;
			input.ignore();
			input >> value;
			p.activate_on_contact = (bool)value;
			input.ignore();
			input >> p.source_map_id;
			input.ignore();
			input >> p.destination_map_id;
			input.ignore();
			input >> p.scaled_position.x;
			input.ignore();
			input >> p.scaled_position.y;
			input.ignore();
			p.update();
			if (p.dimensions.x != 0) { // only push if one was read, otherwise we reached the end of the file
				portals.push_back(p);
				portals.back().update();
			}
		}
		input.close();
	}

	// get inspectable data
	input.open(path + "/map_inspectables.txt");
	if (input.is_open()) {
		while (!input.eof()) {
			entity::Inspectable p{};
			input >> p.scaled_dimensions.x;
			input.ignore();
			input >> p.scaled_dimensions.y;
			input.ignore();
			input >> value;
			p.activate_on_contact = (bool)value;
			input.ignore();
			input.ignore();
			std::getline(input, p.key, '#');
			input >> p.scaled_position.x;
			input.ignore();
			input >> p.scaled_position.y;
			input.ignore();
			p.update();
			if (p.dimensions.x != 0) { // only push if one was read, otherwise we reached the end of the file
				inspectables.push_back(p);
				inspectables.back().update();
			}
		}
		input.close();
	}

	// get animator data
	input.open(path + "/map_animators.txt");
	if (input.is_open()) {
		while (!input.eof()) {
			entity::Animator p{};
			input >> p.scaled_dimensions.x;
			input.ignore();
			input >> p.scaled_dimensions.y;
			input.ignore();
			input >> value;
			p.id = value;
			input.ignore();
			input >> value;
			p.automatic = (bool)value;
			input.ignore();
			input >> value;
			p.foreground = (bool)value;
			input.ignore();
			input >> p.scaled_position.x;
			input.ignore();
			input >> p.scaled_position.y;
			input.ignore();
			if (p.scaled_dimensions.x != 0) { // only push if one was read, otherwise we reached the end of the file
				uint32_t large_dim = 16;
				p.dimensions = static_cast<Vec>(p.scaled_dimensions * large_dim);
				p.bounding_box = shape::Shape(p.dimensions);
				animators.push_back(p);
			}
		}
		input.close();
	}

	// get save point data
	input.open(path + "/map_save_point.txt");
	if (input.is_open()) {
		while (!input.eof()) {
			input >> value;
			save_point.id = value;
			input.ignore();
			input >> save_point.scaled_position.x;
			input.ignore();
			input >> save_point.scaled_position.y;
			input.ignore();
		}
		input.close();
	}

	// get critter data
	// zero the pool_counter
	critter::pool_counter.fill(0);
	input.open(path + "/map_critters.txt");
	if (input.is_open()) {
		while (!input.eof()) {

			int id{};
			sf::Vector2<int> pos{};

			// extract id and position
			input >> id;
			input.ignore();
			input >> pos.x;
			input.ignore();
			input >> pos.y;
			input.ignore();

			// fetch the type
			critter::CRITTER_TYPE type = critter::get_critter_type.at(id);

			// push the critter
			// which type of critter? and how deep into the pool are we?
			critters.push_back(*bestiary.fetch_critter_of_type(type, critter::pool_counter.at(id)));
			critters.back()->load_data();
			critters.back()->set_position({pos.x * asset::TILE_WIDTH, pos.y * asset::TILE_WIDTH});
			for (auto& collider : critters.back()->colliders) { collider.physics.zero(); }
			critter::pool_counter.at(id)++;
		}
		input.close();
	}

	for (auto& critter : critters) {
		for (auto& collider : critter->colliders) { colliders.push_back(&collider); }
	}
	colliders.push_back(&player.collider);
	// for (auto& a : player.antennae) { colliders.push_back(&a.collider); }

	transition.fade_in = true;
	minimap = sf::View(sf::FloatRect(0.0f, 0.0f, cam::screen_dimensions.x * 2, cam::screen_dimensions.y * 2));
	minimap.setViewport(sf::FloatRect(0.0f, 0.75f, 0.2f, 0.2f));

	generate_collidable_layer();
}

void Map::update(services::ServiceLocator& svc, player::Player& player) {

	svc.consoleLocator.get().update();

	player.collider.reset();
	for (auto& a : player.antennae) { a.collider.reset(); }

	for (auto& critter : critters) {
		for (auto& collider : critter->colliders) { collider.reset(); }
	}

	manage_projectiles(svc);

	// someday, I will have a for(auto& entity : entities) loop and the player will be included in that
	for (auto& collider : colliders) {
		for (auto& index : collidable_indeces) {
			auto& cell = layers.at(MIDDLEGROUND).grid.cells.at(index);
			cell.collision_check = false;
			if (!nearby(cell.bounding_box, collider->bounding_box)) {
				continue;
			} else {
				// check vicinity so we can escape early
				if (!collider->vicinity.overlaps(cell.bounding_box)) {
					continue;
				} else {
					cell.collision_check = true;
					if (cell.value > 0) { collider->handle_map_collision(cell.bounding_box, cell.type); }
				}
			}
		}
	}

	// i need to refactor this...
	for (auto& index : collidable_indeces) {
		auto& cell = layers.at(MIDDLEGROUND).grid.cells.at(index);
		for (auto& emitter : active_emitters) {
			for (auto& particle : emitter.get_particles()) {
				if (!nearby(cell.bounding_box, particle.bounding_box)) {
					continue;
				} else {
					cell.collision_check = true;
					if (particle.bounding_box.overlaps(cell.bounding_box) && cell.value > 0) {
						shape::Shape::Vec mtv = particle.bounding_box.testCollisionGetMTV(particle.bounding_box, cell.bounding_box);
						sf::operator+=(particle.physics.position, mtv);
						particle.physics.acceleration.y *= -1.0f;
						particle.physics.acceleration.x *= -1.0f;
						if (abs(mtv.y) > abs(mtv.x)) { particle.physics.velocity.y *= -1.0f; }
						if (abs(mtv.x) > abs(mtv.y)) { particle.physics.velocity.x *= -1.0f; }
					}
				}
			}
		}
		// damage player if spikes
		if (cell.type == lookup::TILE_TYPE::TILE_SPIKES && player.collider.hurtbox.overlaps(cell.bounding_box)) { player.hurt(1); }
		if (cell.type == lookup::TILE_TYPE::TILE_DEATH_SPIKES && player.collider.hurtbox.overlaps(cell.bounding_box)) { player.hurt(64); }
		for (auto& proj : active_projectiles) {
			if (!nearby(cell.bounding_box, proj.bounding_box)) {
				continue;
			} else {
				cell.collision_check = true;
				if ((proj.bounding_box.overlaps(cell.bounding_box) && cell.is_occupied())) {
					if (cell.type == lookup::TILE_TYPE::TILE_BREAKABLE && !proj.stats.transcendent) {
						--cell.value;
						if (lookup::tile_lookup.at(cell.value) != lookup::TILE_TYPE::TILE_BREAKABLE) {
							cell.value = 0;
							// i need to not do this here
							//active_emitters.push_back(breakable_debris);
							active_emitters.back().get_physics().acceleration += proj.physics.acceleration;
							active_emitters.back().set_position(cell.position.x + CELL_SIZE / 2.f, cell.position.y + CELL_SIZE / 2.f);
							active_emitters.back().set_direction(proj.direction);
							active_emitters.back().update(svc);
							//svc.assetLocator.get().shatter.play();
						}
					}
					if (cell.type == lookup::TILE_TYPE::TILE_PLATFORM || cell.type == lookup::TILE_TYPE::TILE_SPIKES) { continue; }
					if (!proj.stats.transcendent) { proj.destroy(false); }
					if (proj.stats.spring && cell.is_hookable()) {
						if (proj.hook.grapple_flags.test(arms::GrappleState::probing)) {
							proj.hook.spring.set_anchor(cell.middle_point());
							proj.hook.grapple_triggers.set(arms::GrappleTriggers::found);
						}
						handle_grappling_hook(svc, proj);
					}
				}
			}
		}
	}

	for (auto& proj : active_projectiles) {
		if (proj.state.test(arms::ProjectileState::destruction_initiated)) { continue; }
		for (auto& critter : critters) {
			for (auto& hurtbox : critter->hurtboxes) {
				if (proj.team != arms::TEAMS::SKYCORPS) {
					if (proj.bounding_box.overlaps(hurtbox)) {
						critter->flags.set(critter::Flags::shot);
						if (critter->flags.test(critter::Flags::vulnerable)) {
							critter->flags.set(critter::Flags::hurt);
							critter->flags.set(critter::Flags::just_hurt);
							critter->condition.hp -= proj.stats.base_damage;
						}
						if (!proj.stats.persistent) { proj.destroy(false); }
					}
				}
			}
		}
		if (proj.bounding_box.overlaps(player.collider.hurtbox) && proj.team != arms::TEAMS::NANI) {
			player.hurt(svc, proj.stats.base_damage);
			proj.destroy(false);
		}
	}

	for (auto& critter : critters) {

		// handle collision
		for (auto& collider : critter->colliders) {
			player.collider.handle_collider_collision(collider.bounding_box);
			for (auto& other_critter : critters) {
				if (!(other_critter == critter)) {
					for (auto& other_collider : other_critter->colliders) { collider.handle_collider_collision(other_collider.bounding_box); }
				}
			}
		}
		if (!critter->colliders.empty()) {
			critter->direction.lr = (player.collider.physics.position.x < critter->colliders.at(0).physics.position.x) ? dir::LR::right : dir::LR::left;
			if (critter->flags.test(critter::Flags::seeking)) { critter->direction.lr = critter->colliders.at(0).physics.direction.lr; }
		}
	}

	for (auto& collider : colliders) { collider->reset_ground_flags(); }

	for (auto& portal : portals) {
		portal.update();
		portal.handle_activation(room_id, transition.fade_out, transition.done);
	}

	for (auto& inspectable : inspectables) {
		if (player.controller.inspecting() && inspectable.bounding_box.overlaps(player.collider.hurtbox)) {
			inspectable.activated = true;
			svc.consoleLocator.get().flags.set(gui::ConsoleFlags::active);
		}
		if (inspectable.activated && svc.consoleLocator.get().flags.test(gui::ConsoleFlags::active)) {
			svc.consoleLocator.get().begin();
			if (player.controller.transponder_exit()) {
				inspectable.activated = false;
				svc.consoleLocator.get().end();
			}
		}
	}

	for (auto& animator : animators) {
		if (animator.bounding_box.overlaps(player.collider.bounding_box) && player.controller.moving()) {
			animator.anim.on();
			animator.activated = true;
		} else {
			animator.activated = false;
		}
		animator.update();
	}

	if (save_point.id != -1) { save_point.update(); }

	// check if player died
	if (!player.flags.state.test(player::State::alive) && !game_over) {
		active_emitters.push_back(player_death);
		active_emitters.back().get_physics().acceleration += player.collider.physics.acceleration;
		active_emitters.back().set_position(player.collider.physics.position.x, player.collider.physics.position.y);
		active_emitters.back().set_direction(dir::Direction{});
		active_emitters.back().update();
		svc.assetLocator.get().player_death.play();
		game_over = true;
	}

	if (game_over) {
		transition.fade_out = true;
		if (transition.done) {
			player.start_over();
			svc.stateControllerLocator.get().next_state = lookup::get_map_label.at(101); // temporary. later, we will load the last save
			svc.stateControllerLocator.get().trigger = true;
			player.set_position(sf::Vector2<float>(200.f, 390.f));
		}
	}

	if (svc.tickerLocator.get().every_x_frames(1)) { transition.update(); }
}

void Map::render(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam, services::ServiceLocator& svc) {
	for (auto& proj : active_projectiles) {
		proj.render(win, cam);
		if (proj.hook.grapple_flags.test(arms::GrappleState::anchored)) { proj.hook.spring.render(win, cam); }
	}

	// emitters
	for (auto& emitter : active_emitters) { emitter.render(win, cam); }

	// player
	player.render(win, camera.physics.position);

	// enemies
	for (auto& critter : critters) {
		if (camera.within_frame(critter->sprite_position.x, critter->sprite_position.y)) { critter->render(win, cam); }
	}

	// foreground animators
	for (auto& animator : animators) {
		if (!animator.foreground) { animator.render(win, cam); }
	}

	if (save_point.id != -1) { save_point.render(win, cam); }

	// level foreground
	for (auto& layer : layers) {
		if (layer.render_order >= 4) {
			for (auto& cell : layer.grid.cells) {
				if (cell.is_occupied()) {
					if (!svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state) || layer.render_order == 4) {
						int cell_x = cell.bounding_box.position.x - cam.x;
						int cell_y = cell.bounding_box.position.y - cam.y;
						tileset.at(cell.value).setPosition(cell_x, cell_y);
						if (camera.within_frame(cell_x + CELL_SIZE, cell_y + CELL_SIZE)) {
							win.draw(tileset.at(cell.value));
							
						}
					}
					cell.render(win, cam);
				}
			}
		}
	}

	if (real_dimensions.y < cam::screen_dimensions.y) {
		float ydiff = (cam::screen_dimensions.y - real_dimensions.y) / 2;
		borderbox.setFillColor(flcolor::black);
		borderbox.setSize({(float)cam::screen_dimensions.x, ydiff});
		borderbox.setPosition(0.0f, 0.0f);
		win.draw(borderbox);
		
		borderbox.setPosition(0.0f, real_dimensions.y + ydiff);
		win.draw(borderbox);
		
	}
	if (real_dimensions.x < cam::screen_dimensions.x) {
		float xdiff = (cam::screen_dimensions.x - real_dimensions.x) / 2;
		borderbox.setFillColor(flcolor::black);
		borderbox.setSize({xdiff, (float)cam::screen_dimensions.y});
		borderbox.setPosition(0.0f, 0.0f);
		win.draw(borderbox);
		
		borderbox.setPosition(real_dimensions.x + xdiff, 0.0f);
		win.draw(borderbox);
		
	}

	for (auto& portal : portals) { portal.render(win, cam); }
	for (auto& inspectable : inspectables) {
		inspectable.render(win, cam); // for debug
	}
	for (auto& animator : animators) {
		if (animator.foreground) { animator.render(win, cam); }
	}

	// render minimap
	if (show_minimap) {
		win.setView(minimap);
		for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
			minimap_tile.setPosition(cell.position.x - cam.x, cell.position.y - cam.y);
			minimap_tile.setSize(sf::Vector2<float>{(float)cell.bounding_box.dimensions.x, (float)cell.bounding_box.dimensions.y});
			if (cell.value > 0) {
				minimap_tile.setFillColor(sf::Color{20, 240, 20, 120});
				win.draw(minimap_tile);
				
			} else {
				minimap_tile.setFillColor(sf::Color{20, 20, 20, 120});
				win.draw(minimap_tile);
				
			}
		}
		minimap_tile.setPosition(player.collider.physics.position.x - cam.x, player.collider.physics.position.y - cam.y);
		minimap_tile.setFillColor(sf::Color{240, 240, 240, 180});
		win.draw(minimap_tile);
		
		win.setView(sf::View(sf::FloatRect{0.f, 0.f, (float)cam::screen_dimensions.x, (float)cam::screen_dimensions.y}));
	}
}

void Map::render_background(sf::RenderWindow& win, std::vector<sf::Sprite>& tileset, sf::Vector2<float> cam, services::ServiceLocator& svc) {
	if (!svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state)) {
		background->render(win, cam, real_dimensions);
	} else {
		sf::RectangleShape box{};
		box.setPosition(0, 0);
		box.setFillColor(flcolor::black);
		box.setSize({(float)cam::screen_dimensions.x, (float)cam::screen_dimensions.y});
		win.draw(box);
		
	}
	if (real_dimensions.y < cam::screen_dimensions.y) { camera.fix_horizontally(real_dimensions); }
	for (auto& layer : layers) {
		if (layer.render_order < 4) {
			for (auto& cell : layer.grid.cells) {
				if (cell.is_occupied()) {
					int cell_x = cell.bounding_box.position.x - cam.x;
					int cell_y = cell.bounding_box.position.y - cam.y;
					tileset.at(cell.value).setPosition(cell_x, cell_y);
					if (!svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state)) {
						if (camera.within_frame(cell_x + CELL_SIZE, cell_y + CELL_SIZE)) {
							win.draw(tileset.at(cell.value));
							
						}
					}
				}
			}
		}
	}
}

void Map::render_console(sf::RenderWindow& win, services::ServiceLocator& svc) {
	if (svc.consoleLocator.get().flags.test(gui::ConsoleFlags::active)) {
		svc.consoleLocator.get().render(win);
		for (auto& inspectable : inspectables) {
			if (inspectable.activated) {
				svc.consoleLocator.get().load_and_launch(inspectable.key);
				svc.consoleLocator.get().write(win);
				// svc.consoleLocator.get().write(win, inspectable.message);
				//  svc.consoleLocator.get().write(win, "ab?:-_()#`");
			}
		}
	}
	svc.consoleLocator.get().write(win, false);
}

void Map::spawn_projectile_at(sf::Vector2<float> pos, services::ServiceLocator& svc, player::Player& player) {
	active_projectiles.push_back(player.equipped_weapon().projectile);
	active_projectiles.back().set_sprite();
	active_projectiles.back().set_position(pos);
	active_projectiles.back().seed();
	active_projectiles.back().update(svc);
	active_projectiles.back().sync_position();
	if (active_projectiles.back().stats.boomerang) { active_projectiles.back().set_boomerang_speed(); }
	if (active_projectiles.back().stats.spring) {
		active_projectiles.back().set_hook_speed();
		active_projectiles.back().hook.grapple_flags.set(arms::GrappleState::probing);
	}

	active_emitters.push_back(player.equipped_weapon().spray);
	active_emitters.back().get_physics().acceleration += player.collider.physics.acceleration;
	active_emitters.back().set_position(pos.x, pos.y);
	active_emitters.back().set_direction(player.equipped_weapon().firing_direction);
	active_emitters.back().update(svc);
}

void Map::spawn_critter_projectile_at(sf::Vector2<float> pos, critter::Critter& critter, services::ServiceLocator& svc) {
	active_projectiles.push_back(critter.weapon.projectile);
	active_projectiles.back().fired_point = pos;
	active_projectiles.back().set_sprite();
	active_projectiles.back().physics.position = pos;
	active_projectiles.back().seed();
	active_projectiles.back().update(svc);

	active_emitters.push_back(critter.weapon.spray);
	active_emitters.back().get_physics().acceleration += critter.colliders.at(0).physics.acceleration;
	active_emitters.back().set_position(pos.x, pos.y);
	active_emitters.back().set_direction(critter.weapon.firing_direction);
	active_emitters.back().update(svc);
	critter.flags.reset(critter::Flags::weapon_fired);
}

void Map::manage_projectiles(services::ServiceLocator& svc, player::Player& player) {
	for (auto& proj : active_projectiles) { proj.update(svc); }
	for (auto& spray : active_emitters) { spray.update(svc); }

	std::erase_if(active_projectiles, [](auto const& p) {
		if (p.state.test(arms::ProjectileState::destroyed)) {
			//--player.extant_instances(lookup::type_to_index.at(p.type));
			return true;
		} else {
			return false;
		}
	});
	std::erase_if(active_emitters, [](auto const& p) { return p.particles.empty(); });
	std::erase_if(critters, [](auto const& c) { return c->condition.hp <= 0; });

	if (!player.arsenal.loadout.empty()) {
		if (player.fire_weapon(svc)) {
			spawn_projectile_at(player.equipped_weapon().barrel_point);
			++player.equipped_weapon().active_projectiles;
			player.equipped_weapon().shoot();
			if (!player.equipped_weapon().attributes.automatic) { player.controller.set_shot(false); }
		}
	}

	for (auto& critter : critters) {
		if (critter->flags.test(critter::Flags::weapon_fired)) { spawn_critter_projectile_at(critter->barrel_point, *critter, svc); }
	}
}

void Map::generate_collidable_layer() {
	layers.at(MIDDLEGROUND).grid.check_neighbors();
	for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
		if (!cell.surrounded && cell.is_occupied()) { collidable_indeces.push_back(cell.one_d_index); }
	}
}

bool Map::check_cell_collision(shape::Collider collider) {
	for (auto& index : collidable_indeces) {
		auto& cell = layers.at(MIDDLEGROUND).grid.cells.at(index);
		if (!nearby(cell.bounding_box, collider.bounding_box)) {
			continue;
		} else {
			// check vicinity so we can escape early
			if (!collider.vicinity.overlaps(cell.bounding_box)) {
				continue;
			} else if(!cell.is_solid()) {
				continue;
			} else {
				if (cell.value > 0 && collider.predictive_combined.SAT(cell.bounding_box)) { return true; }
			}
		}
	}
	return false;
}

void Map::handle_grappling_hook(arms::Projectile& proj, player::Player& player) {
	// do this first block once
	if (proj.hook.grapple_triggers.test(arms::GrappleTriggers::found) && !proj.hook.grapple_flags.test(arms::GrappleState::anchored) && !proj.hook.grapple_flags.test(arms::GrappleState::snaking)) {
		proj.hook.spring.set_bob(player.apparent_position);
		proj.hook.grapple_triggers.reset(arms::GrappleTriggers::found);
		proj.hook.grapple_flags.set(arms::GrappleState::anchored);
		proj.hook.grapple_flags.reset(arms::GrappleState::probing);
		proj.hook.spring.set_force(proj.stats.spring_constant);
		proj.hook.spring.variables.physics.acceleration += player.collider.physics.acceleration;
		proj.hook.spring.variables.physics.velocity += player.collider.physics.velocity;
	}
	if (player.controller.hook_held() && proj.hook.grapple_flags.test(arms::GrappleState::anchored)) {
		proj.hook.spring.variables.physics.acceleration += player.collider.physics.acceleration;
		proj.hook.spring.variables.physics.acceleration.x += player.controller.horizontal_movement();
		proj.lock_to_anchor();
		proj.hook.spring.update();

		//shorthand
		auto& player_collider = player.collider;

		//update rest length
		auto next_length = proj.stats.spring_slack * abs(player.collider.physics.position.y - proj.hook.spring.get_anchor().y);
		next_length = std::clamp(next_length, lookup::min_hook_length, lookup::max_hook_length);
		proj.hook.spring.set_rest_length(next_length);

		//break out if player is far away from bob. we don't want the player to teleport.
		auto distance = util::magnitude(player_collider.physics.position - proj.hook.spring.get_bob());
		if (distance > 32.f) {
			proj.hook.break_free();
		}

		//handle map collisions while anchored
		player_collider.predictive_combined.set_position(proj.hook.spring.variables.physics.position);
		if (check_cell_collision(player_collider)) {
			player_collider.physics.zero();
		} else {
			player_collider.physics.position = proj.hook.spring.variables.physics.position - player_collider.dimensions / 2.f;
		}
		player_collider.sync_components();
	} else if (proj.hook.grapple_flags.test(arms::GrappleState::anchored)) {
		proj.hook.break_free();
	}

	if (player.controller.released_hook() && !proj.hook.grapple_flags.test(arms::GrappleState::snaking)) { proj.hook.break_free(); }
}

sf::Vector2<float> Map::get_spawn_position(int portal_source_map_id) {
	for (auto& portal : portals) {
		if (portal.source_map_id == portal_source_map_id) { return (portal.position); }
	}
	return Vec(300.f, 390.f);
}

bool Map::nearby(shape::Shape& first, shape::Shape& second) {
	return abs(first.position.x + first.dimensions.x * 0.5f - second.position.x) < lookup::unit_size_f * collision_barrier && abs(first.position.y - second.position.y) < lookup::unit_size_f * collision_barrier;
}

Tile& Map::tile_at(uint8_t const i, uint8_t const j) {
	// for checking tile value
	if (i * j < layers.at(MIDDLEGROUND).grid.cells.size()) { return layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x); }
}

shape::Shape& Map::shape_at(uint8_t const i, uint8_t const j) {
	// for testing collision
	if (i * j < layers.at(MIDDLEGROUND).grid.cells.size()) { return layers.at(MIDDLEGROUND).grid.cells.at(i + j * layers.at(MIDDLEGROUND).grid.dimensions.x).bounding_box; }
}

} // namespace world
