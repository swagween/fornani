#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

// design based on Impossibly Fast Event:
// https://docs.google.com/presentation/d/1Y5JktAemPYNDmVJ5-3f_KQH1LTDmey-YJFAnly0Cxpo/edit?usp=sharing

// If you have questions about this, ask Ian.

namespace fornani::core {

namespace details {
struct ConnectionBase;

struct EventBase {
	struct call {
		void* object;
		void* func;
	};

	// We are able to optimize space by taking advantage of "struct of arrays" containers as both will always have the same size
	mutable std::vector<call> calls;
	mutable std::vector<ConnectionBase*> connections;

	// We are able to optimize space by stealing 2 unused bit from our vector size
	mutable bool calling = false;
	mutable bool dirty = false;

	EventBase() = default;
	~EventBase();
	EventBase(EventBase const&) = delete;
	EventBase& operator=(EventBase const&) = delete;
	EventBase(EventBase&& other) noexcept;
	EventBase& operator=(EventBase&& other) noexcept;
};

struct BlockedConnection {
	EventBase const* sig = nullptr;
	EventBase::call call = {.object = nullptr, .func = nullptr};
};

struct ConnectionBase {
	union {
		EventBase const* event;
		BlockedConnection* blocked_conn;
	};

	size_t idx;

	// space can be optimized by stealing bits from index as it's impossible to support max uint64 number of slots
	bool blocked = false;
	bool owned = false;

	ConnectionBase(EventBase const* sig, size_t idx) : event(sig), idx(idx) {}

	virtual ~ConnectionBase() {
		if (!blocked) {
			// NOLINTBEGIN
			if (event != nullptr) {
				event->calls[idx].object = nullptr;
				event->calls[idx].func = nullptr;
				event->connections[idx] = nullptr;
				event->dirty = true;
			}
			// NOLINTEND
		} else {
			delete blocked_conn; // NOLINT
		}
	}

	void set_event(EventBase const* sig) {
		if (blocked) {
			this->blocked_conn->sig = sig; // NOLINT
		} else {
			this->event = sig; // NOLINT
		}
	}

	void block() {
		if (!blocked) {
			blocked = true;
			EventBase const* orig_sig = event;					 // NOLINT
			event = nullptr;									 // NOLINT
			blocked_conn = new BlockedConnection;				 // NOLINT
			blocked_conn->sig = orig_sig;						 // NOLINT
			std::swap(blocked_conn->call, orig_sig->calls[idx]); // NOLINT
		}
	}

	void unblock() {
		if (blocked) {
			EventBase const* orig_sig = blocked_conn->sig;		 // NOLINT
			std::swap(blocked_conn->call, orig_sig->calls[idx]); // NOLINT
			delete blocked_conn;								 // NOLINT
			blocked_conn = nullptr;								 // NOLINT
			event = orig_sig;									 // NOLINT
			blocked = false;
		}
	}
};

template <typename T>
struct ConnectionNontrivial final : ConnectionBase {
	using ConnectionBase::ConnectionBase;

	~ConnectionNontrivial() override {
		if (event) { reinterpret_cast<T*>(&event->calls[idx].object)->~T(); } // NOLINT
	}
};

inline EventBase::~EventBase() {
	for (ConnectionBase* c : connections) {
		if (c != nullptr) {
			if (c->owned) {
				c->set_event(nullptr);
			} else {
				delete c; // NOLINT
			}
		}
	}
}

inline EventBase::EventBase(EventBase&& other) noexcept : calls(std::move(other.calls)), connections(std::move(other.connections)), calling(other.calling), dirty(other.dirty) {
	for (ConnectionBase* c : connections) {
		if (c != nullptr) { c->set_event(this); }
	}
}

inline EventBase& EventBase::operator=(EventBase&& other) noexcept {
	calls = std::move(other.calls);
	connections = std::move(other.connections);
	calling = other.calling;
	dirty = other.dirty;
	for (ConnectionBase* c : connections) {
		if (c != nullptr) { c->set_event(this); }
	}
	return *this;
}
} // namespace details

template <typename F>
struct Event;

// A connection without auto disconnection
struct ConnectionRaw {
	details::ConnectionBase* ptr = nullptr;
};

struct Connection {
	details::ConnectionBase* ptr = nullptr;

	void disconnect() {
		delete ptr;
		ptr = nullptr;
	}

	void block() const { ptr->block(); }

	void unblock() const { ptr->unblock(); }

	Connection() = default;

	~Connection() { disconnect(); }
	Connection(Connection const&) = delete;

	Connection& operator=(Connection const&) = delete;

	Connection(Connection&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }

	Connection& operator=(Connection&& other) noexcept {
		disconnect();
		ptr = other.ptr;
		other.ptr = nullptr;
		return *this;
	}

	Connection(ConnectionRaw conn) : ptr(conn.ptr) { ptr->owned = true; } // NOLINT
};

template <typename... A>
struct Event<void(A...)> : details::EventBase {
	template <typename... ActualArgsT>
	void operator()(ActualArgsT&&... args) const {
		bool const recursion = calling;
		if (!calling) { calling = true; }
		for (size_t i = 0, n = calls.size(); i < n; ++i) {
			auto& [object, func] = calls[i];
			if (func) {
				if (object == func) {
					reinterpret_cast<void (*)(A...)>(func)(std::forward<ActualArgsT>(args)...); // NOLINT
				} else {
					reinterpret_cast<void (*)(void*, A...)>(func)(&object, std::forward<ActualArgsT>(args)...); // NOLINT
				}
			}
		}

		if (!recursion) {
			calling = false;

			if (dirty) {
				dirty = false;
				// remove all empty slots while patching the stored index in the connection
				size_t sz = 0;
				for (size_t i = 0, n = connections.size(); i < n; ++i) {
					if (connections[i]) {
						connections[sz] = connections[i];
						calls[sz] = calls[i];
						connections[sz]->idx = sz;
						++sz;
					}
				}
				connections.resize(sz);
				calls.resize(sz);
			}
		}
	}

	template <typename... ActualArgsT>
	void invoke(ActualArgsT&&... args) const {
		operator()(std::forward<ActualArgsT>(args)...);
	}

	template <auto PMF, class C>
	ConnectionRaw connect(C* object) const {
		size_t idx = connections.size();
		// ReSharper disable once CppUseStructuredBinding
		auto& call = calls.emplace_back();
		call.object = object;
		call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { ((*static_cast<C**>(obj))->*PMF)(args...); }); // NOLINT
		auto* conn = new details::ConnectionBase(this, idx);														   // NOLINT
		connections.emplace_back(conn);
		return {conn};
	}

	template <auto PMF, class C>
	ConnectionRaw operator+=(C* object) const {
		return connect<PMF, C>(object);
	}

	template <auto func>
	ConnectionRaw connect() const {
		return connect(func);
	}

	ConnectionRaw connect(void (*func)(A...)) const {
		size_t idx = connections.size();
		auto& call = calls.emplace_back();
		call.func = call.object = reinterpret_cast<void*>(func); // NOLINT
		auto* conn = new details::ConnectionBase(this, idx);	 // NOLINT
		connections.emplace_back(conn);
		return {conn};
	}

	ConnectionRaw operator+=(void (*func)(A...)) const { return connect(func); }

	template <typename F>
	ConnectionRaw connect(F&& functor) const {
		using f_type = std::remove_pointer_t<std::remove_reference_t<F>>;
		if constexpr (std::is_convertible_v<f_type, void (*)(A...)>) {
			return connect(+functor);
		} else if constexpr (std::is_lvalue_reference_v<F>) {
			size_t idx = connections.size();
			auto& call = calls.emplace_back();
			call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { (*static_cast<f_type**>(obj))->operator()(args...); }); // NOLINT
			call.object = &functor;
			auto* conn = new details::ConnectionBase(this, idx); // NOLINT
			connections.emplace_back(conn);
			return {conn};
		} else if constexpr (sizeof(std::remove_pointer_t<f_type>) <= sizeof(void*)) {
			// copy the functor.
			size_t idx = connections.size();
			auto& call = calls.emplace_back();
			call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { static_cast<f_type*>(obj)->operator()(args...); }); // NOLINT
			new (&call.object) f_type(std::forward<F>(functor));
			using conn_t = std::conditional_t<std::is_trivially_destructible_v<F>, details::ConnectionBase, details::ConnectionNontrivial<F>>;
			details::ConnectionBase* conn = new conn_t(this, idx); // NOLINT
			connections.emplace_back(conn);
			return {conn};
		} else {
			struct unique {
				f_type* ptr;

				unique(f_type* ptr) : ptr(ptr) {} // NOLINT
				unique(unique const&) = delete;
				unique(unique&&) = delete;

				~unique() { delete ptr; }
			};

			size_t idx = connections.size();
			auto& call = calls.emplace_back();
			call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { static_cast<unique*>(obj)->ptr->operator()(args...); }); // NOLINT
			new (&call.object) unique{new f_type(std::forward<F>(functor))};
			details::ConnectionBase* conn = new details::ConnectionNontrivial<unique>(this, idx); // NOLINT
			connections.emplace_back(conn);
			return {conn};
		}
	}

	template <typename F>
	ConnectionRaw operator+=(F&& functor) const {
		return connect(std::forward<F>(functor));
	}
};

} // namespace fornani::core