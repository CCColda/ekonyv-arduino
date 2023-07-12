#ifndef EKONYV_EVENTQUEUE_H
#define EKONYV_EVENTQUEUE_H

#include <Arduino.h>

#include <CircularBuffer.h>

#include <type_traits>

enum class Event {
	RERUN,
	REMOVE
};

template <typename T>
using TypedEventHandlerPtr = Event (*)(T &data);

template <size_t Length>
class EventQueue {
private:
	using EventProxyPtr = Event (*)(void *typedeventhandler, void *data, size_t size);

	struct EventData {
		void *data;
		size_t size;
		void *handler;
		EventProxyPtr handler_proxy;
	};

private:
	CircularBuffer<EventData, Length> m_events;

private:
	template <typename T>
	static Event handlerProxy(void *typedeventhandler, void *data, size_t size)
	{
		auto *handler = reinterpret_cast<TypedEventHandlerPtr<T>>(typedeventhandler);

		if (size != sizeof(T))
			return Event::REMOVE; // should remove

		return handler(*(T *)data);
	}

public:
	using index_t = typename decltype(m_events)::index_t;

public:
	EventQueue() : m_events() {}
	EventQueue(EventQueue &&other) : m_events()
	{
		for (index_t i = 0; i < other.m_events.size(); ++i)
			m_events.push(other.m_events.shift());
	}
	~EventQueue()
	{
		while (!m_events.isEmpty()) {
			auto item = m_events.shift();
			delete[] item.data;
		}
	}

	/**
	 * @brief Tries to enqueue an event
	 */
	template <typename T>
	bool tryEnqueue(T input_data, TypedEventHandlerPtr<T> handler)
	{
		static_assert(std::is_trivial<T>::value, "Data type must be trivial; so it can be put on the heap.");

		if (m_events.isFull())
			return false;

		byte *data = new byte[sizeof(T)];
		(*(T *)data) = input_data;

		m_events.push(EventData{(void *)data, sizeof(T), reinterpret_cast<void *>(handler), handlerProxy<T>});

		return true;
	}

	/**
	 * @brief Runs events until one gets removed; then adds the current event
	 * @warning May block
	 * @tparam T
	 */
	template <typename T>
	void forceEnqueue(T input_data, TypedEventHandlerPtr<T> handler)
	{
		while (m_events.isFull())
			execute(1);

		tryEnqueue(input_data, handler);
	}

	void execute(index_t n)
	{
		const auto size_at_start = m_events.size();
		for (index_t i = 0; i < min(n, size_at_start); ++i) {
			auto item = m_events.shift();

			const bool should_remove = (*item.handler_proxy)(item.handler, item.data, item.size) == Event::REMOVE;

			if (should_remove) {
				delete[] item.data;
			}
			else {
				// if the handler returned false, put it back in the buffer for later processing
				m_events.push(item);
			}
		}
	}
};

#endif // !defined(EKONYV_EVENTQUEUE_H)