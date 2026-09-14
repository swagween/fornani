
#include <fornani/entities/player/PhotoAlbum.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

void PhotoAlbum::serialize(dj::Json& out) const {
	out = dj::Json::empty_array();
	for (auto const& postcard : m_postcards) { out.push_back(postcard.get_tag()); }
}

void PhotoAlbum::unserialize(automa::ServiceProvider& svc, dj::Json const& in) {
	m_postcards.clear();
	for (auto const& postcard : in.as_array()) { m_postcards.push_back(Postcard{svc, postcard.as_string()}); }
}

void PhotoAlbum::add_postcard(automa::ServiceProvider& svc, std::string_view tag) {
	m_postcards.push_back(Postcard{svc, tag});
	svc.notifications.push_notification(svc, svc.data.gui_text["notifications"]["add_postcard"].as_string());
}

} // namespace fornani
