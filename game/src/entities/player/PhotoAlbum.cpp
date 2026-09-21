
#include <fornani/entities/player/PhotoAlbum.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

void PhotoAlbum::serialize(dj::Json& out) const {
	out = dj::Json::empty_array();
	for (auto const& postcard : m_postcards) { out.push_back(postcard.get_tag()); }
}

void PhotoAlbum::unserialize(automa::ServiceProvider& svc, dj::Json const& in) {
	m_postcards.clear();
	for (auto [i, postcard] : std::views::enumerate(in.as_array())) { m_postcards.add(Postcard{svc, postcard.as_string(), std::uint8_t(i)}); }
}

void PhotoAlbum::add_postcard(automa::ServiceProvider& svc, int index) {
	if (m_postcards.add(Postcard{svc, svc.data.postcards[index]["tag"].as_string_view(), std::uint8_t(index)})) { svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["add_postcard"].as_string()); }
}

void PhotoAlbum::add_postcard(automa::ServiceProvider& svc, std::string_view tag) {
	for (auto [i, p] : std::views::enumerate(svc.data.postcards.as_array())) {
		if (tag == p["tag"].as_string()) {
			if (m_postcards.add(Postcard{svc, tag, std::uint8_t(i)})) { svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["add_postcard"].as_string()); }
		}
	}
}

} // namespace fornani
