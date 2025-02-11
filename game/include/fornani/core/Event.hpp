#pragma once

#include <vector>
#include <shared_mutex>
#include <atomic>
#include <memory>
#include <mutex>
#include <algorithm>
#include <cassert>
#include <unordered_map>

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
	EventBase(const EventBase&) = delete;
	EventBase& operator=(const EventBase&) = delete;
	EventBase(EventBase&& other) noexcept;
	EventBase& operator=(EventBase&& other) noexcept;
};

struct BlockedConnection {
	const EventBase* sig = nullptr;
	EventBase::call call = {nullptr, nullptr};
};

struct ConnectionBase {
	union {
		const EventBase* event;
		BlockedConnection* blocked_conn;
	};

	size_t idx;

	// space can be optimized by stealing bits from index as it's impossible to support max uint64 number of slots
	bool blocked = false;
	bool owned = false;

	ConnectionBase(const EventBase* sig, size_t idx) : event(sig), idx(idx) {}

	virtual ~ConnectionBase() {
		if (!blocked) {
			if (event) {
				event->calls[idx].object = nullptr;
				event->calls[idx].func = nullptr;
				event->connections[idx] = nullptr;
				event->dirty = true;
			}
		} else { delete blocked_conn; }
	}

	void set_event(const EventBase* sig) {
		if (blocked) this->blocked_conn->sig = sig;
		else this->event = sig;
	}

	void block() {
		if (!blocked) {
			blocked = true;
			const EventBase* orig_sig = event;
			event = nullptr;
			blocked_conn = new BlockedConnection;
			blocked_conn->sig = orig_sig;
			std::swap(blocked_conn->call, orig_sig->calls[idx]);
		}
	}

	void unblock() {
		if (blocked) {
			const EventBase* orig_sig = blocked_conn->sig;
			std::swap(blocked_conn->call, orig_sig->calls[idx]);
			delete blocked_conn;
			blocked_conn = nullptr;
			event = orig_sig;
			blocked = false;
		}
	}
};

template <typename T>
struct ConnectionNontrivial final : ConnectionBase {
	using ConnectionBase::ConnectionBase;

	~ConnectionNontrivial() override { if (event) reinterpret_cast<T*>(&event->calls[idx].object)->~T(); }
};

inline EventBase::~EventBase() {
	for (ConnectionBase* c : connections) {
		if (c) {
			if (c->owned) c->set_event(nullptr);
			else delete c;
		}
	}
}

inline EventBase::EventBase(EventBase&& other) noexcept
	: calls(std::move(other.calls))
	  , connections(std::move(other.connections))
	  , calling(other.calling)
	  , dirty(other.dirty) { for (ConnectionBase* c : connections) if (c) c->set_event(this); }

inline EventBase& EventBase::operator=(EventBase&& other) noexcept {
	calls = std::move(other.calls);
	connections = std::move(other.connections);
	calling = other.calling;
	dirty = other.dirty;
	for (ConnectionBase* c : connections) if (c) c->set_event(this);
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
	Connection(const Connection&) = delete;

	Connection& operator=(const Connection&) = delete;

	Connection(Connection&& other) noexcept
		: ptr(other.ptr) { other.ptr = nullptr; }

	Connection& operator=(Connection&& other) noexcept {
		disconnect();
		ptr = other.ptr;
		other.ptr = nullptr;
		return *this;
	}

	Connection(ConnectionRaw conn) : ptr(conn.ptr) { ptr->owned = true; }
};

template <typename... A>
struct Event<void(A...)> : details::EventBase {
	template <typename... ActualArgsT>
	void operator()(ActualArgsT&&... args) const {
		const bool recursion = calling;
		if (!calling) calling = true;
		for (size_t i = 0, n = calls.size(); i < n; ++i) {
			auto& [object, func] = calls[i];
			if (func) {
				if (object == func) reinterpret_cast<void(*)(A...)>(func)(std::forward<ActualArgsT>(args)...);
				else reinterpret_cast<void(*)(void*, A...)>(func)(&object, std::forward<ActualArgsT>(args)...);
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
		auto& call = calls.emplace_back();
		call.object = object;
		call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { ((*static_cast<C**>(obj))->*PMF)(args...); });
		details::ConnectionBase* conn = new details::ConnectionBase(this, idx);
		connections.emplace_back(conn);
		return {conn};
	}

	template <auto PMF, class C>
	ConnectionRaw operator+=(C* object) const {
		return connect<PMF, C>(object);
	}

	template <auto func>
	ConnectionRaw connect() const { return connect(func); }

	ConnectionRaw connect(void (*func)(A...)) const {
		size_t idx = connections.size();
		auto& call = calls.emplace_back();
		call.func = call.object = reinterpret_cast<void*>(func);
		details::ConnectionBase* conn = new details::ConnectionBase(this, idx);
		connections.emplace_back(conn);
		return {conn};
	}

	ConnectionRaw operator+=(void (*func)(A...)) const {
		return connect(func);
	}

	template <typename F>
	ConnectionRaw connect(F&& functor) const {
		using f_type = std::remove_pointer_t<std::remove_reference_t<F>>;
		if constexpr (std::is_convertible_v<f_type, void(*)(A...)>) { return connect(+functor); } else if constexpr (std::is_lvalue_reference_v<F>) {
			size_t idx = connections.size();
			auto& call = calls.emplace_back();
			call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { (*static_cast<f_type**>(obj))->operator()(args...); });
			call.object = &functor;
			details::ConnectionBase* conn = new details::ConnectionBase(this, idx);
			connections.emplace_back(conn);
			return {conn};
		} else if constexpr (sizeof(std::remove_pointer_t<f_type>) <= sizeof(void*)) {
			// copy the functor.
			size_t idx = connections.size();
			auto& call = calls.emplace_back();
			call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { static_cast<f_type*>(obj)->operator()(args...); });
			new(&call.object) f_type(std::move(functor));
			using conn_t = std::conditional_t<std::is_trivially_destructible_v<F>, details::ConnectionBase, details::ConnectionNontrivial<F>>;
			details::ConnectionBase* conn = new conn_t(this, idx);
			connections.emplace_back(conn);
			return {conn};
		} else {
			struct unique {
				f_type* ptr;

				unique(f_type* ptr) : ptr(ptr) {}
				unique(const unique&) = delete;
				unique(unique&&) = delete;

				~unique() { delete ptr; }
			};

			size_t idx = connections.size();
			auto& call = calls.emplace_back();
			call.func = reinterpret_cast<void*>(+[](void* obj, A... args) { static_cast<unique*>(obj)->ptr->operator()(args...); });
			new(&call.object) unique{new f_type(std::move(functor))};
			details::ConnectionBase* conn = new details::ConnectionNontrivial<unique>(this, idx);
			connections.emplace_back(conn);
			return {conn};
		}
	}

	template <typename F>
	ConnectionRaw operator+=(F&& functor) const {
		return connect(functor);
	}
};

} // namespace fornani::core