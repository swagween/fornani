
#include <fornani/entities/enemy/EnemyCatalog.hpp>
#include <fornani/entities/enemy/boss/GrandMastiff.hpp>
#include <fornani/entities/enemy/boss/Haunch.hpp>
#include <fornani/entities/enemy/boss/Lynx.hpp>
#include <fornani/entities/enemy/boss/Miaag.hpp>
#include <fornani/entities/enemy/boss/Minigus.hpp>
#include <fornani/entities/enemy/boss/Nimbus.hpp>
#include <fornani/entities/enemy/catalog/Archer.hpp>
#include <fornani/entities/enemy/catalog/Beamsprout.hpp>
#include <fornani/entities/enemy/catalog/Beamstalk.hpp>
#include <fornani/entities/enemy/catalog/Caster.hpp>
#include <fornani/entities/enemy/catalog/Crow.hpp>
#include <fornani/entities/enemy/catalog/Demon.hpp>
#include <fornani/entities/enemy/catalog/DumpsterDiver.hpp>
#include <fornani/entities/enemy/catalog/Eyebit.hpp>
#include <fornani/entities/enemy/catalog/Eyebot.hpp>
#include <fornani/entities/enemy/catalog/Frdog.hpp>
#include <fornani/entities/enemy/catalog/Grappler.hpp>
#include <fornani/entities/enemy/catalog/Hellion.hpp>
#include <fornani/entities/enemy/catalog/Hulmet.hpp>
#include <fornani/entities/enemy/catalog/Hurtle.hpp>
#include <fornani/entities/enemy/catalog/Imp.hpp>
#include <fornani/entities/enemy/catalog/Junker.hpp>
#include <fornani/entities/enemy/catalog/Junkfly.hpp>
#include <fornani/entities/enemy/catalog/Lymphocyte.hpp>
#include <fornani/entities/enemy/catalog/Macrophage.hpp>
#include <fornani/entities/enemy/catalog/Mastiff.hpp>
#include <fornani/entities/enemy/catalog/Meatsquash.hpp>
#include <fornani/entities/enemy/catalog/Minion.hpp>
#include <fornani/entities/enemy/catalog/Mizzle.hpp>
#include <fornani/entities/enemy/catalog/MizzleEgg.hpp>
#include <fornani/entities/enemy/catalog/Sentinel.hpp>
#include <fornani/entities/enemy/catalog/Spitefly.hpp>
#include <fornani/entities/enemy/catalog/Summoner.hpp>
#include <fornani/entities/enemy/catalog/Tank.hpp>
#include <fornani/entities/enemy/catalog/Thief.hpp>
#include <fornani/entities/enemy/catalog/Thug.hpp>

namespace fornani::enemy {

EnemyCatalog::EnemyCatalog(automa::ServiceProvider& svc) {
	EnemyRegistry::register_factory(0, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Frdog>(svc, map); });
	EnemyRegistry::register_factory(1, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Tank>(svc, map, p.variant); });
	EnemyRegistry::register_factory(3, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Thug>(svc, map); });
	EnemyRegistry::register_factory(4, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Eyebot>(svc, map); });
	EnemyRegistry::register_factory(5, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Eyebit>(svc, map, p.spawned); });
	EnemyRegistry::register_factory(6, [](auto& svc, auto& map, auto& ctx, EnemyParameters const&) { return std::make_unique<Minigus>(svc, map, ctx); });
	EnemyRegistry::register_factory(7, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Demon>(svc, map, p.variant); });
	EnemyRegistry::register_factory(8, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Caster>(svc, map, p.variant); });
	EnemyRegistry::register_factory(9, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Archer>(svc, map); });
	EnemyRegistry::register_factory(10, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Beamstalk>(svc, map, p.dir); });
	EnemyRegistry::register_factory(11, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Meatsquash>(svc, map); });
	EnemyRegistry::register_factory(12, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Imp>(svc, map, p.variant); });
	EnemyRegistry::register_factory(13, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Hulmet>(svc, map); });
	EnemyRegistry::register_factory(14, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Miaag>(svc, map); });
	EnemyRegistry::register_factory(15, [](auto& svc, auto& map, auto& ctx, EnemyParameters const&) { return std::make_unique<Lynx>(svc, map, ctx); });
	EnemyRegistry::register_factory(16, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Summoner>(svc, map, p.variant); });
	EnemyRegistry::register_factory(17, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Minion>(svc, map, p.variant); });
	EnemyRegistry::register_factory(18, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Spitefly>(svc, map, p.variant); });
	EnemyRegistry::register_factory(19, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Junkfly>(svc, map, p.variant); });
	EnemyRegistry::register_factory(20, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Junker>(svc, map, p.variant); });
	EnemyRegistry::register_factory(21, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<DumpsterDiver>(svc, map, p.variant); });
	EnemyRegistry::register_factory(22, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Mastiff>(svc, map, p.variant); });
	EnemyRegistry::register_factory(23, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Beamsprout>(svc, map, p.dir); });
	EnemyRegistry::register_factory(24, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<GrandMastiff>(svc, map); });
	EnemyRegistry::register_factory(25, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Crow>(svc, map, p.multispawn.spread); });
	EnemyRegistry::register_factory(26, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Mizzle>(svc, map); });
	EnemyRegistry::register_factory(27, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<MizzleEgg>(svc, map); });
	EnemyRegistry::register_factory(28, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Haunch>(svc, map); });
	EnemyRegistry::register_factory(29, [](auto& svc, auto& map, auto&, EnemyParameters const&) { return std::make_unique<Lymphocyte>(svc, map); });
	EnemyRegistry::register_factory(30, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Sentinel>(svc, map, p.variant); });
	EnemyRegistry::register_factory(31, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Hellion>(svc, map); });
	EnemyRegistry::register_factory(32, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Hurtle>(svc, map); });
	EnemyRegistry::register_factory(33, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Thief>(svc, map); });
	EnemyRegistry::register_factory(34, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Grappler>(svc, map); });
	EnemyRegistry::register_factory(35, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Nimbus>(svc, map); });
	EnemyRegistry::register_factory(36, [](auto& svc, auto& map, auto&, EnemyParameters const& p) { return std::make_unique<Macrophage>(svc, map); });
}

void EnemyCatalog::update() {
	std::erase_if(enemies, [this](auto const& e) { return e->gone() || e->despawn_requested(); });
}

void EnemyCatalog::push_enemy(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, int id, EnemyParameters params) {
	if (auto enemy = EnemyRegistry::create(id, svc, map, context, params)) { enemies.push_back(std::move(enemy)); }
	enemies.back()->set_handle(++m_next_handle);
	if (params.spawned) { enemies.back()->center_at_position(); }
}

} // namespace fornani::enemy
